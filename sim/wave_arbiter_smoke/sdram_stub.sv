//
// sdram_stub.sv — modelo SDRAM mínimo para Verilator testbench.
//
// Reproduce las propiedades CRÍTICAS del SDRAM real (sdram.sv) que
// pueden interactuar con el wave_arbiter:
//
//  1. LEVEL_TRIG=1: begin_wr = !Ram.WE_n, begin_rd = !Ram.OE_n.
//     Sampleado mientras state==IDLE.
//
//  2. TIMING = (state == STATE_IDLE).
//
//  3. Cuando begin_wr/rd en IDLE: latch save_addr <= Ram.ADDR
//     (single sample; require que ADDR esté válido en este ciclo).
//
//  4. STATE_SETUP_WDATA_1 lee Ram.ADDR[1:0] LIVE para calcular DQM
//     (este es el bug-trap del arbiter: si ADDR no se mantiene 1
//      ciclo después de begin_wr, el DQM sale mal).
//
//  5. ACK_n=0 desde el ciclo siguiente a begin_wr/rd hasta
//     STATE_INACTIVE_ACK (mismo timing que el real).
//
//  6. WRITE: aplica save_din[7:0] a la posición indicada por
//     save_addr y save_dqm (= reproduce la replicación save_din[7:0]
//     × 4 con máscara DQM).
//
//  7. READ: devuelve Ram.DOUT={mem[3], mem[2], mem[1], mem[0]} de
//     la palabra apuntada por save_addr[31:2], luego mempointer
//     extrae el byte por save_addr[1:0].
//
// Memoria: 16 bytes accesibles vía Ram.ADDR[3:0] cuando los bits
// altos coinciden con SDRAM_BASE (0x300000 por convención del
// testbench, igual que wavemem.asc → ymf278 0x200000 → SDRAM
// 0x100000+0x200000=0x300000).
//
`default_nettype none

module sdram_stub #(
    parameter [23:0] SDRAM_BASE = 24'h300000
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

    // Memoria 16 bytes (testbench solo escribe en SDRAM_BASE..+15)
    logic [7:0] mem [0:15];
    initial begin
        for (int i = 0; i < 16; i++) mem[i] = 8'h00;
    end

    wire begin_rd_lvl = !Ram.OE_n;
    wire begin_wr_lvl = !Ram.WE_n;

    // TIMING combinacional (igual que SDRAM real)
    assign Ram.TIMING = (state == STATE_IDLE);

    // Detección de in-range
    wire        addr_in_range_live  = (Ram.ADDR[23:4]   == SDRAM_BASE[23:4]);
    wire        addr_in_range_saved = (save_addr[23:4]  == SDRAM_BASE[23:4]);

    // (apply_write/read_word inlined en always_ff abajo para evitar
    //  posible quirk de Verilator con NBA en tasks/functions)

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
            // === State machine ===
            unique case (1'b1)
                (state == STATE_IDLE): begin
                    if (begin_wr_lvl || begin_rd_lvl) begin
                        save_addr     <= Ram.ADDR;       // sample LIVE
                        save_din      <= Ram.DIN;
                        cmd_is_write  <= begin_wr_lvl;
                        cmd_is_read   <= begin_rd_lvl;
                        state         <= state + 7'd1;
                        Ram.ACK_n     <= 1'b0;
                    end
                end
                (state == STATE_SETUP_WDATA_1): begin
                    // CRÍTICO: lee Ram.ADDR[1:0] LIVE.
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
                    // Apply WRITE inline (en lugar de task)
                    if (cmd_is_write && addr_in_range_saved) begin
                        if (save_dqm[0] == 1'b0)
                            mem[{save_addr[3:2], 2'd0}] <= save_din[7:0];
                        if (save_dqm[1] == 1'b0)
                            mem[{save_addr[3:2], 2'd1}] <= save_din[7:0];
                        if (save_dqm[2] == 1'b0)
                            mem[{save_addr[3:2], 2'd2}] <= save_din[7:0];
                        if (save_dqm[3] == 1'b0)
                            mem[{save_addr[3:2], 2'd3}] <= save_din[7:0];
                    end
                    // Apply READ inline: extraer byte según save_addr[1:0]
                    // (mismo behavior que SDRAM real en STATE_FETCH_RDATA_1).
                    if (cmd_is_read && addr_in_range_saved) begin
                        case (save_addr[1:0])
                            2'd0: Ram.DOUT <= {24'h0, mem[{save_addr[3:2], 2'd0}]};
                            2'd1: Ram.DOUT <= {24'h0, mem[{save_addr[3:2], 2'd1}]};
                            2'd2: Ram.DOUT <= {24'h0, mem[{save_addr[3:2], 2'd2}]};
                            2'd3: Ram.DOUT <= {24'h0, mem[{save_addr[3:2], 2'd3}]};
                        endcase
                    end
                    state <= state + 7'd1;
                end
                (state == STATE_END): begin
                    state         <= STATE_IDLE;
                    cmd_is_write  <= 1'b0;
                    cmd_is_read   <= 1'b0;
                end
                default: begin
                    // Cycles 34-38: sólo avanza
                    state <= state + 7'd1;
                end
            endcase
        end
    end

endmodule

`default_nettype wire
