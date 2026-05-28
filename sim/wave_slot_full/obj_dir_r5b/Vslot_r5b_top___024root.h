// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design internal header
// See Vslot_r5b_top.h for the primary calling header

#ifndef VERILATED_VSLOT_R5B_TOP___024ROOT_H_
#define VERILATED_VSLOT_R5B_TOP___024ROOT_H_  // guard

#include "verilated.h"
class Vslot_r5b_top_RAM_IF;


class Vslot_r5b_top__Syms;

class alignas(VL_CACHE_LINE_BYTES) Vslot_r5b_top___024root final : public VerilatedModule {
  public:
    // CELLS
    Vslot_r5b_top_RAM_IF* __PVT__slot_r5b_top__DOT__Ram_slot;

    // DESIGN SPECIFIC STATE
    VL_IN8(CLK,0,0);
    VL_IN8(RESET_n,0,0);
    VL_IN8(bus_reset_n,0,0);
    VL_IN8(sample_tick,0,0);
    VL_IN8(octave,3,0);
    VL_IN8(key_on,0,0);
    VL_OUT8(dbg_byte_a,7,0);
    VL_OUT8(dbg_byte_b,7,0);
    VL_OUT8(dbg_fetch_state,3,0);
    VL_OUT8(dbg_ram_timing,0,0);
    VL_OUT8(dbg_need_header,0,0);
    VL_OUT8(dbg_header_valid,0,0);
    VL_OUT8(dbg_hdr0,7,0);
    VL_OUT8(dbg_hdr1,7,0);
    VL_OUT8(dbg_ram_oe_n,0,0);
    VL_OUT8(dbg_ram_ack_n,0,0);
    CData/*0:0*/ slot_r5b_top__DOT__u_slot__DOT__u_phase__DOT__key_on_prev;
    CData/*3:0*/ slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state;
    CData/*0:0*/ slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__key_on_prev_internal;
    CData/*0:0*/ slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__need_header;
    CData/*0:0*/ slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_valid;
    CData/*7:0*/ slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr0;
    CData/*7:0*/ slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr1;
    CData/*7:0*/ slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__byte_a;
    CData/*7:0*/ slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__byte_b;
    CData/*6:0*/ slot_r5b_top__DOT__u_sdram__DOT__state;
    CData/*0:0*/ slot_r5b_top__DOT__u_sdram__DOT__cmd_is_read;
    CData/*0:0*/ __VstlFirstIteration;
    CData/*0:0*/ __VicoFirstIteration;
    CData/*0:0*/ __Vtrigprevexpr___TOP__CLK__0;
    CData/*0:0*/ __Vtrigprevexpr___TOP__RESET_n__0;
    CData/*0:0*/ __VactContinue;
    VL_IN16(fnum,9,0);
    VL_IN16(wave_num,8,0);
    VL_OUT16(sample_out,15,0);
    SData/*15:0*/ slot_r5b_top__DOT__u_slot__DOT__interp_out;
    SData/*15:0*/ slot_r5b_top__DOT__u_slot__DOT__sample_out_reg;
    SData/*15:0*/ slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__last_idx_fetched;
    VL_OUT(phase_acc_out,31,0);
    VL_OUT(dbg_start_addr,23,0);
    VL_OUT(dbg_ram_addr,23,0);
    IData/*31:0*/ slot_r5b_top__DOT__u_slot__DOT__phase_acc;
    IData/*23:0*/ slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__start_addr_real;
    IData/*23:0*/ slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_base;
    IData/*17:0*/ slot_r5b_top__DOT__u_slot__DOT__u_interp__DOT__u_interp__DOT__sum;
    IData/*23:0*/ slot_r5b_top__DOT__u_sdram__DOT__save_addr;
    IData/*31:0*/ __VactIterCount;
    VlTriggerVec<1> __VstlTriggered;
    VlTriggerVec<1> __VicoTriggered;
    VlTriggerVec<1> __VactTriggered;
    VlTriggerVec<1> __VnbaTriggered;

    // INTERNAL VARIABLES
    Vslot_r5b_top__Syms* const vlSymsp;

    // CONSTRUCTORS
    Vslot_r5b_top___024root(Vslot_r5b_top__Syms* symsp, const char* v__name);
    ~Vslot_r5b_top___024root();
    VL_UNCOPYABLE(Vslot_r5b_top___024root);

    // INTERNAL METHODS
    void __Vconfigure(bool first);
};


#endif  // guard
