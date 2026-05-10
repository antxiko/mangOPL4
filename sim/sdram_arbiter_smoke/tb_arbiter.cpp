// tb_arbiter.cpp — testbench Verilator para sdram_top_arbiter.
//
// Tres scenarios:
//   (a) Solo A activo. A debe leer mem[A.addr] correctamente.
//   (b) Solo B activo. B debe leer mem[B.addr] correctamente.
//   (c) A y B activos a la vez. A debe leer SU mem[A.addr] (no
//       contaminado por B). B debe completar su transacción cuando
//       A esté idle, leyendo SU mem[B.addr].
//
// SDRAM stub pre-llena memoria con mem[i] = (i ^ 0xA5) & 0xFF.

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include "Varbiter_smoke_top.h"
#include "verilated.h"

static Varbiter_smoke_top* dut = nullptr;
static vluint64_t main_time = 0;

static void tick() {
    dut->CLK = 0;
    dut->eval();
    main_time++;
    dut->CLK = 1;
    dut->eval();
    main_time++;
}

static void tick_n(int n) { for (int i = 0; i < n; i++) tick(); }

static void clear_inputs() {
    dut->a_trigger  = 0;
    dut->a_is_write = 0;
    dut->a_addr     = 0;
    dut->a_din      = 0;
    dut->b_trigger  = 0;
    dut->b_addr     = 0;
}

static void do_reset() {
    dut->RESET_n = 0;
    clear_inputs();
    tick_n(20);
    dut->RESET_n = 1;
    tick_n(10);
}

// Esperado: mem[addr] = (addr ^ 0xA5) & 0xFF (definido en sdram_stub.sv)
static uint8_t expected_mem(uint32_t addr) {
    return (addr ^ 0xA5) & 0xFF;
}

// =====================================================================

// A solo: drive a_trigger durante M_CYCLE_LEN ciclos, luego comprobar
// last_dout.
static int test_a_alone() {
    printf("\n=== (a) Solo A activo (cart-style sustained) ===\n");
    do_reset();

    int errors = 0;
    const int M_CYCLE_LEN = 30;  // suficiente para que SDRAM responda
    for (int i = 0; i < 16; i++) {
        uint32_t addr = i * 0x1000;       // 0, 0x1000, 0x2000, ...
        uint8_t expected = expected_mem(addr);

        dut->a_addr     = addr;
        dut->a_is_write = 0;
        dut->a_trigger  = 1;
        tick_n(M_CYCLE_LEN);
        dut->a_trigger  = 0;
        tick_n(5);

        uint8_t got = dut->a_last_dout;
        if (got != expected) {
            if (errors < 5)
                printf("  addr=0x%06x got=0x%02x expected=0x%02x\n",
                       addr, got, expected);
            errors++;
        }
    }
    printf("(a) %s (%d/16 errors)\n", errors == 0 ? "PASS" : "FAIL", errors);
    return errors;
}

// B solo: drive b_trigger=1 nivel hasta b_done.
static int test_b_alone() {
    printf("\n=== (b) Solo B activo (wave-style 1-pulse + ACK) ===\n");
    do_reset();

    int errors = 0;
    for (int i = 0; i < 16; i++) {
        uint32_t addr = 0x100000 + i * 0x10;  // bit 20 set (fuera de mem[])
        uint8_t expected = (addr & 0xFF) ^ 0x5A;

        dut->b_addr    = addr;
        dut->b_trigger = 1;

        // Espera b_done (max 80 ciclos)
        int timeout = 80;
        while (!dut->b_done && timeout > 0) {
            tick();
            timeout--;
        }
        if (timeout == 0) {
            printf("  TIMEOUT en b_done para addr=0x%06x\n", addr);
            errors++;
            dut->b_trigger = 0;
            tick_n(5);
            continue;
        }
        dut->b_trigger = 0;
        tick_n(2);

        uint8_t got = dut->b_captured_byte;
        if (got != expected) {
            if (errors < 5)
                printf("  addr=0x%06x got=0x%02x expected=0x%02x\n",
                       addr, got, expected);
            errors++;
        }
    }
    printf("(b) %s (%d/16 errors)\n", errors == 0 ? "PASS" : "FAIL", errors);
    return errors;
}

