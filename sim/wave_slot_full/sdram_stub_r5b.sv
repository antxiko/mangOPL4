//
// sdram_stub_r5b.sv — SDRAM stub para validar header lookup de R5.b.
//
// Modelo simple: para CUALQUIER addr, DOUT = addr[7:0]. Predecible y
// sin división (un primer intento con división N/12 en una function NO
// evaluó bien en Verilator — devolvía 0 — y dio falsa impresión de que
// fetch1 estaba roto. Lección: stub lo más simple posible).
//
// Con DOUT=addr[7:0], el header del sample N (en 0x100000 + N*12) tiene:
//   byte0 = (0x100000 + N*12)[7:0]
//   byte1 = (0x100000 + N*12 + 1)[7:0]
//   byte2 = (0x100000 + N*12 + 2)[7:0]
// fetch1 decodifica start_addr = 0x100000 + {byte0[5:0], byte1, byte2}.
// El TB computa el mismo valor esperado y verifica.
//
// RESULTADO 2026-05-28: 6/6 wave_nums OK. fetch1 header lookup VALIDADO.
// El bug HW (start_addr=0 en debug reg) NO es la lógica fetch1 — es
// instrumentación (read del debug reg 0xD0-D2) o timing en HW real.
//
// Replica FSM del SDRAM real (LEVEL_TRIG, TIMING, ACK_n).
//
`default_nettype none

module sdram_stub_r5b (
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
    logic        cmd_is_read;

    wire begin_rd_lvl = !Ram.OE_n;
    wire begin_wr_lvl = !Ram.WE_n;

    assign Ram.TIMING = (state == STATE_IDLE);

    // Modelo de memoria combinacional. addr relativo al YRW801 (0x100000).
    // Zona header: rel < 384*12 = 4608. Zona sample: rel >= 4608.
    function automatic [7:0] mem_model(input [23:0] addr);
        logic [23:0] rel;
        logic [15:0] sample_n;
        logic [3:0]  byte_in_hdr;
        logic [23:0] offset;
        begin
            rel = addr - 24'h100000;
            if (rel < 24'd4608) begin
                // Zona de headers. sample_n = rel / 12, byte = rel % 12.
                sample_n    = rel / 16'd12;
                byte_in_hdr = rel % 24'd12;
                // offset(N) = 0x001000 + N*0x100
                offset = 24'h001000 + ({8'h0, sample_n} << 8);
                case (byte_in_hdr)
                    4'd0:    mem_model = {2'b00, offset[21:16]};
                    4'd1:    mem_model = offset[15:8];
                    4'd2:    mem_model = offset[7:0];
                    default: mem_model = 8'h00;
                endcase
            end
            else begin
                // Zona de samples: byte = addr bajo (predecible).
                mem_model = addr[7:0];
            end
        end
    endfunction

    always_ff @(posedge CLK or negedge RESET_n) begin
        if (!RESET_n) begin
            state        <= STATE_IDLE;
            Ram.ACK_n    <= 1'b1;
            Ram.DOUT     <= 32'h0;
            cmd_is_read  <= 1'b0;
            save_addr    <= 24'h0;
        end
        else begin
            unique case (1'b1)
                (state == STATE_IDLE): begin
                    if (begin_wr_lvl || begin_rd_lvl) begin
                        save_addr   <= Ram.ADDR;
                        cmd_is_read <= begin_rd_lvl;
                        state       <= state + 7'd1;
                        Ram.ACK_n   <= 1'b0;
                    end
                end
                (state == STATE_INACTIVE_ACK): begin
                    Ram.ACK_n <= 1'b1;
                    if (cmd_is_read) begin
                        // TEST aislante: devuelve addr[7:0] para todo.
                        Ram.DOUT <= {24'h0, save_addr[7:0]};
                    end
                    state <= state + 7'd1;
                end
                (state == STATE_END): begin
                    state       <= STATE_IDLE;
                    cmd_is_read <= 1'b0;
                end
                default: begin
                    state <= state + 7'd1;
                end
            endcase
        end
    end

endmodule

`default_nettype wire
