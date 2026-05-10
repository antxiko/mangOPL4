//
// arbiter_smoke_top.sv — top Verilator para validar sdram_top_arbiter.
//
// Estructura:
//   mock_a_cart  → BusA (RAM_IF)
//   mock_b_wave  → BusB (RAM_IF)
//   sdram_top_arbiter (SUT) → Primary (RAM_IF) → sdram_stub
//
// Expone al testbench (C++) controles de cada mock + observables.
//
`default_nettype none

module arbiter_smoke_top (
    input  wire        CLK,
    input  wire        RESET_n,

    // Control mock A (cart-style)
    input  wire        a_trigger,
    input  wire        a_is_write,
    input  wire [23:0] a_addr,
    input  wire [7:0]  a_din,
    output wire [7:0]  a_last_dout,

    // Control mock B (wave-style)
    input  wire        b_trigger,
    input  wire [23:0] b_addr,
    output wire [7:0]  b_captured_byte,
    output wire        b_done,

    // Observables del bus
    output wire        dbg_grant_a,
    output wire        dbg_grant_b,
    output wire [23:0] dbg_primary_addr,
    output wire        dbg_primary_oe_n,
    output wire        dbg_primary_we_n,
    output wire        dbg_primary_ack_n,
    output wire        dbg_primary_timing,
    output wire        dbg_busa_timing,
    output wire        dbg_busb_timing,

    // SDRAM observables
    output wire [31:0] dbg_write_count,
    output wire [31:0] dbg_read_count,
    output wire        dbg_op_capture_pulse,
    output wire [23:0] dbg_op_capture_addr,
    output wire        dbg_op_capture_is_write
);

    RAM_IF BusA();
    RAM_IF BusB();
    RAM_IF Primary();

    mock_a_cart u_mock_a (
        .CLK       (CLK),
        .RESET_n   (RESET_n),
        .trigger   (a_trigger),
        .is_write  (a_is_write),
        .addr      (a_addr),
        .din       (a_din),
        .last_dout (a_last_dout),
        .Ram       (BusA)
    );

    mock_b_wave u_mock_b (
        .CLK            (CLK),
        .RESET_n        (RESET_n),
        .trigger        (b_trigger),
        .addr           (b_addr),
        .captured_byte  (b_captured_byte),
        .done           (b_done),
        .Ram            (BusB)
    );

    sdram_top_arbiter u_arbiter (
        .RESET_n (RESET_n),
        .CLK     (CLK),
        .Primary (Primary),
        .BusA    (BusA),
        .BusB    (BusB)
    );

    sdram_stub u_sdram (
        .CLK                     (CLK),
        .RESET_n                 (RESET_n),
        .dbg_write_count         (dbg_write_count),
        .dbg_read_count          (dbg_read_count),
        .dbg_op_capture_pulse    (dbg_op_capture_pulse),
        .dbg_op_capture_addr     (dbg_op_capture_addr),
        .dbg_op_capture_is_write (dbg_op_capture_is_write),
        .Ram                     (Primary)
    );

    // Observables del arbiter (sondear por path)
    assign dbg_grant_a        = u_arbiter.active_a;
    assign dbg_grant_b        = u_arbiter.active_b;
    assign dbg_primary_addr   = Primary.ADDR;
    assign dbg_primary_oe_n   = Primary.OE_n;
    assign dbg_primary_we_n   = Primary.WE_n;
    assign dbg_primary_ack_n  = Primary.ACK_n;
    assign dbg_primary_timing = Primary.TIMING;
    assign dbg_busa_timing    = BusA.TIMING;
    assign dbg_busb_timing    = BusB.TIMING;

endmodule

`default_nettype wire
