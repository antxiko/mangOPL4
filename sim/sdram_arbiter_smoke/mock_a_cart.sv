//
// mock_a_cart.sv — stub estilo cartridge_ram para validar TOP_ARBITER.
//
// Simula el patrón de cartridge_ram REAL:
//   - Cuando trigger=1, drivea Ram.OE_n (read) o Ram.WE_n (write) = 0
//     SOSTENIDO durante M-cycle (n_cycles ticks). NO chequea Ram.ACK_n.
//   - Lee Ram.DOUT directamente cada ciclo. El último valor leído antes
//     de que trigger baje queda en `last_dout`.
//
// El testbench drivea trigger / addr / is_write / data, y al final del
// M-cycle simulado lee `last_dout` (= lo que cartridge_ram daría a Z80).
//
`default_nettype none

module mock_a_cart (
    input  wire        CLK,
    input  wire        RESET_n,

    // Control desde el testbench
    input  wire        trigger,        // 1 mientras el M-cycle está activo
    input  wire        is_write,       // 1=write, 0=read
    input  wire [23:0] addr,
    input  wire [7:0]  din,

    // Output: último DOUT[7:0] capturado durante el último ciclo de trigger
    output reg  [7:0]  last_dout,

    RAM_IF.HOST        Ram
);

    always_ff @(posedge CLK or negedge RESET_n) begin
        if (!RESET_n) begin
            Ram.ADDR     <= 24'h0;
            Ram.DIN      <= 32'h0;
            Ram.DIN_SIZE <= 3'b000;
            Ram.OE_n     <= 1'b1;
            Ram.WE_n     <= 1'b1;
            Ram.RFSH_n   <= 1'b1;
            last_dout    <= 8'h00;
        end
        else begin
            // Defaults idle (igual que cartridge_ram REAL: clear ADDR/DIN
            // cuando no hay M-cycle).
            Ram.ADDR     <= 24'h0;
            Ram.DIN      <= 32'h0;
            Ram.DIN_SIZE <= 3'b000;
            Ram.OE_n     <= 1'b1;
            Ram.WE_n     <= 1'b1;
            Ram.RFSH_n   <= 1'b1;

            if (trigger) begin
                Ram.ADDR     <= addr;
                Ram.DIN      <= {24'h0, din};
                Ram.DIN_SIZE <= 3'b000;
                Ram.OE_n     <= is_write ? 1'b1 : 1'b0;
                Ram.WE_n     <= is_write ? 1'b0 : 1'b1;
                last_dout    <= Ram.DOUT[7:0];   // lee cada ciclo
            end
        end
    end

endmodule

`default_nettype wire
