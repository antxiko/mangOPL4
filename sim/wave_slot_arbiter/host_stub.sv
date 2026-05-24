//
// host_stub.sv — stubs RAM_IF.HOST configurables para sim de arbitración.
//
// Dos módulos:
//
// 1. ram_host_idle_stub: drivea siempre idle (OE_n=WE_n=RFSH_n=1, ADDR=0).
//    Para tener una entrada del arbiter "presente pero quieta".
//
// 2. ram_host_z80_stub: simula el patrón de cartridge_ram (host MSX que
//    NO chequea ACK_n y drivea OE_n sostenido durante M-cycles del Z80).
//    Controlado por inputs:
//      - stim_active: 1 → drive bus con OE_n=0 + ADDR=stim_addr durante
//                     stim_cycles_on ciclos, luego idle stim_cycles_off
//                     ciclos. 0 → idle siempre.
//      - stim_addr: dirección que escribirá (mapper region tipica).
//      - stim_cycles_on/off: duty cycle del bus z80.
//
// 3. ram_host_mempointer_pulse_stub: simula mempointer firing read único.
//    Controlado por strobe pulse_fire (1 ciclo) que dispara 1 read a
//    pulse_addr. FSM: IDLE → REQ → WAIT_ACK_DEASSERT → IDLE. Chequea
//    ACK_n (como mempointer real).
//
`default_nettype none

module ram_host_idle_stub (
    RAM_IF.HOST Ram
);
    assign Ram.OE_n     = 1'b1;
    assign Ram.WE_n     = 1'b1;
    assign Ram.RFSH_n   = 1'b1;
    assign Ram.ADDR     = 24'h0;
    assign Ram.DIN      = 32'h0;
    assign Ram.DIN_SIZE = 3'b000;
endmodule

module ram_host_z80_stub (
    input  wire        CLK,
    input  wire        RESET_n,
    input  wire        stim_active,        // 1 = fire pattern z80
    input  wire [23:0] stim_addr,
    input  wire [15:0] stim_cycles_on,     // ciclos OE_n=0 sostenido
    input  wire [15:0] stim_cycles_off,    // ciclos idle entre pulsos
    RAM_IF.HOST        Ram
);
    typedef enum logic [1:0] {
        S_IDLE,
        S_DRIVING,
        S_GAP
    } state_t;

    state_t      state;
    logic [15:0] counter;

    always_ff @(posedge CLK or negedge RESET_n) begin
        if (!RESET_n) begin
            state   <= S_IDLE;
            counter <= 16'h0;
        end
        else if (!stim_active) begin
            state   <= S_IDLE;
            counter <= 16'h0;
        end
        else begin
            case (state)
                S_IDLE: begin
                    state   <= S_DRIVING;
                    counter <= 16'h0;
                end
                S_DRIVING: begin
                    if (counter >= stim_cycles_on) begin
                        state   <= S_GAP;
                        counter <= 16'h0;
                    end
                    else begin
                        counter <= counter + 16'h1;
                    end
                end
                S_GAP: begin
                    if (counter >= stim_cycles_off) begin
                        state   <= S_DRIVING;
                        counter <= 16'h0;
                    end
                    else begin
                        counter <= counter + 16'h1;
                    end
                end
                default: state <= S_IDLE;
            endcase
        end
    end

    wire driving = (state == S_DRIVING) && stim_active;

    assign Ram.OE_n     = !driving;          // OE_n=0 cuando driving
    assign Ram.WE_n     = 1'b1;              // no write
    assign Ram.RFSH_n   = 1'b1;
    assign Ram.ADDR     = driving ? stim_addr : 24'h0;
    assign Ram.DIN      = 32'h0;
    assign Ram.DIN_SIZE = 3'b000;
endmodule

module ram_host_mempointer_pulse_stub (
    input  wire        CLK,
    input  wire        RESET_n,
    input  wire        pulse_fire,           // 1-cycle strobe → fire 1 read
    input  wire [23:0] pulse_addr,
    output logic [7:0] last_byte_read,
    output logic       last_read_valid,
    RAM_IF.HOST        Ram
);
    typedef enum logic [1:0] {
        S_IDLE,
        S_REQ,
        S_WAIT_ACK_DEASSERT
    } state_t;

    state_t      state;
    logic [23:0] req_addr;
    logic        pending_fire;

    always_ff @(posedge CLK or negedge RESET_n) begin
        if (!RESET_n) begin
            state          <= S_IDLE;
            req_addr       <= 24'h0;
            pending_fire   <= 1'b0;
            last_byte_read <= 8'h0;
            last_read_valid<= 1'b0;
            Ram.ADDR       <= 24'h0;
            Ram.DIN        <= 32'h0;
            Ram.DIN_SIZE   <= 3'b000;
            Ram.OE_n       <= 1'b1;
            Ram.WE_n       <= 1'b1;
            Ram.RFSH_n     <= 1'b1;
        end
        else begin
            // Defaults
            Ram.ADDR     <= 24'h0;
            Ram.DIN      <= 32'h0;
            Ram.DIN_SIZE <= 3'b000;
            Ram.OE_n     <= 1'b1;
            Ram.WE_n     <= 1'b1;
            Ram.RFSH_n   <= 1'b1;

            if (pulse_fire) begin
                pending_fire <= 1'b1;
                req_addr     <= pulse_addr;
            end

            case (state)
                S_IDLE: begin
                    if (pending_fire && Ram.TIMING) begin
                        state        <= S_REQ;
                        pending_fire <= 1'b0;
                        Ram.ADDR     <= req_addr;
                        Ram.OE_n     <= 1'b0;
                    end
                end
                S_REQ: begin
                    Ram.ADDR <= req_addr;
                    if (Ram.ACK_n == 1'b0) begin
                        state <= S_WAIT_ACK_DEASSERT;
                    end
                end
                S_WAIT_ACK_DEASSERT: begin
                    if (Ram.ACK_n == 1'b1) begin
                        last_byte_read  <= Ram.DOUT[7:0];
                        last_read_valid <= 1'b1;
                        state           <= S_IDLE;
                    end
                end
                default: state <= S_IDLE;
            endcase
        end
    end
endmodule

`default_nettype wire
