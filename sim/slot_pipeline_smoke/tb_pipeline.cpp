// tb_pipeline.cpp — testbench Verilator para ymf278_slot_pipeline (2c.3.b).
//
// Escenarios:
//   (a) Reset: phase_acc_out=0.
//   (b) key_on edge 0→1: phase_acc se resetea a 0.
//   (c) sample_tick con key_on=1: phase_acc avanza phase_inc por tick.
//       Con FNUM=0, OCT=0: phase_inc = (1<<10) + 0 = 1024. Tras N ticks,
//       phase_acc debería ser N*1024.
//   (d) key_on=0: phase_acc se queda fijo (hold).
//   (e) Re-key_on edge: phase_acc se resetea a 0.
//
// Pipeline latency: 3 ciclos entre sample_tick y phase_acc actualizado
// (stages 0, 1, 2). Esperamos suficientes ticks entre tick y check.

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include "Vpipeline_smoke_top.h"
#include "verilated.h"

static Vpipeline_smoke_top* dut = nullptr;
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
    dut->sample_tick = 0;
    dut->fnum        = 0;
    dut->octave      = 0;
    dut->key_on      = 0;
}

// Emite sample_tick (1 pulse) y espera 10 ciclos para que el pipeline
// complete sus 8 stages + margen. Declarado antes de do_reset porque
// do_reset lo usa para limpiar el state BSRAM.
static void pulse_sample_tick_internal() {
    dut->sample_tick = 1;
    dut->CLK = 0; dut->eval(); main_time++;
    dut->CLK = 1; dut->eval(); main_time++;
    dut->sample_tick = 0;
    for (int i = 0; i < 10; i++) {
        dut->CLK = 0; dut->eval(); main_time++;
        dut->CLK = 1; dut->eval(); main_time++;
    }
}

static void do_reset() {
    dut->RESET_n = 0;
    clear_inputs();
    tick_n(10);
    dut->RESET_n = 1;
    tick_n(10);
    // CRÍTICO: limpia state BSRAM (mem persiste tras RESET_n igual que
    // hardware Gowin BSRAM). Pulse con key_on=0 escribe mem[0] con
    // key_on_prev=0, permitiendo que el próximo key_on=1 dispare el
    // edge detect correctamente.
    dut->key_on = 0;
    pulse_sample_tick_internal();
    tick_n(5);
}

// Emite sample_tick (1 pulse) y espera 10 ciclos para que el pipeline
// complete sus 8 stages + margen.
static void pulse_sample_tick() {
    dut->sample_tick = 1;
    tick();
    dut->sample_tick = 0;
    tick_n(10);
}

static int errors_total = 0;
#define CHECK(cond, fmt, ...) do { \
    if (!(cond)) { \
        printf("  FAIL: " fmt "\n", ##__VA_ARGS__); \
        errors_total++; \
    } \
} while(0)

static int test_reset_phase_zero() {
    printf("\n=== (a) Reset → phase_acc=0 ===\n");
    do_reset();
    int errors = errors_total;
    CHECK(dut->phase_acc_out == 0,
          "phase_acc_out=0x%x esperado 0", dut->phase_acc_out);
    int new_errors = errors_total - errors;
    printf("(a) %s\n", new_errors == 0 ? "PASS" : "FAIL");
    return new_errors;
}

static int test_keyon_edge_resets() {
    printf("\n=== (b) key_on edge 0→1 → phase_acc resetea ===\n");
    do_reset();
    int errors = errors_total;
    // Inicialmente key_on=0
    dut->key_on = 0;
    pulse_sample_tick();
    // Edge a 1
    dut->key_on = 1;
    pulse_sample_tick();
    CHECK(dut->phase_acc_out == 0,
          "tras edge phase_acc=0x%x esperado 0", dut->phase_acc_out);
    int new_errors = errors_total - errors;
    printf("(b) %s\n", new_errors == 0 ? "PASS" : "FAIL");
    return new_errors;
}

static int test_increment() {
    printf("\n=== (c) sample_tick + key_on=1 → phase_acc += phase_inc ===\n");
    do_reset();
    int errors = errors_total;
    dut->fnum   = 0;
    dut->octave = 0;
    // phase_inc = (1<<10 | 0) << 0 = 1024
    uint32_t expected_phase_inc = 1024;

    // Edge a 1 (primer pulse resetea phase_acc)
    dut->key_on = 1;
    pulse_sample_tick();
    CHECK(dut->phase_acc_out == 0,
          "post-edge phase_acc=0x%x esperado 0", dut->phase_acc_out);

    // Pulses consecutivos: phase_acc += phase_inc cada vez
    for (int i = 1; i <= 5; i++) {
        pulse_sample_tick();
        uint32_t expected = i * expected_phase_inc;
        CHECK(dut->phase_acc_out == expected,
              "tras %d ticks phase_acc=0x%x esperado 0x%x",
              i, dut->phase_acc_out, expected);
    }
    int new_errors = errors_total - errors;
    printf("(c) %s\n", new_errors == 0 ? "PASS" : "FAIL");
    return new_errors;
}

static int test_keyoff_holds() {
    printf("\n=== (d) key_on=0 → phase_acc hold ===\n");
    do_reset();
    int errors = errors_total;
    dut->fnum   = 0;
    dut->octave = 0;

    dut->key_on = 1;
    pulse_sample_tick(); // reset
    pulse_sample_tick(); // +1024
    pulse_sample_tick(); // +1024
    pulse_sample_tick(); // +1024 → 3072

    uint32_t held = dut->phase_acc_out;
    CHECK(held == 3072,
          "antes de key_off phase_acc=0x%x esperado 0x%x", held, 3072);

    // Key off
    dut->key_on = 0;
    pulse_sample_tick();
    CHECK(dut->phase_acc_out == held,
          "key_off phase_acc=0x%x esperado 0x%x (hold)",
          dut->phase_acc_out, held);
    pulse_sample_tick();
    CHECK(dut->phase_acc_out == held,
          "key_off+1 phase_acc=0x%x esperado 0x%x (hold)",
          dut->phase_acc_out, held);
    int new_errors = errors_total - errors;
    printf("(d) %s\n", new_errors == 0 ? "PASS" : "FAIL");
    return new_errors;
}

static int test_rekeyon_resets() {
    printf("\n=== (e) Re-key_on edge → phase_acc resetea ===\n");
    do_reset();
    int errors = errors_total;
    dut->fnum   = 0;
    dut->octave = 0;

    dut->key_on = 1;
    pulse_sample_tick(); pulse_sample_tick(); pulse_sample_tick();
    CHECK(dut->phase_acc_out == 2048,
          "tras 3 ticks phase_acc=0x%x esperado 2048", dut->phase_acc_out);

    dut->key_on = 0;
    pulse_sample_tick();
    dut->key_on = 1; // re-edge
    pulse_sample_tick();
    CHECK(dut->phase_acc_out == 0,
          "re-key_on phase_acc=0x%x esperado 0", dut->phase_acc_out);
    int new_errors = errors_total - errors;
    printf("(e) %s\n", new_errors == 0 ? "PASS" : "FAIL");
    return new_errors;
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    dut = new Vpipeline_smoke_top;

    test_reset_phase_zero();
    test_keyon_edge_resets();
    test_increment();
    test_keyoff_holds();
    test_rekeyon_resets();

    printf("\n========================================\n");
    printf("TOTAL ERRORS: %d\n", errors_total);
    printf("========================================\n");

    delete dut;
    return errors_total > 0 ? 1 : 0;
}
