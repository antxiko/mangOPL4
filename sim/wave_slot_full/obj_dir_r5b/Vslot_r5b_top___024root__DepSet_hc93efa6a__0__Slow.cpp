// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vslot_r5b_top.h for the primary calling header

#include "Vslot_r5b_top__pch.h"
#include "Vslot_r5b_top__Syms.h"
#include "Vslot_r5b_top___024root.h"

#ifdef VL_DEBUG
VL_ATTR_COLD void Vslot_r5b_top___024root___dump_triggers__stl(Vslot_r5b_top___024root* vlSelf);
#endif  // VL_DEBUG

VL_ATTR_COLD void Vslot_r5b_top___024root___eval_triggers__stl(Vslot_r5b_top___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vslot_r5b_top__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vslot_r5b_top___024root___eval_triggers__stl\n"); );
    // Body
    vlSelf->__VstlTriggered.set(0U, (IData)(vlSelf->__VstlFirstIteration));
#ifdef VL_DEBUG
    if (VL_UNLIKELY(vlSymsp->_vm_contextp__->debug())) {
        Vslot_r5b_top___024root___dump_triggers__stl(vlSelf);
    }
#endif
}

VL_ATTR_COLD void Vslot_r5b_top___024root___stl_sequent__TOP__0(Vslot_r5b_top___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vslot_r5b_top__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vslot_r5b_top___024root___stl_sequent__TOP__0\n"); );
    // Body
    vlSelf->dbg_fetch_state = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state;
    vlSelf->dbg_ram_timing = (0x20U == (IData)(vlSelf->slot_r5b_top__DOT__u_sdram__DOT__state));
    vlSelf->dbg_need_header = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__need_header;
    vlSelf->dbg_header_valid = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_valid;
    vlSelf->dbg_hdr0 = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr0;
    vlSelf->dbg_hdr1 = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr1;
    vlSelf->dbg_ram_addr = vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ADDR;
    vlSelf->dbg_ram_oe_n = vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.OE_n;
    vlSelf->dbg_ram_ack_n = vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ACK_n;
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_base 
        = (0xffffffU & ((IData)(0x100000U) + (((IData)(vlSelf->wave_num) 
                                               << 3U) 
                                              + ((IData)(vlSelf->wave_num) 
                                                 << 2U))));
    vlSelf->sample_out = vlSelf->slot_r5b_top__DOT__u_slot__DOT__sample_out_reg;
    vlSelf->phase_acc_out = vlSelf->slot_r5b_top__DOT__u_slot__DOT__phase_acc;
    vlSelf->dbg_byte_a = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__byte_a;
    vlSelf->dbg_byte_b = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__byte_b;
    vlSelf->dbg_start_addr = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__start_addr_real;
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_interp__DOT__u_interp__DOT__sum 
        = (0x3ffffU & (((0x30000U & ((- (IData)((1U 
                                                 & (~ 
                                                    ((IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__byte_a) 
                                                     >> 7U))))) 
                                     << 0x10U)) | (0x8000U 
                                                   ^ 
                                                   ((IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__byte_a) 
                                                    << 8U))) 
                       + (IData)((0x3ffffULL & (VL_MULS_QQQ(34, 
                                                            (0x3ffffffffULL 
                                                             & VL_EXTENDS_QI(34,17, 
                                                                             (0x1ffffU 
                                                                              & (((0x10000U 
                                                                                & ((~ 
                                                                                ((IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__byte_b) 
                                                                                >> 7U)) 
                                                                                << 0x10U)) 
                                                                                | (0x8000U 
                                                                                ^ 
                                                                                ((IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__byte_b) 
                                                                                << 8U))) 
                                                                                - 
                                                                                ((0x10000U 
                                                                                & ((~ 
                                                                                ((IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__byte_a) 
                                                                                >> 7U)) 
                                                                                << 0x10U)) 
                                                                                | (0x8000U 
                                                                                ^ 
                                                                                ((IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__byte_a) 
                                                                                << 8U))))))), 
                                                            (0x3ffffffffULL 
                                                             & VL_EXTENDS_QI(34,17, 
                                                                             (0xffffU 
                                                                              & vlSelf->slot_r5b_top__DOT__u_slot__DOT__phase_acc)))) 
                                                >> 0x10U)))));
}
