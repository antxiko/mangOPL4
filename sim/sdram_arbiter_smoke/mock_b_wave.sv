//
// mock_b_wave.sv — stub estilo wave block (mempointer/fetch1) para
// validar TOP_ARBITER.
//
// Patrón validado en mempointer y fetch1:
//   - Cuando trigger=1, transiciona S_IDLE → S_REQ con OE_n=0 1-pulse
//     en la transición. Gate por Ram.TIMING=1 (SDRAM en STATE_IDLE).
//   - En S_REQ espera Ram.ACK_n=0 → S_WAIT_DEASSERT.
//   - En S_WAIT_DEASSERT espera Ram.ACK_n=1 → captura DOUT → S_DONE.
//   - S_DONE pulse `done` 1-cycle, luego S_IDLE.
//
// Esto NO drivea WE_n=0 (este mock solo hace reads — coherente con
// mempointer's reads y todo fetch1).
//
`default_nettype none

module mock_b_wave (
    input  wire        CLK,
    input  wire        RESET_n,

    // Control desde el testbench
    input  wire        trigger,        // pulse 1-cycle para arrancar request
    input  wire [23:0] addr,

    // Output
    output reg  [7:0]  captured_byte,
    output reg         done,           // pulse 1-cycle al completar

    RAM_IF.HOST        Ram
);

    typedef enum logic [1:0] {
        S_IDLE,
        S_REQ,
        S_WAIT_DEASSERT,
        S_DONE
    } state_t;

    state_t state;
    logic [23:0] saved_addr;

    always_ff @(posedge CLK or negedge RESET_n) begin
        if (!RESET_n) begin
            state         <= S_IDLE;
            saved_addr    <= 24'h0;
            captured_byte <= 8'h00;
            done          <= 1'b0;
            Ram.ADDR      <= 24'h0;
            Ram.DIN       <= 32'h0;
            Ram.DIN_SIZE  <= 3'b000;
            Ram.OE_n      <= 1'b1;
            Ram.WE_n      <= 1'b1;
            Ram.RFSH_n    <= 1'b1;
        end
        else begin
            // Defaults idle cada ciclo (clear ADDR/OE_n).
            Ram.ADDR     <= 24'h0;
            Ram.DIN      <= 32'h0;
            Ram.DIN_SIZE <= 3'b000;
            Ram.OE_n     <= 1'b1;
            Ram.WE_n     <= 1'b1;
            Ram.RFSH_n   <= 1'b1;
            done         <= 1'b0;

            case (state)
                S_IDLE: begin
                    if (trigger && Ram.TIMING) begin
                        // 1-pulse OE_n al transicionar a S_REQ
                        state      <= S_REQ;
                        saved_addr <= addr;
                        Ram.ADDR   <= addr;
                        Ram.OE_n   <= 1'b0;
                    end
                end

                S_REQ: begin
                    // OE_n=1 (default), esperando ACK_n=0
                    if (Ram.ACK_n == 1'b0) begin
                        state <= S_WAIT_DEASSERT;
                    end
                end

                S_WAIT_DEASSERT: begin
                    if (Ram.ACK_n == 1'b1) begin
                        captured_byte <= Ram.DOUT[7:0];
                        state         <= S_DONE;
                    end
                end

                S_DONE: begin
                    done  <= 1'b1;
                    state <= S_IDLE;
                end

                default: state <= S_IDLE;
            endcase
        end
    end

endmodule

`default_nettype wire
