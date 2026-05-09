//
// arbiter_smoke_top.sv — top para Verilator testbench.
//
// Instancia: ymf278_mempointer real + wave_arbiter (versión bajo
// test) + sdram_stub. Expone como puertos los signals de control
// que el testbench drivea (reg_wr_stb, reg_addr, reg_data, ...) y
// observables (mem_data_byte, etc.).
//
`default_nettype none

module arbiter_smoke_top (
    input  wire        CLK,
    input  wire        RESET_n,
    input  wire        bus_reset_n,

    input  wire        reg_wr_stb,
    input  wire [7:0]  reg_addr,
    input  wire [7:0]  reg_data,
    input  wire        reg_rd_done_stb,

    input  wire        bus_merq_n,
    input  wire        key_on_slot0,
    input  wire [7:0]  cache_index,

    output wire [7:0]  mem_data_byte,
    output wire signed [15:0] playback_sample,

    // Observables del bus para el testbench
    output wire [23:0] dbg_primary_addr,
    output wire        dbg_primary_we_n,
    output wire        dbg_primary_oe_n,
    output wire        dbg_primary_timing,
    output wire        dbg_primary_ack_n,

    // Internals del SDRAM stub (para debug)
    output wire [6:0]  dbg_sdram_state,
    output wire [23:0] dbg_sdram_save_addr,
    output wire [3:0]  dbg_sdram_save_dqm,

    // Internals del mempointer (para debug)
    output wire [23:0] dbg_busa_addr,
    output wire        dbg_busa_we_n,
    output wire        dbg_busa_ack_n
);

    RAM_IF Ram_primary();
    RAM_IF Ram_busa();
    RAM_IF Ram_busb();

    // BusB (fetch1) tied off
    assign Ram_busb.ADDR     = 24'h0;
    assign Ram_busb.DIN      = 32'h0;
    assign Ram_busb.DIN_SIZE = 3'b000;
    assign Ram_busb.OE_n     = 1'b1;
    assign Ram_busb.WE_n     = 1'b1;
    assign Ram_busb.RFSH_n   = 1'b1;

    // mempointer real
    ymf278_mempointer u_mempointer (
        .RESET_n        (RESET_n),
        .CLK            (CLK),
        .bus_reset_n    (bus_reset_n),
        .reg_wr_stb     (reg_wr_stb),
        .reg_addr       (reg_addr),
        .reg_data       (reg_data),
        .reg_rd_done_stb(reg_rd_done_stb),
        .bus_merq_n     (bus_merq_n),
        .key_on_slot0   (key_on_slot0),
        .cache_index    (cache_index),
        .mem_data_byte  (mem_data_byte),
        .playback_sample(playback_sample),
        .Ram            (Ram_busa)
    );

    // Arbiter bajo test
    wave_arbiter u_arbiter (
        .RESET_n    (RESET_n),
        .CLK        (CLK),
        .Primary    (Ram_primary),
        .BusA       (Ram_busa),
        .BusB       (Ram_busb)
    );

    // SDRAM stub
    sdram_stub u_sdram (
        .CLK     (CLK),
        .RESET_n (RESET_n),
        .Ram     (Ram_primary)
    );

    // Observables para el testbench
    assign dbg_primary_addr   = Ram_primary.ADDR;
    assign dbg_primary_we_n   = Ram_primary.WE_n;
    assign dbg_primary_oe_n   = Ram_primary.OE_n;
    assign dbg_primary_timing = Ram_primary.TIMING;
    assign dbg_primary_ack_n  = Ram_primary.ACK_n;

    assign dbg_sdram_state     = u_sdram.state;
    assign dbg_sdram_save_addr = u_sdram.save_addr;
    assign dbg_sdram_save_dqm  = u_sdram.save_dqm;

    assign dbg_busa_addr  = Ram_busa.ADDR;
    assign dbg_busa_we_n  = Ram_busa.WE_n;
    assign dbg_busa_ack_n = Ram_busa.ACK_n;

endmodule

`default_nettype wire
