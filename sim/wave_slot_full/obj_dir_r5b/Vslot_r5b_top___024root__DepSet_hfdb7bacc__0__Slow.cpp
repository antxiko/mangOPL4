// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vslot_r5b_top.h for the primary calling header

#include "Vslot_r5b_top__pch.h"
#include "Vslot_r5b_top___024root.h"

VL_ATTR_COLD void Vslot_r5b_top___024root___eval_static(Vslot_r5b_top___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vslot_r5b_top__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vslot_r5b_top___024root___eval_static\n"); );
}

VL_ATTR_COLD void Vslot_r5b_top___024root___eval_initial(Vslot_r5b_top___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vslot_r5b_top__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vslot_r5b_top___024root___eval_initial\n"); );
    // Body
    vlSelf->__Vtrigprevexpr___TOP__CLK__0 = vlSelf->CLK;
    vlSelf->__Vtrigprevexpr___TOP__RESET_n__0 = vlSelf->RESET_n;
}

VL_ATTR_COLD void Vslot_r5b_top___024root___eval_final(Vslot_r5b_top___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vslot_r5b_top__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vslot_r5b_top___024root___eval_final\n"); );
}

#ifdef VL_DEBUG
VL_ATTR_COLD void Vslot_r5b_top___024root___dump_triggers__stl(Vslot_r5b_top___024root* vlSelf);
#endif  // VL_DEBUG
VL_ATTR_COLD bool Vslot_r5b_top___024root___eval_phase__stl(Vslot_r5b_top___024root* vlSelf);

VL_ATTR_COLD void Vslot_r5b_top___024root___eval_settle(Vslot_r5b_top___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vslot_r5b_top__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vslot_r5b_top___024root___eval_settle\n"); );
    // Init
    IData/*31:0*/ __VstlIterCount;
    CData/*0:0*/ __VstlContinue;
    // Body
    __VstlIterCount = 0U;
    vlSelf->__VstlFirstIteration = 1U;
    __VstlContinue = 1U;
    while (__VstlContinue) {
        if (VL_UNLIKELY((0x64U < __VstlIterCount))) {
#ifdef VL_DEBUG
            Vslot_r5b_top___024root___dump_triggers__stl(vlSelf);
#endif
            VL_FATAL_MT("/mnt/c/Users/Antxiko/Documents/frutOPL4/sim/wave_slot_full/slot_r5b_top.sv", 11, "", "Settle region did not converge.");
        }
        __VstlIterCount = ((IData)(1U) + __VstlIterCount);
        __VstlContinue = 0U;
        if (Vslot_r5b_top___024root___eval_phase__stl(vlSelf)) {
            __VstlContinue = 1U;
        }
        vlSelf->__VstlFirstIteration = 0U;
    }
}

#ifdef VL_DEBUG
VL_ATTR_COLD void Vslot_r5b_top___024root___dump_triggers__stl(Vslot_r5b_top___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vslot_r5b_top__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vslot_r5b_top___024root___dump_triggers__stl\n"); );
    // Body
    if ((1U & (~ (IData)(vlSelf->__VstlTriggered.any())))) {
        VL_DBG_MSGF("         No triggers active\n");
    }
    if ((1ULL & vlSelf->__VstlTriggered.word(0U))) {
        VL_DBG_MSGF("         'stl' region trigger index 0 is active: Internal 'stl' trigger - first iteration\n");
    }
}
#endif  // VL_DEBUG

VL_ATTR_COLD void Vslot_r5b_top___024root___stl_sequent__TOP__0(Vslot_r5b_top___024root* vlSelf);

VL_ATTR_COLD void Vslot_r5b_top___024root___eval_stl(Vslot_r5b_top___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vslot_r5b_top__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vslot_r5b_top___024root___eval_stl\n"); );
    // Body
    if ((1ULL & vlSelf->__VstlTriggered.word(0U))) {
        Vslot_r5b_top___024root___stl_sequent__TOP__0(vlSelf);
    }
}

VL_ATTR_COLD void Vslot_r5b_top___024root___eval_triggers__stl(Vslot_r5b_top___024root* vlSelf);

VL_ATTR_COLD bool Vslot_r5b_top___024root___eval_phase__stl(Vslot_r5b_top___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vslot_r5b_top__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vslot_r5b_top___024root___eval_phase__stl\n"); );
    // Init
    CData/*0:0*/ __VstlExecute;
    // Body
    Vslot_r5b_top___024root___eval_triggers__stl(vlSelf);
    __VstlExecute = vlSelf->__VstlTriggered.any();
    if (__VstlExecute) {
        Vslot_r5b_top___024root___eval_stl(vlSelf);
    }
    return (__VstlExecute);
}

#ifdef VL_DEBUG
VL_ATTR_COLD void Vslot_r5b_top___024root___dump_triggers__ico(Vslot_r5b_top___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vslot_r5b_top__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vslot_r5b_top___024root___dump_triggers__ico\n"); );
    // Body
    if ((1U & (~ (IData)(vlSelf->__VicoTriggered.any())))) {
        VL_DBG_MSGF("         No triggers active\n");
    }
    if ((1ULL & vlSelf->__VicoTriggered.word(0U))) {
        VL_DBG_MSGF("         'ico' region trigger index 0 is active: Internal 'ico' trigger - first iteration\n");
    }
}
#endif  // VL_DEBUG

