//
// top_test_top.sv — sim del ymf278_top COMPLETO (regfile + read mux +
// slot + fetch1 header lookup + wave_arbiter + mempointer) conectado a
// sdram_stub_r5b. Reproduce el path EXACTO del MSX:
//   - writes a 7E/7F (wr_strobe + addr0 + din) -> regfile
//   - reads de 7F (reg_addr_latch -> rd_data)
// para verificar si el DEBUG REG read de start_addr (0xD0-D2) funciona
// igual que el comportamiento interno. Si el debug reg lee mal aquí ->
// bug de instrumentación encontrado sin flash.
//
`default_nettype none

module top_test_top (
    input  wire        CLK,
    input  wire        CLK_OPL3,
    input  wire        RESET_n,
    input  wire        bus_reset_n,
    input  wire        new2,

    input  wire        wr_strobe,
    input  wire        addr0,
    input  wire [7:0]  din,
    input  wire        rd_done_strobe,
    input  wire        bus_merq_n,

    output wire [7:0]  rd_data,
    output wire signed [23:0] wave_sample
);

    RAM_IF Ram();

    ymf278_top u_top (
        .RESET_n        (RESET_n),
        .CLK            (CLK),
        .CLK_OPL3       (CLK_OPL3),
        .bus_reset_n    (bus_reset_n),
        .new2           (new2),
        .wr_strobe      (wr_strobe),
        .addr0          (addr0),
        .din            (din),
        .rd_done_strobe (rd_done_strobe),
        .bus_merq_n     (bus_merq_n),
        .rd_data        (rd_data),
        .wave_sample    (wave_sample),
        .Ram            (Ram)
    );

    sdram_stub_r5b u_sdram (
        .CLK     (CLK),
        .RESET_n (RESET_n),
        .Ram     (Ram)
    );

endmodule

`default_nettype wire
