// tb_slot_arbiter.cpp — TB del slot 0 con cadena de arbitración completa.
//
// 4 escenarios incrementales:
//
//   Scenario A: Z80=0, Mempointer=0
//      → control: fetch1 debe funcionar IGUAL que en sim/wave_slot_full
//        (los arbiters pasan transparente cuando solo hay 1 host).
//
//   Scenario B: Z80=alta carga (90% duty), Mempointer=0
//      → reproduce el caso típico HW: MSX corriendo BASIC, cartridge_ram
//        martillando el bus. fetch1 debe poder colarse en los huecos.
//        Si fetch_complete_count = 0 → starvation. Si byte_a/b correctos
//        → arbiter funciona OK pese a la contention.
//
//   Scenario C: Z80=0, Mempointer pulsing cada 100 cycles
//      → reproduce mempointer firing prefetches en background mientras
//        slot fetch. wave_arbiter da priority A=mempointer → fetch1
//        retarda 1 op. Debe completar igualmente.
//
//   Scenario D: Z80 + Mempointer ambos activos
//      → caos. Si fetch1 sobrevive a esto, lógica robusta.

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include "Vslot_arbiter_top.h"
#include "verilated.h"

static Vslot_arbiter_top* dut = nullptr;
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
    printf("[t=%5llu %-12s] fst=%d lidx=%04x ba=%02x bb=%02x | sdram_addr=%06x oe=%d ack=%d tim=%d | wA=%d wB=%d tA=%d tB=%d\n",
        (unsigned long long)main_time, label,
        dut->dbg_fetch_state, dut->dbg_fetch_last_idx,
        dut->dbg_fetch_byte_a, dut->dbg_fetch_byte_b,
        dut->dbg_sdram_addr, dut->dbg_sdram_oe_n, dut->dbg_sdram_ack_n, dut->dbg_sdram_timing,
        dut->dbg_wave_active_a, dut->dbg_wave_active_b,
        dut->dbg_top_active_a, dut->dbg_top_active_b);
}

static void sample_tick_pulse() {
    dut->sample_tick = 1;
    tick();
    dut->sample_tick = 0;
}

static bool wait_fetch_complete(int max_cycles) {
    for (int i = 0; i < max_cycles; i++) {
        tick();
        if (dut->dbg_fetch_state == 0) return true;
    }
    return false;
}

static void reset_dut() {
    dut->RESET_n     = 0;
    dut->bus_reset_n = 0;
    dut->sample_tick = 0;
    dut->fnum        = 0;
    dut->octave      = 4;
    dut->key_on      = 0;
    dut->start_addr_sdram_in = 0x100000;
    dut->z80_stim_active     = 0;
    dut->z80_stim_addr       = 0x000000;
    dut->z80_cycles_on       = 50;
    dut->z80_cycles_off      = 5;
    dut->memp_pulse_fire     = 0;
    dut->memp_pulse_addr     = 0x100040;
    tick_n(10);
    dut->RESET_n     = 1;
    dut->bus_reset_n = 1;
    tick_n(20);
}

// Test 1 key_on con la configuración actual de stims. Devuelve los bytes.
static void key_cycle(uint32_t start_addr, uint8_t& byte_a, uint8_t& byte_b,
                       const char* label, int max_wait = 2000) {
    dut->start_addr_sdram_in = start_addr;
    printf("\n=== %s: start_addr=0x%06x ===\n", label, start_addr);
    trace("PRE-keyon");

    dut->key_on = 1;
    tick();
    trace("KEYON");

    sample_tick_pulse();
    tick_n(5);
    trace("POST-TICK");

    if (!wait_fetch_complete(max_wait)) {
        printf("  TIMEOUT esperando fetch (state stuck en %d)\n", dut->dbg_fetch_state);
    }
    trace("FETCH-DONE");

    byte_a = dut->dbg_fetch_byte_a;
    byte_b = dut->dbg_fetch_byte_b;

    dut->key_on = 0;
    tick_n(50);
    trace("KEYOFF");
}

static void scenario_A() {
    printf("\n\n############ Scenario A: solo slot (control) ############\n");
    reset_dut();
    dut->z80_stim_active = 0;
    dut->memp_pulse_fire = 0;

    uint8_t ba1, bb1, ba2, bb2, ba3, bb3;
    key_cycle(0x100000, ba1, bb1, "A1 (sample1)");
    tick_n(50);
    key_cycle(0x100080, ba2, bb2, "A2 (sample2)");
    tick_n(50);
    key_cycle(0x1000C0, ba3, bb3, "A3 (sample3)");

    printf("\n--- RESUMEN A ---\n");
    printf("A1: ba=%02x bb=%02x (exp 01/02) %s\n", ba1, bb1, (ba1==0x01&&bb1==0x02)?"OK":"FAIL");
    printf("A2: ba=%02x bb=%02x (exp 81/82) %s\n", ba2, bb2, (ba2==0x81&&bb2==0x82)?"OK":"FAIL");
    printf("A3: ba=%02x bb=%02x (exp C1/C2) %s\n", ba3, bb3, (ba3==0xC1&&bb3==0xC2)?"OK":"FAIL");
}

