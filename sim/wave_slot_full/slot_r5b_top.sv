//
// slot_r5b_top.sv — top Verilator para validar header lookup de R5.b.
//
// ymf278_slot (con fetch1 que lee header de SDRAM) + sdram_stub_r5b.
// Expone wave_num como input y dbg_start_addr/byte_a/byte_b como salidas
// para que el TB verifique que el header lookup decodifica el start_addr
// correcto por wave_num.
//
`default_nettype none

module slot_r5b_top (
    input  wire        CLK,
    input  wire        RESET_n,
    input  wire        bus_reset_n,
    input  wire        sample_tick,

    input  wire [9:0]              fnum,
    input  wire signed [3:0]       octave,
    input  wire                    key_on,
    input  wire [8:0]              wave_num,

    output wire signed [15:0]      sample_out,
    output wire [31:0]             phase_acc_out,

    output wire [23:0]             dbg_start_addr,
    output wire [7:0]              dbg_byte_a,
    output wire [7:0]              dbg_byte_b,

    // Observables fetch1 FSM (via hierarchical)
    output wire [3:0]              dbg_fetch_state,
    output wire                    dbg_ram_timing,
    output wire                    dbg_need_header,
    output wire                    dbg_header_valid,
    output wire [7:0]              dbg_hdr0,
    output wire [7:0]              dbg_hdr1,
    output wire [23:0]             dbg_ram_addr,
    output wire                    dbg_ram_oe_n,
    output wire                    dbg_ram_ack_n
);

    RAM_IF Ram_slot();

    ymf278_slot u_slot (
        .RESET_n        (RESET_n),
        .CLK            (CLK),
        .bus_reset_n    (bus_reset_n),
        .sample_tick    (sample_tick),
        .fnum           (fnum),
        .octave         (octave),
        .key_on         (key_on),
        .wave_num       (wave_num),
        .sample_out     (sample_out),
        .phase_acc_out  (phase_acc_out),
        .dbg_start_addr (dbg_start_addr),
        .dbg_byte_a     (dbg_byte_a),
        .dbg_byte_b     (dbg_byte_b),
        .Ram            (Ram_slot)
    );

    sdram_stub_r5b u_sdram (
        .CLK     (CLK),
        .RESET_n (RESET_n),
        .Ram     (Ram_slot)
    );

    assign dbg_fetch_state  = u_slot.u_fetch1.state;
    assign dbg_ram_timing   = Ram_slot.TIMING;
    assign dbg_need_header  = u_slot.u_fetch1.need_header;
    assign dbg_header_valid = u_slot.u_fetch1.header_valid;
    assign dbg_hdr0         = u_slot.u_fetch1.hdr0;
    assign dbg_hdr1         = u_slot.u_fetch1.hdr1;
    assign dbg_ram_addr     = Ram_slot.ADDR;
    assign dbg_ram_oe_n     = Ram_slot.OE_n;
    assign dbg_ram_ack_n    = Ram_slot.ACK_n;

endmodule

`default_nettype wire
