//
// pipeline_smoke_top.sv — top Verilator para validar ymf278_slot_pipeline.
//
// Parametrizable por ACTIVE_SLOTS. En 2c.3.c se usa 2.
//
`default_nettype none

module pipeline_smoke_top
    import ymf278_pkg::*;
#(
    parameter int ACTIVE_SLOTS = 2
) (
    input  wire                        CLK,
    input  wire                        RESET_n,

    // Stimuli (= mux externo simulado por el tb)
    input  wire                        sample_tick,
    input  wire [9:0]                  fnum,
    input  wire signed [3:0]           octave,
    input  wire                        key_on,

    // Observables
    output wire [PHASE_WIDTH-1:0]      phase_acc_out_slot0,
    output wire [STATE_ADDR_BITS-1:0]  current_slot
);

    // Wires entre pipeline y BSRAM
    wire [STATE_ADDR_BITS-1:0]         state_read_addr;
    wire [STATE_BITS_PER_SLOT-1:0]     state_read_data;
    wire [STATE_ADDR_BITS-1:0]         state_write_addr;
    wire [STATE_BITS_PER_SLOT-1:0]     state_write_data;
    wire                               state_write_en;

    ymf278_slot_pipeline #(
        .ACTIVE_SLOTS (ACTIVE_SLOTS)
    ) u_pipeline (
        .RESET_n             (RESET_n),
        .CLK                 (CLK),
        .sample_tick         (sample_tick),
        .current_slot        (current_slot),
        .fnum                (fnum),
        .octave              (octave),
        .key_on              (key_on),
        .state_read_addr     (state_read_addr),
        .state_read_data     (state_read_data),
        .state_write_addr    (state_write_addr),
        .state_write_data    (state_write_data),
        .state_write_en      (state_write_en),
        .phase_acc_out_slot0 (phase_acc_out_slot0)
    );

    ymf278_slot_state u_state (
        .CLK         (CLK),
        .RESET_n     (RESET_n),
        .read_addr   (state_read_addr),
        .read_data   (state_read_data),
        .write_addr  (state_write_addr),
        .write_data  (state_write_data),
        .write_en    (state_write_en)
    );

endmodule

`default_nettype wire
