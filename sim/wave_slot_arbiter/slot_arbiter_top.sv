//
// slot_arbiter_top.sv — top sim de slot 0 con cadena de arbitración
// completa: wave_arbiter (mempointer vs slot) + sdram_top_arbiter
// (Z80/MSX vs wave block) + sdram_stub.
//
// Cadena RAM_IF:
//
//   ymf278_slot ────────── Ram_slot ─────┐
//                                        ├── wave_arbiter ── Ram_wave ──┐
//   mempointer_pulse_stub ── Ram_memp ───┘                              │
//                            (priority A=mempointer > B=slot)           │
//                                                                       ├── sdram_top_arbiter ── Ram_sdram ── sdram_stub_yrw
//   z80_stub ────────────── Ram_z80 ──────────────────────────────────  ┘
//                            (priority A=z80 > B=wave)
//
// Inputs de control desde TB para configurar stims:
//   - z80_stim_active/addr/cycles_on/off: pattern z80 sostenido
//   - memp_pulse_fire/addr: pulso mempointer
//
// Esto permite reproducir las 4 escenarios:
//   1. Z80=0, memp=0: solo slot. Equivalente a wave_slot_full.
//   2. Z80=alto duty, memp=0: contention de z80 priority A.
//   3. Z80=0, memp pulsing: competition en wave_arbiter.
//   4. Combinación: caos. Útil para reproducir HW behavior si convergen.
//
`default_nettype none

module slot_arbiter_top (
    input  wire        CLK,
    input  wire        RESET_n,
    input  wire        bus_reset_n,
    input  wire        sample_tick,

    // Slot stim
    input  wire [9:0]              fnum,
    input  wire signed [3:0]       octave,
    input  wire                    key_on,
    input  wire [23:0]             start_addr_sdram_in,

    // Z80 stub stim
    input  wire                    z80_stim_active,
    input  wire [23:0]             z80_stim_addr,
    input  wire [15:0]             z80_cycles_on,
    input  wire [15:0]             z80_cycles_off,

    // Mempointer pulse stub stim
    input  wire                    memp_pulse_fire,
    input  wire [23:0]             memp_pulse_addr,
    output wire [7:0]              memp_last_byte,
    output wire                    memp_last_valid,

    // Slot observables
    output wire signed [15:0]      sample_out,
    output wire [31:0]             phase_acc_out,

    // Fetch1 observables (via hierarchical)
    output wire [2:0]              dbg_fetch_state,
    output wire [15:0]             dbg_fetch_last_idx,
    output wire [7:0]              dbg_fetch_byte_a,
    output wire [7:0]              dbg_fetch_byte_b,

    // SDRAM Primary bus observables (lo que llega al SDRAM real)
    output wire [23:0]             dbg_sdram_addr,
    output wire                    dbg_sdram_oe_n,
    output wire                    dbg_sdram_ack_n,
    output wire                    dbg_sdram_timing,

    // wave_arbiter observables
    output wire                    dbg_wave_active_a,
    output wire                    dbg_wave_active_b,

    // sdram_top_arbiter observables
    output wire                    dbg_top_active_a,
    output wire                    dbg_top_active_b
);

    // Interfaces RAM_IF
    RAM_IF Ram_slot();      // ymf278_slot → wave_arbiter BusB
    RAM_IF Ram_memp();      // mempointer_stub → wave_arbiter BusA
    RAM_IF Ram_wave();      // wave_arbiter Primary → sdram_top_arbiter BusB
    RAM_IF Ram_z80();       // z80_stub → sdram_top_arbiter BusA
    RAM_IF Ram_sdram();     // sdram_top_arbiter Primary → sdram_stub_yrw

    // === Slot real ===
    ymf278_slot u_slot (
        .RESET_n                (RESET_n),
        .CLK                    (CLK),
        .bus_reset_n            (bus_reset_n),
        .sample_tick            (sample_tick),
        .fnum                   (fnum),
        .octave                 (octave),
        .key_on                 (key_on),
        .start_addr_sdram_in    (start_addr_sdram_in),
        .sample_out             (sample_out),
        .phase_acc_out          (phase_acc_out),
        .Ram                    (Ram_slot)
    );

    // === Mempointer stub (BusA del wave_arbiter, priority A>B) ===
    ram_host_mempointer_pulse_stub u_memp (
        .CLK             (CLK),
        .RESET_n         (RESET_n),
        .pulse_fire      (memp_pulse_fire),
        .pulse_addr      (memp_pulse_addr),
        .last_byte_read  (memp_last_byte),
        .last_read_valid (memp_last_valid),
        .Ram             (Ram_memp)
    );

    // === wave_arbiter (BusA=mempointer, BusB=slot/fetch1) ===
    wave_arbiter u_wave_arb (
        .RESET_n (RESET_n),
        .CLK     (CLK),
        .Primary (Ram_wave),
        .BusA    (Ram_memp),
        .BusB    (Ram_slot)
    );

    // === Z80 stub (BusA del sdram_top_arbiter, priority A>B) ===
    ram_host_z80_stub u_z80 (
        .CLK             (CLK),
        .RESET_n         (RESET_n),
        .stim_active     (z80_stim_active),
        .stim_addr       (z80_stim_addr),
        .stim_cycles_on  (z80_cycles_on),
        .stim_cycles_off (z80_cycles_off),
        .Ram             (Ram_z80)
    );

    // === sdram_top_arbiter (BusA=z80, BusB=wave block) ===
    sdram_top_arbiter u_top_arb (
        .RESET_n (RESET_n),
        .CLK     (CLK),
        .Primary (Ram_sdram),
        .BusA    (Ram_z80),
        .BusB    (Ram_wave)
    );

    // === SDRAM stub ===
    sdram_stub_yrw u_sdram (
        .CLK     (CLK),
        .RESET_n (RESET_n),
        .Ram     (Ram_sdram)
    );

    // Observables fetch1
    assign dbg_fetch_state    = u_slot.u_fetch1.state;
    assign dbg_fetch_last_idx = u_slot.u_fetch1.last_idx_fetched;
    assign dbg_fetch_byte_a   = u_slot.u_fetch1.byte_a;
    assign dbg_fetch_byte_b   = u_slot.u_fetch1.byte_b;

    // Observables SDRAM primary
    assign dbg_sdram_addr   = Ram_sdram.ADDR;
    assign dbg_sdram_oe_n   = Ram_sdram.OE_n;
    assign dbg_sdram_ack_n  = Ram_sdram.ACK_n;
    assign dbg_sdram_timing = Ram_sdram.TIMING;

    // Observables arbiters
    assign dbg_wave_active_a = u_wave_arb.active_a;
    assign dbg_wave_active_b = u_wave_arb.active_b;
    assign dbg_top_active_a  = u_top_arb.active_a;
    assign dbg_top_active_b  = u_top_arb.active_b;

endmodule

`default_nettype wire
