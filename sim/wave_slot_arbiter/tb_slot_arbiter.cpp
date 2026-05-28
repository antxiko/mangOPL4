// tb_slot_arbiter.cpp — reproduce el escenario HW de R5.b:
// slot (nuevo fetch1 con header lookup) -> wave_arbiter -> sdram_top_arbiter
// (con Z80 stub contention) -> sdram_stub_r5b (DOUT = addr[7:0]).
//
// Objetivo: ver si CON la cadena de arbiters + Z80, el header lookup
// produce start_addr distinto por wave_num (como en el sim simple 6/6),
// o se rompe (reproduciendo el bug HW donde todos daban igual).
//
// Expected (stub addr[7:0]): para wave_num N, header_base=0x100000+N*12,
//   start_addr = 0x100000 + {hb[5:0], (hb+1)[7:0], (hb+2)[7:0]}.

#include <cstdio>
#include <cstdint>
#include "Vslot_arbiter_top.h"
#include "verilated.h"

static Vslot_arbiter_top* dut = nullptr;
static vluint64_t main_time = 0;

static void tick() {
    dut->CLK=0; dut->eval(); main_time++;
    dut->CLK=1; dut->eval(); main_time++;
}
static void tick_n(int n){ for(int i=0;i<n;i++) tick(); }

static void reset_dut() {
    dut->RESET_n=0; dut->bus_reset_n=0; dut->sample_tick=0;
    dut->fnum=0; dut->octave=4; dut->key_on=0; dut->wave_num=0;
    dut->z80_stim_active=0; dut->z80_stim_addr=0; dut->z80_cycles_on=50; dut->z80_cycles_off=5;
    dut->memp_pulse_fire=0; dut->memp_pulse_addr=0x100040;
    tick_n(10);
    dut->RESET_n=1; dut->bus_reset_n=1;
    tick_n(20);
}

static void key_cycle(int wave, uint32_t& sa, uint8_t& ba, uint8_t& bb) {
    dut->wave_num = wave;
    dut->key_on = 0; tick_n(5);
    dut->key_on = 1; tick();
    dut->sample_tick=1; tick(); dut->sample_tick=0;
    tick_n(800);  // mas margen por contention Z80
    sa = dut->dbg_start_addr; ba = dut->dbg_fetch_byte_a; bb = dut->dbg_fetch_byte_b;
    dut->key_on = 0; tick_n(30);
}

static void run_scenario(const char* name, bool z80) {
    printf("\n###### %s (Z80=%d) ######\n", name, z80);
    reset_dut();
    dut->z80_stim_active = z80;
    dut->z80_stim_addr   = 0x000000;
    dut->z80_cycles_on   = 50;
    dut->z80_cycles_off  = 5;

    int waves[] = {0,1,2,10};
    int npass=0;
    for (int i=0;i<4;i++) {
        int w=waves[i]; uint32_t sa; uint8_t ba,bb;
        key_cycle(w, sa, ba, bb);
        uint32_t hb = 0x100000 + w*12;
        uint32_t b0=hb&0xFF,b1=(hb+1)&0xFF,b2=(hb+2)&0xFF;
        uint32_t exp = 0x100000 + (((b0&0x3F)<<16)|(b1<<8)|b2);
        bool ok = (sa==exp);
        if(ok) npass++;
        printf("  wave=%-3d start=0x%06x (exp 0x%06x) bA=0x%02x bB=0x%02x %s\n",
               w, sa, exp, ba, bb, ok?"OK":"FAIL");
    }
    printf("  -> %d/4 OK\n", npass);
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    dut = new Vslot_arbiter_top;
    run_scenario("A: sin Z80 (control)", false);
    run_scenario("B: con Z80 90%% duty (= escenario HW)", true);
    printf("\n###### FIN ######\n");
    delete dut;
    return 0;
}
