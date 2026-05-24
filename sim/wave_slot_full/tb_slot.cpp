// tb_slot.cpp — Verilator TB para slot_full_top.
//
// DISCRIMINADOR DEL CACHE BUG R5 v0:
//   1. Setup: start_addr=0x100000 (region "sample 1": mem[0..15] = 0x01..0x10).
//   2. key_on, esperar fetch1 complete (byte_a, byte_b populados).
//   3. Verificar: byte_a=0x01, byte_b=0x02 (de mem[0..1]).
//   4. key_off.
//   5. Cambiar start_addr a 0x100080 (region "sample 2": mem[0x80..0x8F] = 0x81..0x90).
//   6. key_on, esperar fetch1 complete.
//   7. Verificar: byte_a=0x81, byte_b=0x82.
//      - Si byte_a=0x01 → cache NO se invalida (mi R5 v1 fix NO funciona).
//      - Si byte_a=0x81 → cache invalidate funciona correctamente.
//
// 8. Cambiar a start_addr=0x1000C0 (region "sample 3": mem[0xC0..0xCF] = 0xC1..0xD0).
// 9. key_on, verificar byte_a=0xC1, byte_b=0xC2.
//
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include "Vslot_full_top.h"
#include "verilated.h"

static Vslot_full_top* dut = nullptr;
static vluint64_t main_time = 0;

static void tick() {
    dut->CLK = 0;
    dut->eval();
    main_time++;
    dut->CLK = 1;
    dut->eval();
    main_time++;
}

static void tick_n(int n) {
    for (int i = 0; i < n; i++) tick();
}

static void trace(const char* label) {
    printf("[t=%5llu %-12s] state=%d last_idx=0x%04x byte_a=0x%02x byte_b=0x%02x | ram_addr=0x%06x oe=%d ack=%d timing=%d | sample=%d\n",
        (unsigned long long)main_time, label,
        dut->dbg_fetch_state, dut->dbg_fetch_last_idx,
        dut->dbg_fetch_byte_a, dut->dbg_fetch_byte_b,
        dut->dbg_ram_addr, dut->dbg_ram_oe_n, dut->dbg_ram_ack_n, dut->dbg_ram_timing,
        (int16_t)dut->sample_out);
}

// Genera 1 pulso sample_tick (alto 1 ciclo, luego bajo).
static void sample_tick_pulse() {
    dut->sample_tick = 1;
    tick();
    dut->sample_tick = 0;
}

// Esperar N sample_ticks (cada uno separado por M cycles).
static void wait_sample_ticks(int n, int cycles_between = 50) {
    for (int i = 0; i < n; i++) {
        sample_tick_pulse();
        tick_n(cycles_between);
    }
}

// Esperar a que fetch1 vuelva a S_IDLE (= fetch completo) o timeout.
static bool wait_fetch_complete(int max_cycles = 200) {
    for (int i = 0; i < max_cycles; i++) {
        tick();
        if (dut->dbg_fetch_state == 0) return true;  // S_IDLE
    }
    return false;
}

// Test 1 key_on/off ciclo con start_addr dado. Devuelve los bytes
// observados tras fetch.
static void key_cycle(uint32_t start_addr, uint8_t& byte_a, uint8_t& byte_b, const char* label) {
    dut->start_addr_sdram_in = start_addr;
    printf("\n=== %s: start_addr=0x%06x ===\n", label, start_addr);
    trace("PRE-keyon");

    dut->key_on = 1;
    tick();  // edge para que key_on llegue
    trace("KEYON");

    // Dispara 1 sample_tick para que phase_acc avance (curr_idx cambia)
    sample_tick_pulse();
    tick_n(5);
    trace("POST-TICK");

    // Esperar fetch1 transitar y completar
    if (!wait_fetch_complete(300)) {
        printf("  TIMEOUT esperando fetch (state stuck en %d)\n", dut->dbg_fetch_state);
    }
    trace("FETCH-DONE");

    byte_a = dut->dbg_fetch_byte_a;
    byte_b = dut->dbg_fetch_byte_b;

    dut->key_on = 0;
    tick_n(20);
    trace("KEYOFF");
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    dut = new Vslot_full_top;

    // Reset
    dut->RESET_n     = 0;
    dut->bus_reset_n = 0;
    dut->sample_tick = 0;
    dut->fnum        = 0;
    dut->octave      = 4;        // OCT=4
    dut->key_on      = 0;
    dut->start_addr_sdram_in = 0x100000;
    tick_n(10);
    dut->RESET_n     = 1;
    dut->bus_reset_n = 1;
    tick_n(20);

    printf("\n############ Cache bug discriminator ############\n");

    uint8_t ba1, bb1, ba2, bb2, ba3, bb3;
    key_cycle(0x100000, ba1, bb1, "Test1 (region sample 1, bytes 0x01..0x02)");
    tick_n(50);
    key_cycle(0x100080, ba2, bb2, "Test2 (region sample 2, bytes 0x81..0x82)");
    tick_n(50);
    key_cycle(0x1000C0, ba3, bb3, "Test3 (region sample 3, bytes 0xC1..0xC2)");

    printf("\n############ RESUMEN ############\n");
    printf("Test 1: byte_a=0x%02x (exp 0x01), byte_b=0x%02x (exp 0x02) %s\n",
           ba1, bb1, (ba1==0x01 && bb1==0x02) ? "OK" : "FAIL");
    printf("Test 2: byte_a=0x%02x (exp 0x81), byte_b=0x%02x (exp 0x82) %s\n",
           ba2, bb2, (ba2==0x81 && bb2==0x82) ? "OK" : "FAIL");
    printf("Test 3: byte_a=0x%02x (exp 0xC1), byte_b=0x%02x (exp 0xC2) %s\n",
           ba3, bb3, (ba3==0xC1 && bb3==0xC2) ? "OK" : "FAIL");

    if (ba2 == 0x01 || ba3 == 0x81) {
        printf("\n*** CACHE BUG: fetch1 no se invalida en key_on edge — byte_a quedo stale\n");
    } else if (ba2 == 0x81 && ba3 == 0xC1) {
        printf("\n*** Cache fix WORKS: byte_a refleja start_addr en cada key_on\n");
    }

    delete dut;
    return 0;
}
