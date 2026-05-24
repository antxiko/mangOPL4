//
// slot_full_top.sv — top para Verilator testbench del slot completo.
//
// Instancia: ymf278_slot real + sdram_stub_yrw. Expone como puertos
// los inputs que el TB drivea (FN, OCT, key_on, start_addr) y outputs
// observables (sample_out, phase_acc, fetch1 internals via
// hierarchical access en TB).
//
`default_nettype none

module slot_full_top (
    input  wire        CLK,
    input  wire        RESET_n,
    input  wire        bus_reset_n,
    input  wire        sample_tick,

    input  wire [9:0]              fnum,
    input  wire signed [3:0]       octave,
    input  wire                    key_on,
    input  wire [23:0]             start_addr_sdram_in,

    output wire signed [15:0]      sample_out,
    output wire [31:0]             phase_acc_out,

    // Observables del fetch1 (via hierarchical en SV)
    output wire [2:0]              dbg_fetch_state,
    output wire [15:0]             dbg_fetch_last_idx,
    output wire [7:0]              dbg_fetch_byte_a,
    output wire [7:0]              dbg_fetch_byte_b,

    // Observables del bus
    output wire [23:0]             dbg_ram_addr,
    output wire                    dbg_ram_oe_n,
    output wire                    dbg_ram_ack_n,
    output wire                    dbg_ram_timing
);

    RAM_IF Ram_slot();

    // El slot real (R5 v0 = con start_addr_sdram_in input)
    ymf278_slot u_slot (
        .RESET_n                (RESET_n),
        .CLK                    (CLK),
        .bus_reset_n            (bus_reset_n),
        .sample_tick            (sample_tick),
        .fnum                   (fnum),
        .octave                 (octave),
        .key_on                 (key_on),
        .start_addr_sdram_in    (start_addr_sdram_in),
        .sample_out             (sample_out),
        .phase_acc_out          (phase_acc_out),
        .Ram                    (Ram_slot)
    );

    // SDRAM stub directo (sin wave_arbiter — slot es el único host)
    sdram_stub_yrw u_sdram (
        .CLK     (CLK),
        .RESET_n (RESET_n),
        .Ram     (Ram_slot)
    );

    // Observables fetch1 via hierarchical
    assign dbg_fetch_state    = u_slot.u_fetch1.state;
    assign dbg_fetch_last_idx = u_slot.u_fetch1.last_idx_fetched;
    assign dbg_fetch_byte_a   = u_slot.u_fetch1.byte_a;
    assign dbg_fetch_byte_b   = u_slot.u_fetch1.byte_b;

    assign dbg_ram_addr   = Ram_slot.ADDR;
    assign dbg_ram_oe_n   = Ram_slot.OE_n;
    assign dbg_ram_ack_n  = Ram_slot.ACK_n;
    assign dbg_ram_timing = Ram_slot.TIMING;

endmodule

`default_nettype wire
