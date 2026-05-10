//
// sdram_stub.sv — SDRAM determinístico para validar sdram_top_arbiter.
//
// Memoria 1 MB pre-poblada con patrón: mem[i] = (i ^ 0xA5) & 0xFF
// (ofuscado para detectar reads de addr equivocada).
//
// LEVEL_TRIG=1, ACK_n broadcast 7 ciclos, save_addr LIVE Ram.ADDR
// (igual que wave_contention_smoke).
//
// Counters:
//   write_count       — total escrituras (ignoramos los datos en este sim).
//   read_count        — total lecturas.
//   op_capture_pulse  — pulso 1-cycle cada vez que SDRAM transiciona
//                       IDLE → setup (= momento de captura save_addr).
//
`default_nettype none

module sdram_stub (
    input  wire        CLK,
    input  wire        RESET_n,
    output wire [31:0] dbg_write_count,
    output wire [31:0] dbg_read_count,
    output wire        dbg_op_capture_pulse,
    output wire [23:0] dbg_op_capture_addr,
    output wire        dbg_op_capture_is_write,
    RAM_IF.DEVICE      Ram
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
    logic [31:0] write_count;
    logic [31:0] read_count;

    // Memoria 1 MB. Pre-poblada al reset con (addr ^ 0xA5).
    logic [7:0] mem [0:1024*1024-1];
    integer init_i;
    initial begin
        for (init_i = 0; init_i < 1024*1024; init_i = init_i + 1)
            mem[init_i] = (init_i ^ 8'hA5) & 8'hFF;
    end

    wire begin_rd_lvl = !Ram.OE_n;
    wire begin_wr_lvl = !Ram.WE_n;

    assign Ram.TIMING = (state == STATE_IDLE);

    assign dbg_write_count        = write_count;
    assign dbg_read_count         = read_count;
    assign dbg_op_capture_pulse   = (state == STATE_IDLE) && (begin_wr_lvl || begin_rd_lvl);
    assign dbg_op_capture_addr    = Ram.ADDR;
    assign dbg_op_capture_is_write = begin_wr_lvl;

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
            write_count   <= 0;
            read_count    <= 0;
        end
        else begin
            unique case (1'b1)
                (state == STATE_IDLE): begin
                    if (begin_wr_lvl || begin_rd_lvl) begin
                        save_addr     <= Ram.ADDR;
                        save_din      <= Ram.DIN;
                        cmd_is_write  <= begin_wr_lvl;
                        cmd_is_read   <= !begin_wr_lvl;
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
                    if (cmd_is_write) begin
                        write_count <= write_count + 1;
                        // Solo escribimos si addr en rango de mem[].
                        if (save_addr[23:20] == 4'h0) begin
                            if (save_dqm[0] == 1'b0) mem[{save_addr[19:2], 2'd0}] <= save_din[7:0];
                            if (save_dqm[1] == 1'b0) mem[{save_addr[19:2], 2'd1}] <= save_din[7:0];
                            if (save_dqm[2] == 1'b0) mem[{save_addr[19:2], 2'd2}] <= save_din[7:0];
                            if (save_dqm[3] == 1'b0) mem[{save_addr[19:2], 2'd3}] <= save_din[7:0];
                        end
                    end
                    if (cmd_is_read) begin
                        read_count <= read_count + 1;
                        if (save_addr[23:20] == 4'h0) begin
                            case (save_addr[1:0])
                                2'd0: Ram.DOUT <= {24'h0, mem[{save_addr[19:2], 2'd0}]};
                                2'd1: Ram.DOUT <= {24'h0, mem[{save_addr[19:2], 2'd1}]};
                                2'd2: Ram.DOUT <= {24'h0, mem[{save_addr[19:2], 2'd2}]};
                                2'd3: Ram.DOUT <= {24'h0, mem[{save_addr[19:2], 2'd3}]};
                            endcase
                        end
                        else begin
                            // Fuera de mem[]: devuelve patrón addr-as-content
                            // sin tocar memoria (= comportamiento "ROM" para
                            // testing de B leyendo YRW801 area)
                            Ram.DOUT <= {24'h0, save_addr[7:0] ^ 8'h5A};
                        end
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
