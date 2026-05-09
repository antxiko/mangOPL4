// tb_arbiter.cpp — Verilator harness para wave_arbiter smoke test.
//
// Reproduce la secuencia de wavemem.asc:
//   1. Setup pointer → 0x200000 (regs 0x03/0x04/0x05)
//   2. OUT &H7E,&H06 (latch reg_addr=0x06)
//   3. Loop: OUT &H7F,(I*17) for I=0..15  (reg_wr_stb a reg 0x06)
//   4. Reset pointer
//   5. OUT &H7E,&H06
//   6. Loop: INP &H7F → reg_rd_done_strobe + leer mem_data_byte
//
// Y verifica que los 16 bytes leídos = lo que se escribió.
//
#include <cstdio>
#include <cstdint>
#include <cstdlib>
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

static void tick_n(int n) {
    for (int i = 0; i < n; i++) tick();
}

static int trace_enable = 0;

static void trace_one(const char* label) {
    if (!trace_enable) return;
    printf("[t=%5llu %s] sdram_st=%2d busa_addr=0x%06x busa_we=%d busa_ack=%d  primary_addr=0x%06x primary_we=%d primary_ack=%d  save_addr=0x%06x save_dqm=0x%x\n",
        (unsigned long long)main_time, label,
        dut->dbg_sdram_state,
        dut->dbg_busa_addr, dut->dbg_busa_we_n, dut->dbg_busa_ack_n,
        dut->dbg_primary_addr, dut->dbg_primary_we_n, dut->dbg_primary_ack_n,
        dut->dbg_sdram_save_addr, dut->dbg_sdram_save_dqm);
}

// Pulso de 1 ciclo en reg_wr_stb con reg_addr/reg_data dados.
// El reg_wr_stb se asserta durante 1 ciclo, luego se desasserta.
static void reg_write(uint8_t addr, uint8_t data) {
    dut->reg_addr = addr;
    dut->reg_data = data;
    dut->reg_wr_stb = 1;
    tick();
    trace_one("STB1");
    dut->reg_wr_stb = 0;
    // Espera unos ciclos para que mempointer procese (FSM SDRAM ~10
    // ciclos por operación — wavemem en MSX da varios µs entre OUTs).
    for (int i = 0; i < 20; i++) {
        tick();
        trace_one("WAIT");
    }
}

// INP &H7F en MSX: el byte ya está en mem_data_byte (de prefetch
// previo), se devuelve, y luego reg_rd_done_strobe pulsa para
// auto-incrementar pointer + iniciar nuevo prefetch.
static uint8_t reg_read_06() {
    dut->reg_addr = 0x06;
    // Capture ANTES del strobe (= valor del prefetch previo)
    uint8_t got = dut->mem_data_byte;
    // Strobe: increment + nuevo prefetch
    dut->reg_rd_done_stb = 1;
    tick();
    dut->reg_rd_done_stb = 0;
    tick_n(20);  // wait nuevo prefetch
    return got;
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    dut = new Varbiter_smoke_top;

    // Reset
    dut->RESET_n         = 0;
    dut->bus_reset_n     = 0;
    dut->reg_wr_stb      = 0;
    dut->reg_addr        = 0;
    dut->reg_data        = 0;
    dut->reg_rd_done_stb = 0;
    dut->bus_merq_n      = 1;   // Z80 en I/O cycle siempre (simplificación)
    dut->key_on_slot0    = 0;
    dut->cache_index     = 0;
    tick_n(10);
    dut->RESET_n     = 1;
    dut->bus_reset_n = 1;
    tick_n(10);

    printf("=== Setup pointer = 0x200000 (Sample RAM main) ===\n");
    reg_write(0x03, 0x20);  // ptr hi
    reg_write(0x04, 0x00);  // ptr mid
    reg_write(0x05, 0x00);  // ptr lo

    printf("=== Write 16 bytes (I*17 for I=0..15) ===\n");
    // Latch reg_addr = 0x06 (data port)
    printf("--- write i=0, expected sdram_addr=0x300000 (low=00) ---\n");
    trace_enable = 1;
    reg_write(0x06, 0 * 17);  // primer write con reg_addr=0x06 latched
    trace_enable = 0;
    printf("--- write i=1, expected sdram_addr=0x300001 (low=01) ---\n");
    trace_enable = 1;
    reg_write(0x06, 1 * 17);
    trace_enable = 0;
    for (int i = 2; i < 16; i++) {
        // Subsiguientes writes a reg 0x06 (reg_addr ya está latched).
        // Pero nuestra reg_write hace OUT &H7E,addr y luego OUT &H7F,data;
        // aquí simplificamos pasando addr=0x06 cada vez. El behavior es
        // idéntico al del MSX real (mempointer detecta reg_wr_stb && reg_addr=0x06).
        reg_write(0x06, i * 17);
    }

    printf("=== Reset pointer = 0x200000 ===\n");
    reg_write(0x03, 0x20);
    reg_write(0x04, 0x00);
    reg_write(0x05, 0x00);

    printf("=== Read back 16 bytes ===\n");
    // Primer prefetch: el reg_write a 0x05 ya disparó pending_read.
    // Esperar a que termine el primer fetch tras el último reg_write.
    tick_n(30);

    int errors = 0;
    for (int i = 0; i < 16; i++) {
        uint8_t got = reg_read_06();
        uint8_t expected = (i * 17) & 0xFF;
        const char* mark = (got == expected) ? "OK" : "FAIL";
        printf("  [%2d] addr=0x20%04x  got=0x%02x  expected=0x%02x  %s\n",
               i, i, got, expected, mark);
        if (got != expected) errors++;
    }

    printf("\n=== RESULT: %s (%d errors) ===\n",
           errors == 0 ? "PASS" : "FAIL", errors);

    delete dut;
    return errors;
}
