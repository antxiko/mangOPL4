// tb_contention.cpp — testbench Verilator para reproducir el bug de
// bus contention con cartridge_ram.
//
// No usa accesos directos a memoria SDRAM. Todo via interfaces:
//   - mempointer (wave regs 02-06) para wavemem-like.
//   - fake_cartridge_ram (msx_in_mcycle/addr/din) para Z80 memory access.
//
// Scenarios:
//   S1 — Z80 write/read via fake_cartridge_ram, sin fetch1. Sanity.
//   S2 — fetch1 firing en paralelo con Z80 cycles. Detect corrupción.
//
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <random>
#include "Vcontention_top.h"
#include "verilated.h"

static Vcontention_top* dut = nullptr;
static vluint64_t main_time = 0;

// Log de cada SDRAM op capture
struct OpLog { uint64_t t; uint32_t addr; bool is_write; };
static std::vector<OpLog> op_log;
static bool op_log_enabled = false;

static void tick() {
    dut->CLK = 0;
    dut->eval();
    main_time++;
    dut->CLK = 1;
    dut->eval();
    main_time++;
    if (op_log_enabled && dut->dbg_op_capture_pulse) {
        op_log.push_back({main_time, dut->dbg_op_capture_addr, dut->dbg_op_capture_is_write != 0});
    }
}

static void tick_n(int n) { for (int i = 0; i < n; i++) tick(); }

static void clear_all_inputs() {
    dut->wr_strobe = 0;
    dut->addr0 = 0;
    dut->din = 0;
    dut->rd_done_strobe = 0;
    dut->bus_merq_n = 1;
    dut->new2 = 0;
    dut->msx_in_mcycle = 0;
    dut->msx_addr = 0;
    dut->msx_din = 0;
    dut->msx_is_write = 0;
    dut->bank_wr_stb = 0;
    dut->bank_wr_idx = 0;
    dut->bank_wr_data = 0;
}

static void do_reset() {
    dut->RESET_n = 0;
    dut->bus_reset_n = 0;
    clear_all_inputs();
    tick_n(20);
    dut->RESET_n = 1;
    dut->bus_reset_n = 1;
    tick_n(10);
}

// Wave register write: simula OUT &H7E,addr; OUT &H7F,data.
static void wave_reg_write(uint8_t reg, uint8_t data) {
    dut->addr0 = 0;
    dut->din = reg;
    dut->wr_strobe = 1;
    tick();
    dut->wr_strobe = 0;
    tick_n(2);
    dut->addr0 = 1;
    dut->din = data;
    dut->wr_strobe = 1;
    tick();
    dut->wr_strobe = 0;
    tick_n(20);
}

// Z80-like memory write. Default: bus_merq_n correctly = 0 during M-cycle.
static void z80_write(uint16_t addr, uint8_t data) {
    dut->msx_addr = addr;
    dut->msx_din = data;
    dut->msx_is_write = 1;
    dut->msx_in_mcycle = 1;
    dut->bus_merq_n = 0;
    tick_n(30);
    dut->msx_in_mcycle = 0;
    dut->bus_merq_n = 1;
    tick_n(15);
}

// Z80 write SIMULATING METAESTABILIDAD: fetch1 ve bus_merq_n=1, pero
// cartridge_ram (msx_in_mcycle) está activo. Reproduce el escenario
// async donde los dos FFs ven MERQ_n diferentemente.
static void z80_write_metastable(uint16_t addr, uint8_t data) {
    dut->msx_addr = addr;
    dut->msx_din = data;
    dut->msx_is_write = 1;
    dut->msx_in_mcycle = 1;       // cartridge_ram sees M-cycle.
    dut->bus_merq_n = 1;          // fetch1 thinks no M-cycle.
    tick_n(30);
    dut->msx_in_mcycle = 0;
    dut->bus_merq_n = 1;
    tick_n(15);
}

// Z80-like memory read.
static uint8_t z80_read(uint16_t addr) {
    dut->msx_addr = addr;
    dut->msx_din = 0;
    dut->msx_is_write = 0;
    dut->msx_in_mcycle = 1;
    dut->bus_merq_n = 0;
    tick_n(30);
    uint8_t got = dut->msx_dout;
    dut->msx_in_mcycle = 0;
    dut->bus_merq_n = 1;
    tick_n(15);
    return got;
}

// Set bank register
static void set_bank(uint8_t idx, uint8_t bank_val) {
    dut->bank_wr_idx = idx;
    dut->bank_wr_data = bank_val & 0x3F;
    dut->bank_wr_stb = 1;
    tick();
    dut->bank_wr_stb = 0;
    tick_n(2);
}

// Compute SDRAM addr for a given Z80 addr, given current banks.
// (Default banks: bank[0]=3, bank[1]=2, bank[2]=1, bank[3]=0).
static uint32_t z80_to_sdram_default(uint16_t addr) {
    uint8_t default_banks[4] = {3, 2, 1, 0};
    uint8_t bank_idx = (addr >> 14) & 3;
    return ((uint32_t)default_banks[bank_idx] << 14) + (addr & 0x3FFF);
}

// =====================================================================