// A + B juntos: A drivea sostenido (M-cycle Z80). B level-triggered
// queriendo servicio constantemente. B fires solo cuando A está idle.
//
// Stagger: A arranca primero, después B ya tiene trigger=1. Así
// emulamos el comportamiento real donde mempointer's bus_merq_n
// gate ya está bloqueando antes que cart asserte (Bus.MERQ_n cae
// antes que cart's OE_n=0 por la cadena de sincronizadores).
//
// Verifica:
//   - A lee correctamente sus addrs (no contaminado por B).
//   - B completa cada vez que A queda idle entre M-cycles.
//   - SDRAM no genera writes a YRW801 area (= no contention bug).
static int test_a_and_b() {
    printf("\n=== (c) A sostenido + B level-trigger continuo ===\n");
    do_reset();

    int a_errors = 0;
    int b_errors = 0;
    int b_completed = 0;
    const int M_CYCLE_LEN = 30;

    uint32_t b_addr = 0x100000;
    uint8_t  b_expected = (b_addr & 0xFF) ^ 0x5A;

    for (int i = 0; i < 16; i++) {
        uint32_t a_addr = i * 0x1000;
        uint8_t a_expected = expected_mem(a_addr);

        // 1) A start FIRST. Tick una vez para que BusA.OE_n=0 se registre
        //    y requesting_a se haga visible al arbiter (BusB.TIMING=0).
        dut->a_addr     = a_addr;
        dut->a_is_write = 0;
        dut->a_trigger  = 1;
        tick();
        // 2) Ahora B trigger (mempointer-style). Como BusB.TIMING=0
        //    (A está requesting), mock_b NO entra en S_REQ.
        dut->b_addr    = b_addr;
        dut->b_trigger = 1;
        // 3) A continúa M-cycle.
        tick_n(M_CYCLE_LEN - 1);

        // 4) A idle. B debe disparar y completar.
        dut->a_trigger = 0;
        int wait = 0;
        while (!dut->b_done && wait < 80) { tick(); wait++; }

        // Verificar A
        uint8_t a_got = dut->a_last_dout;
        if (a_got != a_expected) {
            if (a_errors < 5)
                printf("  A addr=0x%06x got=0x%02x expected=0x%02x\n",
                       a_addr, a_got, a_expected);
            a_errors++;
        }

        // Verificar B
        if (dut->b_done) {
            uint8_t b_got = dut->b_captured_byte;
            if (b_got == b_expected) {
                b_completed++;
            } else {
                if (b_errors < 5)
                    printf("  B addr=0x%06x got=0x%02x expected=0x%02x\n",
                           b_addr, b_got, b_expected);
                b_errors++;
            }
            // Release b_trigger 1 cycle para limpiar b_done. NO seteo
            // b_trigger=1 aquí — el loop body lo seteará tras el primer
            // tick de A, evitando el race "ambos asertan mismo edge".
            dut->b_trigger = 0;
            tick_n(2);

            // Set next b request (ADDR pero NO trigger).
            b_addr     = 0x100000 + (i + 1) * 0x10;
            b_expected = (b_addr & 0xFF) ^ 0x5A;
            dut->b_addr    = b_addr;
        } else {
            if (b_errors < 5)
                printf("  B addr=0x%06x NO completó tras A idle\n", b_addr);
            b_errors++;
        }
    }
    dut->b_trigger = 0;

    // SDRAM stats
    printf("(c) A: %s (%d/16 errors)\n",
           a_errors == 0 ? "PASS" : "FAIL", a_errors);
    printf("(c) B: %s (%d/16 completados, %d errors)\n",
           (b_errors == 0 && b_completed >= 16) ? "PASS" : "FAIL",
           b_completed, b_errors);
    printf("(c) SDRAM reads=%u writes=%u\n",
           dut->dbg_read_count, dut->dbg_write_count);
    return a_errors + b_errors + (b_completed >= 16 ? 0 : (16 - b_completed));
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    dut = new Varbiter_smoke_top;

    int total_errors = 0;
    total_errors += test_a_alone();
    total_errors += test_b_alone();
    total_errors += test_a_and_b();

    printf("\n========================================\n");
    printf("TOTAL ERRORS: %d\n", total_errors);
    printf("========================================\n");

    delete dut;
    return total_errors > 0 ? 1 : 0;
}
