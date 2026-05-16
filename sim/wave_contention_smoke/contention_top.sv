//
// contention_top.sv — top para reproducir bus contention en Verilator.
//
// Estructura:
//   - ymf278_top (mempointer + fetch1 + arbiter v5) en BusA[0].
//   - fake_cartridge_ram en BusA[1].
//   - EXPANSION_RAM REAL de tnCart (USE_FF=0, COUNT=2) merged.
//   - sdram_stub.
//
// Expone como puertos al testbench:
//   - mempointer control (reg_wr_stb, reg_addr, reg_data, ...).
//   - fake_cartridge_ram control (msx_in_mcycle, msx_addr, ...).
//   - bank_wr_stb/idx/data para setear bank registers.
//   - Debug observables (SDRAM contents accesibles).
//
`default_nettype none

module contention_top (
    input  wire        CLK,
    input  wire        RESET_n,
    input  wire        bus_reset_n,

    // Wave block control
    input  wire        new2,
    input  wire        wr_strobe,        // a wave (7E/7F)
    input  wire        addr0,
    input  wire [7:0]  din,
    input  wire        rd_done_strobe,
    input  wire        bus_merq_n,
    output wire [7:0]  rd_data,
    output wire signed [23:0] wave_sample,

    // fake_cartridge_ram control (= "Z80 simulator")
    input  wire        msx_in_mcycle,
    input  wire [15:0] msx_addr,
    input  wire [7:0]  msx_din,
    input  wire        msx_is_write,
    output wire [7:0]  msx_dout,

    // Bank reg control
    input  wire        bank_wr_stb,
    input  wire [1:0]  bank_wr_idx,
    input  wire [5:0]  bank_wr_data,

    // Observables del SDRAM stub para tb (no hace falta exponerlo aquí
    // porque Verilator puede acceder a internos vía path).
    // Solo exponemos un par para debug rápido.
    output wire        dbg_primary_timing,
    output wire        dbg_primary_ack_n,
    output wire [23:0] dbg_primary_addr,
    output wire        dbg_primary_oe_n,
    output wire        dbg_primary_we_n,
    output wire [31:0] dbg_sdram_op_count,
    output wire [31:0] dbg_contention_count,
    output wire [31:0] dbg_fetch1_fire_count,
    output wire [31:0] dbg_writes_to_mapper,
    output wire [31:0] dbg_writes_to_yrw801,
    output wire [31:0] dbg_write_count,
    output wire        dbg_op_capture_pulse,
    output wire [23:0] dbg_op_capture_addr,
    output wire        dbg_op_capture_is_write
);

    parameter int CLK_OPL3_DIV = 4;  // dummy, no se usa para audio en sim

    // Bus interfaces. Array de 2 secondaries para EXPANSION_RAM.
    // Idx 0 = wave block (ymf278_top). Idx 1 = fake_cartridge_ram.
    RAM_IF Ram_secondary[0:1]();
    RAM_IF Ram_primary();

    // ymf278_top (wave block) en idx 0
    wire signed [23:0] wave_sample_internal;
    ymf278_top u_wave (
        .RESET_n        (RESET_n),
        .CLK            (CLK),
        .CLK_OPL3       (CLK),           // mismo CLK en sim (no audio path here)
        .bus_reset_n    (bus_reset_n),
        .new2           (new2),
        .wr_strobe      (wr_strobe),
        .addr0          (addr0),
        .din            (din),
        .rd_done_strobe (rd_done_strobe),
        .bus_merq_n     (bus_merq_n),
        .rd_data        (rd_data),
        .wave_sample    (wave_sample_internal),
        .Ram            (Ram_secondary[0])
    );
    assign wave_sample = wave_sample_internal;

    // fake_cartridge_ram en idx 1
    fake_cartridge_ram #(.RAM_ADDR_BASE(24'h0)) u_cart (
        .CLK             (CLK),
        .RESET_n         (RESET_n),
        .bus_reset_n     (bus_reset_n),
        .msx_in_mcycle   (msx_in_mcycle),
        .msx_addr        (msx_addr),
        .msx_din         (msx_din),
        .msx_is_write    (msx_is_write),
        .msx_dout        (msx_dout),
        .bank_wr_stb     (bank_wr_stb),
        .bank_wr_idx     (bank_wr_idx),
        .bank_wr_data    (bank_wr_data),
        .Ram             (Ram_secondary[1])
    );

    // EXPANSION_RAM real de tnCart (USE_FF=0, COUNT=2)
    EXPANSION_RAM #(.COUNT(2), .USE_FF(0)) u_exp (
        .RESET_n   (RESET_n),
        .CLK       (CLK),
        .Primary   (Ram_primary),
        .Secondary (Ram_secondary)
    );

    // SDRAM stub
    sdram_stub u_sdram (
        .CLK     (CLK),
        .RESET_n (RESET_n),
        .dbg_writes_to_mapper (dbg_writes_to_mapper),
        .dbg_writes_to_yrw801 (dbg_writes_to_yrw801),
        .dbg_write_count       (dbg_write_count),
        .dbg_op_capture_pulse  (dbg_op_capture_pulse),
        .dbg_op_capture_addr   (dbg_op_capture_addr),
        .dbg_op_capture_is_write (dbg_op_capture_is_write),
        .Ram     (Ram_primary)
    );

    // Observables
    assign dbg_primary_timing = Ram_primary.TIMING;
    assign dbg_primary_ack_n  = Ram_primary.ACK_n;
    assign dbg_primary_addr   = Ram_primary.ADDR;
    assign dbg_primary_oe_n   = Ram_primary.OE_n;
    assign dbg_primary_we_n   = Ram_primary.WE_n;

    // Counters: cuántas veces se han triggereado SDRAM ops, y de quien.
    // Detect "fetch1 + cartridge_ram simultaneously asserting" event.
    logic [31:0] sdram_op_count;
    logic [31:0] contention_count;
    logic [31:0] fetch1_fire_count;
    wire fetch1_oe_n = Ram_secondary[0].OE_n;
    wire cart_oe_n   = Ram_secondary[1].OE_n;
    wire cart_we_n   = Ram_secondary[1].WE_n;
    wire fetch1_asserting = !fetch1_oe_n;
    wire cart_asserting   = !cart_oe_n || !cart_we_n;
    wire both_asserting   = fetch1_asserting && cart_asserting;

    always_ff @(posedge CLK or negedge RESET_n) begin
        if (!RESET_n) begin
            sdram_op_count    <= 0;
            contention_count  <= 0;
            fetch1_fire_count <= 0;
        end
        else begin
            // Cuenta cuando SDRAM transiciona de IDLE a estado 33.
            if (Ram_primary.TIMING && (!Ram_primary.OE_n || !Ram_primary.WE_n))
                sdram_op_count <= sdram_op_count + 1;
            if (both_asserting)
                contention_count <= contention_count + 1;
            if (fetch1_asserting)
                fetch1_fire_count <= fetch1_fire_count + 1;
        end
    end

    assign dbg_sdram_op_count    = sdram_op_count;
    assign dbg_contention_count  = contention_count;
    assign dbg_fetch1_fire_count = fetch1_fire_count;

endmodule

`default_nettype wire
