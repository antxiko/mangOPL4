//
// fake_cartridge_ram.sv — modelo del cartridge_ram REAL para sim.
//
// Reproduce el patrón problemático que genera el bug de contención:
//   - Drivea Ram.OE_n=0 (read) o Ram.WE_n=0 (write) SOSTENIDO durante
//     todo el M-cycle del Z80 simulado (= no checkea Ram.ACK_n).
//   - Lee Ram.DOUT directamente cada ciclo.
//   - 4 bank registers de 6-bit (BANK_MASK=0x3F = mapper 1MB).
//
// Differs del cartridge_ram REAL en que este NO escucha Bus.IORQ_n
// para bank registers (los bank registers se setean directo desde
// el testbench).
//
// Translación de address: igual que cartridge_ram REAL:
//   Ram.ADDR = RAM_ADDR_BASE + {2'b00, bank[msx_addr[15:14]], msx_addr[13:0]}
//
`default_nettype none

module fake_cartridge_ram #(
    parameter [23:0] RAM_ADDR_BASE = 24'h0
) (
    input  wire        CLK,
    input  wire        RESET_n,
    input  wire        bus_reset_n,

    // "Z80 simulator" interface
    input  wire        msx_in_mcycle,    // 1 = Z80 driving M-cycle
    input  wire [15:0] msx_addr,         // Z80 logical addr
    input  wire [7:0]  msx_din,
    input  wire        msx_is_write,
    output reg  [7:0]  msx_dout,

    // Bank reg control (= simula OUT a F8/F9/FA/FB)
    input  wire        bank_wr_stb,      // pulso cuando setea bank
    input  wire [1:0]  bank_wr_idx,
    input  wire [5:0]  bank_wr_data,

    // SDRAM
    RAM_IF.HOST        Ram
);

    reg [5:0] bank [0:3];

    // Translación a SDRAM addr (igual que cartridge_ram REAL).
    wire [21:0] mapper_offset = {2'b00, bank[msx_addr[15:14]], msx_addr[13:0]};
    wire [23:0] sdram_addr_translated = RAM_ADDR_BASE + {2'b00, mapper_offset};

    always_ff @(posedge CLK or negedge RESET_n) begin
        if (!RESET_n) begin
            bank[0] <= 6'd3;
            bank[1] <= 6'd2;
            bank[2] <= 6'd1;
            bank[3] <= 6'd0;
            Ram.ADDR     <= 24'h0;
            Ram.DIN      <= 32'h0;
            Ram.DIN_SIZE <= 3'b000;
            Ram.OE_n     <= 1'b1;
            Ram.WE_n     <= 1'b1;
            Ram.RFSH_n   <= 1'b1;
            msx_dout     <= 8'h00;
        end
        else if (!bus_reset_n) begin
            bank[0] <= 6'd3;
            bank[1] <= 6'd2;
            bank[2] <= 6'd1;
            bank[3] <= 6'd0;
            Ram.ADDR     <= 24'h0;
            Ram.OE_n     <= 1'b1;
            Ram.WE_n     <= 1'b1;
        end
        else begin
            // Bank register updates
            if (bank_wr_stb) begin
                bank[bank_wr_idx] <= bank_wr_data;
            end

            // Drive SDRAM during Z80 M-cycle
            if (msx_in_mcycle) begin
                Ram.ADDR     <= sdram_addr_translated;
                Ram.DIN      <= {24'h0, msx_din};
                Ram.DIN_SIZE <= 3'b000;
                Ram.OE_n     <= msx_is_write ? 1'b1 : 1'b0;
                Ram.WE_n     <= msx_is_write ? 1'b0 : 1'b1;
                Ram.RFSH_n   <= 1'b1;
                msx_dout     <= Ram.DOUT[7:0];
            end
            else begin
                // Idle defaults (igual que cartridge_ram REAL).
                Ram.ADDR     <= 24'h0;
                Ram.DIN      <= 32'h0;
                Ram.DIN_SIZE <= 3'b000;
                Ram.OE_n     <= 1'b1;
                Ram.WE_n     <= 1'b1;
                Ram.RFSH_n   <= 1'b1;
            end
        end
    end

endmodule

`default_nettype wire
