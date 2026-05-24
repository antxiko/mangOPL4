//
// sdram_stub_yrw.sv — SDRAM stub con datos YRW801 simulados.
//
// Basado en sim/wave_arbiter_smoke/sdram_stub.sv pero con memoria
// más grande (256 bytes en lugar de 16) y con datos hand-crafted
// distintos en regiones distintas para poder discriminar fetches:
//
//   mem[0x00..0x0F] = 0x01..0x10 (= bytes simulando sample 1)
//   mem[0x80..0x8F] = 0x81..0x90 (= bytes simulando sample 2)
//   mem[0xC0..0xCF] = 0xC1..0xD0 (= bytes simulando sample 3)
//
// SDRAM_BASE = 0x100000 (= mapping YRW801 en SDRAM real, R5 v0 default).
// Address de fetch1: start_addr + curr_idx → si start_addr=0x100000,
// idx=0 → SDRAM 0x100000 → mem[0x00] = 0x01.
//
`default_nettype none

module sdram_stub_yrw #(
    parameter [23:0] SDRAM_BASE = 24'h100000
) (
    input  wire CLK,
    input  wire RESET_n,
    RAM_IF.DEVICE Ram
);

    localparam logic [6:0] STATE_IDLE          = 7'd32;
    localparam logic [6:0] STATE_SETUP_WDATA_1 = 7'd33;
    localparam logic [6:0] STATE_INACTIVE_ACK  = 7'd39;
    localparam logic [6:0] STATE_END           = 7'd40;

    logic [6:0]  state;
    logic [23:0] save_addr;
    logic [31:0] save_din;
    logic [3:0]  save_dqm;
    logic        cmd_is_write;
    logic        cmd_is_read;

    // Memoria 256 bytes con datos distintos en regiones para
    // discriminación de start_addr changes.
    logic [7:0] mem [0:255];
    initial begin
        integer i;
        for (i = 0; i < 256; i = i + 1) mem[i] = 8'h00;
        // Region "sample 1" (offset 0x00): bytes 0x01..0x10
        for (i = 0; i < 16; i = i + 1) mem[i] = 8'h01 + i;
        // Region "sample 2" (offset 0x80): bytes 0x81..0x90
        for (i = 0; i < 16; i = i + 1) mem[8'h80 + i] = 8'h81 + i;
        // Region "sample 3" (offset 0xC0): bytes 0xC1..0xD0
        for (i = 0; i < 16; i = i + 1) mem[8'hC0 + i] = 8'hC1 + i;
    end

    wire begin_rd_lvl = !Ram.OE_n;
    wire begin_wr_lvl = !Ram.WE_n;

    assign Ram.TIMING = (state == STATE_IDLE);

    // Detección in-range: SDRAM_BASE..+255 (256 bytes)
    wire addr_in_range_saved = (save_addr[23:8] == SDRAM_BASE[23:8]);

    always_ff @(posedge CLK or negedge RESET_n) begin
        if (!RESET_n) begin
            state         <= STATE_IDLE;
            Ram.ACK_n     <= 1'b1;
            Ram.DOUT      <= 32'h0;
            cmd_is_write  <= 1'b0;
            cmd_is_read   <= 1'b0;
            save_addr     <= 24'h0;
            save_din      <= 32'h0;
            save_dqm      <= 4'b1111;
        end
        else begin
            unique case (1'b1)
                (state == STATE_IDLE): begin
                    if (begin_wr_lvl || begin_rd_lvl) begin
                        save_addr     <= Ram.ADDR;
                        save_din      <= Ram.DIN;
                        cmd_is_write  <= begin_wr_lvl;
                        cmd_is_read   <= begin_rd_lvl;
                        state         <= state + 7'd1;
                        Ram.ACK_n     <= 1'b0;
                    end
                end
                (state == STATE_SETUP_WDATA_1): begin
                    case (Ram.ADDR[1:0])
                        2'd0: save_dqm <= 4'b1110;
                        2'd1: save_dqm <= 4'b1101;
                        2'd2: save_dqm <= 4'b1011;
                        2'd3: save_dqm <= 4'b0111;
                    endcase
                    state <= state + 7'd1;
                end
                (state == STATE_INACTIVE_ACK): begin
                    Ram.ACK_n <= 1'b1;
                    if (cmd_is_read && addr_in_range_saved) begin
                        Ram.DOUT <= {24'h0, mem[save_addr[7:0]]};
                    end
                    state <= state + 7'd1;
                end
                (state == STATE_END): begin
                    state         <= STATE_IDLE;
                    cmd_is_write  <= 1'b0;
                    cmd_is_read   <= 1'b0;
                end
                default: begin
                    state <= state + 7'd1;
                end
            endcase
        end
    end

endmodule

`default_nettype wire
