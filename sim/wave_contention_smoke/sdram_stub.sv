//
// sdram_stub.sv — SDRAM model con memoria de 1 MB para reproducir
// el bug del 256 KB. Versión refinada del de wave_arbiter_smoke con:
//
//   - Memoria de 1 MB (cubre todo el mapper de cartridge_ram).
//   - Memoria adicional para YRW801 area (0x100000-0x10FFFF, 64 KB
//     suficiente para fetch1 de nuestro test).
//   - Reproduce LEVEL_TRIG=1, TIMING, ACK_n, save_addr LIVE-DQM.
//
// Memoria interna:
//   - mem_mapper: 1 MB en 0x000000-0x0FFFFF.
//   - mem_yrw801: 64 KB en 0x100000-0x10FFFF (pre-fillable desde tb).
//
`default_nettype none

module sdram_stub (
    input  wire CLK,
    input  wire RESET_n,
    output wire [31:0] dbg_writes_to_mapper,
    output wire [31:0] dbg_writes_to_yrw801,
    output wire [31:0] dbg_write_count,
    output wire        dbg_op_capture_pulse, // 1-cycle pulse cada vez que SDRAM captura save_addr
    output wire [23:0] dbg_op_capture_addr,
    output wire        dbg_op_capture_is_write,
    RAM_IF.DEVICE Ram
);
    assign dbg_writes_to_mapper = writes_to_mapper;
    assign dbg_writes_to_yrw801 = writes_to_yrw801;
    assign dbg_write_count       = write_count;
    // Pulso 1-cycle cuando SDRAM transiciona IDLE → state+1 (= captura save_addr).
    assign dbg_op_capture_pulse = (state == STATE_IDLE) && (begin_wr_lvl || begin_rd_lvl);
    assign dbg_op_capture_addr  = Ram.ADDR;
    assign dbg_op_capture_is_write = begin_wr_lvl;

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
    // Counters para análisis post-mortem.
    logic [31:0] write_count;
    logic [31:0] read_count;
    logic [31:0] writes_to_yrw801;   // writes que cayeron en YRW801 (= corruptas)
    logic [31:0] writes_to_mapper;

    // Memoria mapper (1 MB) y "fuera de mapper" (catch-all, 2 MB).
    logic [7:0] mem_mapper [0:1024*1024-1];
    logic [7:0] mem_outside [0:2*1024*1024-1];   // 0x100000-0x2FFFFF

    integer init_i;
    initial begin
        for (init_i = 0; init_i < 1024*1024; init_i = init_i + 1)
            mem_mapper[init_i] = 8'h00;
        for (init_i = 0; init_i < 2*1024*1024; init_i = init_i + 1)
            mem_outside[init_i] = 8'h00;
    end

    wire begin_rd_lvl = !Ram.OE_n;
    wire begin_wr_lvl = !Ram.WE_n;

    assign Ram.TIMING = (state == STATE_IDLE);

    // ¿En qué región cae save_addr? Si bit 20 = 0 → mapper.
    // Si 0x100000-0x2FFFFF → outside (= "corruptas" si Z80 wanted mapper).
    wire        addr_in_mapper  = (save_addr[23:20] == 4'h0);
    wire        addr_in_outside = (save_addr[23:21] == 3'h0) && (save_addr[20] == 1'b1) ||
                                  (save_addr[23:21] == 3'h1);  // 0x100000-0x2FFFFF

    always_ff @(posedge CLK or negedge RESET_n) begin
        if (!RESET_n) begin
            state            <= STATE_IDLE;
            Ram.ACK_n        <= 1'b1;
            Ram.DOUT         <= 32'h0;
            cmd_is_write     <= 1'b0;
            cmd_is_read      <= 1'b0;
            save_addr        <= 24'h0;
            save_din         <= 32'h0;
            save_dqm         <= 4'b1111;
            write_count      <= 0;
            read_count       <= 0;
            writes_to_yrw801 <= 0;
            writes_to_mapper <= 0;
        end
        else begin
            unique case (1'b1)
                (state == STATE_IDLE): begin
                    if (begin_wr_lvl || begin_rd_lvl) begin
                        save_addr     <= Ram.ADDR;
                        save_din      <= Ram.DIN;
                        // CRÍTICO: priority como en SDRAM real — si WE_n=0,
                        // es write (incluso si OE_n=0 también). Esto reproduce
                        // el escenario donde fetch1 (OE_n=0) y cartridge_ram
                        // (WE_n=0) coinciden → SDRAM trata como write a addr
                        // corrupta (OR-collapse de fetch1.ADDR | cart.ADDR).
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
                    // WRITE
                    if (cmd_is_write) begin
                        write_count <= write_count + 1;
                        if (addr_in_mapper) begin
                            writes_to_mapper <= writes_to_mapper + 1;
                            if (save_dqm[0] == 1'b0)
                                mem_mapper[{save_addr[19:2], 2'd0}] <= save_din[7:0];
                            if (save_dqm[1] == 1'b0)
                                mem_mapper[{save_addr[19:2], 2'd1}] <= save_din[7:0];
                            if (save_dqm[2] == 1'b0)
                                mem_mapper[{save_addr[19:2], 2'd2}] <= save_din[7:0];
                            if (save_dqm[3] == 1'b0)
                                mem_mapper[{save_addr[19:2], 2'd3}] <= save_din[7:0];
                        end
                        else if (addr_in_outside) begin
                            writes_to_yrw801 <= writes_to_yrw801 + 1;
                            // index into mem_outside con offset (save_addr - 0x100000).
                            if (save_dqm[0] == 1'b0)
                                mem_outside[{save_addr[20:2], 2'd0}] <= save_din[7:0];
                            if (save_dqm[1] == 1'b0)
                                mem_outside[{save_addr[20:2], 2'd1}] <= save_din[7:0];
                            if (save_dqm[2] == 1'b0)
                                mem_outside[{save_addr[20:2], 2'd2}] <= save_din[7:0];
                            if (save_dqm[3] == 1'b0)
                                mem_outside[{save_addr[20:2], 2'd3}] <= save_din[7:0];
                        end
                        // else: out of range, ignore
                    end
                    // READ
                    if (cmd_is_read) begin
                        if (addr_in_mapper) begin
                            case (save_addr[1:0])
                                2'd0: Ram.DOUT <= {24'h0, mem_mapper[{save_addr[19:2], 2'd0}]};
                                2'd1: Ram.DOUT <= {24'h0, mem_mapper[{save_addr[19:2], 2'd1}]};
                                2'd2: Ram.DOUT <= {24'h0, mem_mapper[{save_addr[19:2], 2'd2}]};
                                2'd3: Ram.DOUT <= {24'h0, mem_mapper[{save_addr[19:2], 2'd3}]};
                            endcase
                        end
                        else if (addr_in_outside) begin
                            case (save_addr[1:0])
                                2'd0: Ram.DOUT <= {24'h0, mem_outside[{save_addr[20:2], 2'd0}]};
                                2'd1: Ram.DOUT <= {24'h0, mem_outside[{save_addr[20:2], 2'd1}]};
                                2'd2: Ram.DOUT <= {24'h0, mem_outside[{save_addr[20:2], 2'd2}]};
                                2'd3: Ram.DOUT <= {24'h0, mem_outside[{save_addr[20:2], 2'd3}]};
                            endcase
                        end
                        else begin
                            Ram.DOUT <= 32'hFFFFFFFF;
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
