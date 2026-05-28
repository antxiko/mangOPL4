// tb_top.cpp — reproduce el path MSX completo sobre ymf278_top.
// Escribe regs (NEW2, wave_num, key_on) y lee debug regs 0xD0-D6,
// igual que el script r5btest.asc. Verifica si el debug reg start_addr
// (0xD0-D2) refleja un start_addr que varía con wave_num.

#include <cstdio>
#include <cstdint>
#include "Vtop_test_top.h"
#include "verilated.h"

static Vtop_test_top* dut = nullptr;
static vluint64_t main_time = 0;
static int clk_opl3_div = 0;

static void tick() {
    dut->CLK = 0;
    // CLK_OPL3 ~ CLK/3 (no crítico para header lookup en CLK domain)
    if (++clk_opl3_div >= 3) { clk_opl3_div = 0; dut->CLK_OPL3 = !dut->CLK_OPL3; }
    dut->eval(); main_time++;
    dut->CLK = 1; dut->eval(); main_time++;
}
static void tick_n(int n){ for(int i=0;i<n;i++) tick(); }

// Escribe registro R con valor V (simula OUT 7E,R : OUT 7F,V).
static void write_reg(uint8_t r, uint8_t v) {
    // select reg (7E): addr0=0, din=R, wr_strobe pulse
    dut->addr0 = 0; dut->din = r; dut->wr_strobe = 1; tick();
    dut->wr_strobe = 0; tick();
    // data (7F): addr0=1, din=V, wr_strobe pulse
    dut->addr0 = 1; dut->din = v; dut->wr_strobe = 1; tick();
    dut->wr_strobe = 0; tick();
    tick_n(4);
}

// Lee registro R (simula OUT 7E,R : INP 7F). rd_data es combinacional
// sobre reg_addr_latch (= se setea con el select).
static uint8_t read_reg(uint8_t r) {
    dut->addr0 = 0; dut->din = r; dut->wr_strobe = 1; tick();
    dut->wr_strobe = 0; tick();
    tick_n(2);
    return dut->rd_data;
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    dut = new Vtop_test_top;

    dut->RESET_n=0; dut->bus_reset_n=0; dut->CLK_OPL3=0; dut->new2=1;
    dut->wr_strobe=0; dut->addr0=0; dut->din=0;
    dut->rd_done_strobe=0; dut->bus_merq_n=1;
    tick_n(20);
    dut->RESET_n=1; dut->bus_reset_n=1;
    tick_n(40);

    printf("\n###### top-level path MSX: debug reg start_addr ######\n");
    int waves[] = {0,1,2,10,50,100};
    int npass=0;
    for (int wi=0; wi<6; wi++) {
        int w = waves[wi];
        write_reg(0x08, w & 0xFF);     // wave_num low
        write_reg(0x20, 0x00);         // wave_num[8]=0, FN low=0
        write_reg(0x68, 0x80);         // key_on -> dispara header read
        tick_n(2000);                  // esperar header read + sample fetch

        uint8_t sa0 = read_reg(0xD0);
        uint8_t sa1 = read_reg(0xD1);
        uint8_t sa2 = read_reg(0xD2);
        uint8_t ba  = read_reg(0xD3);
        uint8_t bb  = read_reg(0xD4);
        uint8_t wn  = read_reg(0xD6);
        uint32_t sa = (sa2<<16)|(sa1<<8)|sa0;

        uint32_t hb = 0x100000 + w*12;
        uint32_t b0=hb&0xFF,b1=(hb+1)&0xFF,b2=(hb+2)&0xFF;
        uint32_t exp = 0x100000 + (((b0&0x3F)<<16)|(b1<<8)|b2);
        bool ok = (sa==exp);
        if(ok) npass++;
        printf("  wave=%-3d(rd=%-3d) start=0x%06x (exp 0x%06x) bA=0x%02x bB=0x%02x %s\n",
               w, wn, sa, exp, ba, bb, ok?"OK":"FAIL");

        write_reg(0x68, 0x00);         // key_off
        tick_n(40);
    }
    printf("\n###### %d/6 OK ######\n", npass);
    if (npass==0) printf("*** debug reg start_addr NO varia -> bug reproducido en top sim\n");
    else if (npass==6) printf("*** top path OK en sim -> bug HW es timing/synth/SDRAM real\n");

    delete dut;
    return 0;
}