static int run_s1_z80_basic() {
    printf("\n=== S1: Z80 write/read sin fetch1 ===\n");
    do_reset();

    // Write a pattern via Z80 to several pages.
    int errors = 0;
    std::vector<std::pair<uint16_t, uint8_t>> writes;
    for (int i = 0; i < 32; i++) {
        uint16_t addr = (i * 100) & 0xFFFF;
        uint8_t data = (i * 7 + 13) & 0xFF;
        z80_write(addr, data);
        writes.push_back({addr, data});
    }

    // Read back
    for (auto& w : writes) {
        uint8_t got = z80_read(w.first);
        if (got != w.second) {
            if (errors < 10)
                printf("  addr=0x%04x got=0x%02x expected=0x%02x\n",
                       w.first, got, w.second);
            errors++;
        }
    }
    printf("S1 result: %s (%d/%zu errors)\n",
           errors == 0 ? "PASS" : "FAIL", errors, writes.size());
    return errors;
}

static int run_s3_with_fetch1() {
    printf("\n=== S3: Z80 write/read CON fetch1 firing + metaestabilidad ===\n");
    do_reset();

    dut->new2 = 1;

    // OCT=7 para crecimiento rapido de phase_acc.
    wave_reg_write(0x20, 0x00);
    wave_reg_write(0x38, 0x70);

    // Key on → fetch1 fires
    wave_reg_write(0x68, 0x80);

    op_log.clear();
    op_log_enabled = true;

    // Test: escribir un patron LINEAL repetido N rondas a la misma zona
    // de mapper. Mas iteraciones = mas oportunidades de contention.
    // Last write to each addr es el data esperado.
    const int ROUNDS = 20;
    const int PER_ROUND = 256;
    std::vector<std::pair<uint16_t, uint8_t>> writes;
    for (int r = 0; r < ROUNDS; r++) {
        for (int i = 0; i < PER_ROUND; i++) {
            uint16_t addr = 0xC000 + i;
            uint8_t data = ((i + r * 13) ^ 0x55) & 0xFF;
            z80_write_metastable(addr, data);
            writes.push_back({addr, data});
        }
    }

    wave_reg_write(0x68, 0x00);
    tick_n(100);
    op_log_enabled = false;

    printf("  fetch1 OE_n=0 cycles: %u\n", dut->dbg_fetch1_fire_count);
    printf("  SDRAM ops triggered: %u\n", dut->dbg_sdram_op_count);
    printf("  Contention events (fetch1+cart simultaneously): %u\n", dut->dbg_contention_count);
    printf("  SDRAM total writes: %u\n", dut->dbg_write_count);
    printf("  SDRAM writes to mapper: %u\n", dut->dbg_writes_to_mapper);
    printf("  SDRAM writes to YRW801 (= corruptas): %u\n", dut->dbg_writes_to_yrw801);
    printf("  SDRAM op log: %zu entries. First 10:\n", op_log.size());
    for (size_t i = 0; i < op_log.size() && i < 10; i++) {
        printf("    t=%llu addr=0x%06x %s\n",
               (unsigned long long)op_log[i].t, op_log[i].addr,
               op_log[i].is_write ? "WR" : "RD");
    }
    // Detect ops with addr having bit 20 OR bit 18 set (= contention indicator).
    int bit20_set = 0, bit18_set = 0;
    int suspicious_writes = 0;
    printf("  Suspicious WRITES (= bit 20 set, would be YRW801):\n");
    for (auto& op : op_log) {
        if (op.addr & 0x100000) bit20_set++;
        if (op.addr & 0x040000) bit18_set++;
        if (op.is_write && (op.addr & 0x100000)) {
            if (suspicious_writes < 10) {
                printf("    t=%llu WR addr=0x%06x\n", (unsigned long long)op.t, op.addr);
            }
            suspicious_writes++;
        }
    }
    printf("  Ops with bit 20 set (= YRW801 area or corrupted): %d\n", bit20_set);
    printf("  Ops with bit 18 set: %d\n", bit18_set);
    printf("  Suspicious WRITES total: %d\n", suspicious_writes);

    // Verify last value for each addr.
    int errors = 0;
    int verified = 0;
    for (int i = 0; i < PER_ROUND; i++) {
        uint16_t addr = 0xC000 + i;
        uint8_t expected = ((i + (ROUNDS-1) * 13) ^ 0x55) & 0xFF;
        uint8_t got = z80_read(addr);
        verified++;
        if (got != expected) {
            if (errors < 20) {
                printf("  z80 addr=0x%04x → sdram=0x%06x got=0x%02x expected=0x%02x diff=0x%02x\n",
                       addr, z80_to_sdram_default(addr), got, expected, got ^ expected);
            }
            errors++;
        }
    }
    printf("S3 result: %d/%d corruptions detected (%d total writes)\n",
           errors, verified, (int)writes.size());
    return errors;
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    dut = new Vcontention_top;

    int total_errors = 0;
    total_errors += run_s1_z80_basic();
    total_errors += run_s3_with_fetch1();

    printf("\n========================================\n");
    printf("TOTAL ERRORS: %d\n", total_errors);
    printf("========================================\n");

    delete dut;
    return total_errors > 0 ? 1 : 0;
}