static void scenario_B() {
    printf("\n\n############ Scenario B: Z80 con duty 90%% ############\n");
    reset_dut();
    dut->z80_stim_active = 1;
    dut->z80_stim_addr   = 0x000000;     // region mapper
    dut->z80_cycles_on   = 50;            // 50 cycles OE_n=0 (= ~M-cycle)
    dut->z80_cycles_off  = 5;             // 5 cycles gap (90% duty)
    dut->memp_pulse_fire = 0;

    uint8_t ba1, bb1, ba2, bb2, ba3, bb3;
    key_cycle(0x100000, ba1, bb1, "B1 (sample1) +Z80", 5000);
    tick_n(50);
    key_cycle(0x100080, ba2, bb2, "B2 (sample2) +Z80", 5000);
    tick_n(50);
    key_cycle(0x1000C0, ba3, bb3, "B3 (sample3) +Z80", 5000);

    printf("\n--- RESUMEN B ---\n");
    printf("B1: ba=%02x bb=%02x (exp 01/02) %s\n", ba1, bb1, (ba1==0x01&&bb1==0x02)?"OK":"FAIL");
    printf("B2: ba=%02x bb=%02x (exp 81/82) %s\n", ba2, bb2, (ba2==0x81&&bb2==0x82)?"OK":"FAIL");
    printf("B3: ba=%02x bb=%02x (exp C1/C2) %s\n", ba3, bb3, (ba3==0xC1&&bb3==0xC2)?"OK":"FAIL");
}

static void scenario_C() {
    printf("\n\n############ Scenario C: Mempointer pulsing ############\n");
    reset_dut();
    dut->z80_stim_active = 0;
    dut->memp_pulse_addr = 0x100040;

    // Para test simple: pulsar memp en mitad de la sequencia.
    uint8_t ba1, bb1;
    dut->start_addr_sdram_in = 0x100000;
    printf("\n=== C1 (sample1) con memp pulse en medio ===\n");
    dut->key_on = 1;
    tick();
    sample_tick_pulse();
    // Pulse mempointer ANTES de que fetch1 complete
    dut->memp_pulse_fire = 1;
    tick();
    dut->memp_pulse_fire = 0;
    trace("MEMP-FIRED");
    if (!wait_fetch_complete(5000)) {
        printf("  TIMEOUT esperando fetch1\n");
    }
    trace("FETCH-DONE");
    ba1 = dut->dbg_fetch_byte_a;
    bb1 = dut->dbg_fetch_byte_b;
    dut->key_on = 0;
    tick_n(50);

    printf("\n--- RESUMEN C ---\n");
    printf("C1: ba=%02x bb=%02x (exp 01/02) %s\n", ba1, bb1, (ba1==0x01&&bb1==0x02)?"OK":"FAIL");
    printf("Memp last byte=%02x valid=%d\n", dut->memp_last_byte, dut->memp_last_valid);
}

static void scenario_D() {
    printf("\n\n############ Scenario D: Z80 + Mempointer caos ############\n");
    reset_dut();
    dut->z80_stim_active = 1;
    dut->z80_stim_addr   = 0x000000;
    dut->z80_cycles_on   = 30;
    dut->z80_cycles_off  = 10;            // 75% duty (menos agresivo)
    dut->memp_pulse_addr = 0x100040;

    uint8_t ba1, bb1, ba2, bb2;
    dut->start_addr_sdram_in = 0x100000;
    printf("\n=== D1 (sample1) +Z80 +memp ===\n");
    dut->key_on = 1;
    tick();
    sample_tick_pulse();
    dut->memp_pulse_fire = 1;
    tick();
    dut->memp_pulse_fire = 0;
    if (!wait_fetch_complete(10000)) {
        printf("  TIMEOUT D1 (state=%d)\n", dut->dbg_fetch_state);
    }
    ba1 = dut->dbg_fetch_byte_a;
    bb1 = dut->dbg_fetch_byte_b;
    dut->key_on = 0;
    tick_n(100);

    dut->start_addr_sdram_in = 0x100080;
    printf("\n=== D2 (sample2) +Z80 +memp ===\n");
    dut->key_on = 1;
    tick();
    sample_tick_pulse();
    dut->memp_pulse_fire = 1;
    tick();
    dut->memp_pulse_fire = 0;
    if (!wait_fetch_complete(10000)) {
        printf("  TIMEOUT D2 (state=%d)\n", dut->dbg_fetch_state);
    }
    ba2 = dut->dbg_fetch_byte_a;
    bb2 = dut->dbg_fetch_byte_b;
    dut->key_on = 0;
    tick_n(100);

    printf("\n--- RESUMEN D ---\n");
    printf("D1: ba=%02x bb=%02x (exp 01/02) %s\n", ba1, bb1, (ba1==0x01&&bb1==0x02)?"OK":"FAIL");
    printf("D2: ba=%02x bb=%02x (exp 81/82) %s\n", ba2, bb2, (ba2==0x81&&bb2==0x82)?"OK":"FAIL");
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    dut = new Vslot_arbiter_top;

    scenario_A();
    scenario_B();
    scenario_C();
    scenario_D();

    printf("\n############ FIN ############\n");

    delete dut;
    return 0;
}
