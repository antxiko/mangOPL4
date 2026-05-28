// tb_r5b.cpp — valida header lookup de R5.b en fetch1.
//
// Para cada wave_num N, espera:
//   dbg_start_addr == 0x100000 + 0x1000 + N*0x100
//     N=0 -> 0x101000
//     N=1 -> 0x101100
//     N=2 -> 0x101200
//     N=10 -> 0x101A00
//   byte_a == start_addr[7:0]  (= 0x00 para estos N, porque offset acaba en 00)
//   byte_b == (start_addr+1)[7:0] = 0x01
//
// Si dbg_start_addr == 0 constante -> reproduce el bug HW.

#include <cstdio>
#include <cstdint>
#include "Vslot_r5b_top.h"
#include "verilated.h"

static Vslot_r5b_top* dut = nullptr;
static vluint64_t main_time = 0;

static void tick() {
    dut->CLK = 0; dut->eval(); main_time++;
    dut->CLK = 1; dut->eval(); main_time++;
}
static void tick_n(int n) { for (int i=0;i<n;i++) tick(); }

static void trace(const char* lbl) {
    printf("[t=%5llu %-10s] wave=%-3d fst=%-2d timing=%d start_addr=0x%06x byteA=0x%02x byteB=0x%02x\n",
        (unsigned long long)main_time, lbl, dut->wave_num,
        dut->dbg_fetch_state, dut->dbg_ram_timing,
        dut->dbg_start_addr, dut->dbg_byte_a, dut->dbg_byte_b);
}

static void dtrace(const char* lbl) {
    printf("[t=%5llu %-6s] fst=%-2d tim=%d oe=%d ack=%d addr=0x%06x needH=%d vH=%d hdr0=0x%02x hdr1=0x%02x start=0x%06x\n",
        (unsigned long long)main_time, lbl,
        dut->dbg_fetch_state, dut->dbg_ram_timing,
        dut->dbg_ram_oe_n, dut->dbg_ram_ack_n, dut->dbg_ram_addr,
        dut->dbg_need_header, dut->dbg_header_valid,
        dut->dbg_hdr0, dut->dbg_hdr1, dut->dbg_start_addr);
}

static void key_cycle(int wave, uint32_t& start_addr, uint8_t& ba, uint8_t& bb, bool verbose=false) {
    dut->wave_num = wave;
    dut->key_on = 0;
    tick_n(5);
    // key_on rising -> dispara header read
    dut->key_on = 1;
    tick();
    if (verbose) dtrace("KEYON");
    // sample_tick para avanzar phase (curr_idx)
    dut->sample_tick = 1; tick(); dut->sample_tick = 0;
    if (verbose) dtrace("TICK");
    // esperar a que fetch1 vuelva a IDLE varias veces (header + samples)
    for (int i=0;i<500;i++) {
        tick();
        if (verbose && i<40) dtrace("step");
    }
    start_addr = dut->dbg_start_addr;
    ba = dut->dbg_byte_a;
    bb = dut->dbg_byte_b;
    char lbl[16]; snprintf(lbl,sizeof(lbl),"w%d",wave);
    trace(lbl);
    dut->key_on = 0;
    tick_n(20);
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    dut = new Vslot_r5b_top;

    dut->RESET_n=0; dut->bus_reset_n=0; dut->sample_tick=0;
    dut->fnum=0; dut->octave=4; dut->key_on=0; dut->wave_num=0;
    tick_n(10);
    dut->RESET_n=1; dut->bus_reset_n=1;
    tick_n(20);

    printf("\n###### R5.b header lookup validation ######\n");
    int waves[] = {0,1,2,10,50,100};
    int npass=0, ntot=0;
    for (int wi=0; wi<6; wi++) {
        int w = waves[wi];
        uint32_t sa; uint8_t ba, bb;
        key_cycle(w, sa, ba, bb, wi==0);  // verbose en el primero
        // Stub devuelve addr[7:0]. header_base = 0x100000 + w*12.
        // start = 0x100000 + {hb[5:0], (hb+1)[7:0], (hb+2)[7:0]}.
        uint32_t hb = 0x100000 + w*12;
        uint32_t b0 = hb & 0xFF, b1 = (hb+1) & 0xFF, b2 = (hb+2) & 0xFF;
        uint32_t exp = 0x100000 + (((b0 & 0x3F) << 16) | (b1 << 8) | b2);
        ntot++;
        bool ok = (sa == exp);
        if (ok) npass++;
        printf("   wave=%-3d start_addr=0x%06x (exp 0x%06x) %s\n",
               w, sa, exp, ok?"OK":"FAIL");
    }
    printf("\n###### RESULTADO: %d/%d OK ######\n", npass, ntot);
    if (npass==0) printf("*** start_addr no varia -> bug reproducido en sim\n");
    else if (npass==ntot) printf("*** Header lookup CORRECTO en sim -> bug HW era timing/synth\n");

    delete dut;
    return 0;
}