#ifdef VL_DEBUG
VL_ATTR_COLD void Vslot_r5b_top___024root___dump_triggers__act(Vslot_r5b_top___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vslot_r5b_top__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vslot_r5b_top___024root___dump_triggers__act\n"); );
    // Body
    if ((1U & (~ (IData)(vlSelf->__VactTriggered.any())))) {
        VL_DBG_MSGF("         No triggers active\n");
    }
    if ((1ULL & vlSelf->__VactTriggered.word(0U))) {
        VL_DBG_MSGF("         'act' region trigger index 0 is active: @(posedge CLK or negedge RESET_n)\n");
    }
}
#endif  // VL_DEBUG

#ifdef VL_DEBUG
VL_ATTR_COLD void Vslot_r5b_top___024root___dump_triggers__nba(Vslot_r5b_top___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vslot_r5b_top__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vslot_r5b_top___024root___dump_triggers__nba\n"); );
    // Body
    if ((1U & (~ (IData)(vlSelf->__VnbaTriggered.any())))) {
        VL_DBG_MSGF("         No triggers active\n");
    }
    if ((1ULL & vlSelf->__VnbaTriggered.word(0U))) {
        VL_DBG_MSGF("         'nba' region trigger index 0 is active: @(posedge CLK or negedge RESET_n)\n");
    }
}
#endif  // VL_DEBUG

VL_ATTR_COLD void Vslot_r5b_top___024root___ctor_var_reset(Vslot_r5b_top___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vslot_r5b_top__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vslot_r5b_top___024root___ctor_var_reset\n"); );
    // Body
    vlSelf->CLK = VL_RAND_RESET_I(1);
    vlSelf->RESET_n = VL_RAND_RESET_I(1);
    vlSelf->bus_reset_n = VL_RAND_RESET_I(1);
    vlSelf->sample_tick = VL_RAND_RESET_I(1);
    vlSelf->fnum = VL_RAND_RESET_I(10);
    vlSelf->octave = VL_RAND_RESET_I(4);
    vlSelf->key_on = VL_RAND_RESET_I(1);
    vlSelf->wave_num = VL_RAND_RESET_I(9);
    vlSelf->sample_out = VL_RAND_RESET_I(16);
    vlSelf->phase_acc_out = VL_RAND_RESET_I(32);
    vlSelf->dbg_start_addr = VL_RAND_RESET_I(24);
    vlSelf->dbg_byte_a = VL_RAND_RESET_I(8);
    vlSelf->dbg_byte_b = VL_RAND_RESET_I(8);
    vlSelf->dbg_fetch_state = VL_RAND_RESET_I(4);
    vlSelf->dbg_ram_timing = VL_RAND_RESET_I(1);
    vlSelf->dbg_need_header = VL_RAND_RESET_I(1);
    vlSelf->dbg_header_valid = VL_RAND_RESET_I(1);
    vlSelf->dbg_hdr0 = VL_RAND_RESET_I(8);
    vlSelf->dbg_hdr1 = VL_RAND_RESET_I(8);
    vlSelf->dbg_ram_addr = VL_RAND_RESET_I(24);
    vlSelf->dbg_ram_oe_n = VL_RAND_RESET_I(1);
    vlSelf->dbg_ram_ack_n = VL_RAND_RESET_I(1);
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__phase_acc = VL_RAND_RESET_I(32);
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__interp_out = VL_RAND_RESET_I(16);
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__sample_out_reg = VL_RAND_RESET_I(16);
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_phase__DOT__key_on_prev = VL_RAND_RESET_I(1);
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state = VL_RAND_RESET_I(4);
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__last_idx_fetched = VL_RAND_RESET_I(16);
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__key_on_prev_internal = VL_RAND_RESET_I(1);
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__need_header = VL_RAND_RESET_I(1);
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_valid = VL_RAND_RESET_I(1);
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr0 = VL_RAND_RESET_I(8);
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr1 = VL_RAND_RESET_I(8);
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__start_addr_real = VL_RAND_RESET_I(24);
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_base = VL_RAND_RESET_I(24);
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__byte_a = VL_RAND_RESET_I(8);
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__byte_b = VL_RAND_RESET_I(8);
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_interp__DOT__u_interp__DOT__sum = VL_RAND_RESET_I(18);
    vlSelf->slot_r5b_top__DOT__u_sdram__DOT__state = VL_RAND_RESET_I(7);
    vlSelf->slot_r5b_top__DOT__u_sdram__DOT__save_addr = VL_RAND_RESET_I(24);
    vlSelf->slot_r5b_top__DOT__u_sdram__DOT__cmd_is_read = VL_RAND_RESET_I(1);
    vlSelf->__Vtrigprevexpr___TOP__CLK__0 = VL_RAND_RESET_I(1);
    vlSelf->__Vtrigprevexpr___TOP__RESET_n__0 = VL_RAND_RESET_I(1);
}
