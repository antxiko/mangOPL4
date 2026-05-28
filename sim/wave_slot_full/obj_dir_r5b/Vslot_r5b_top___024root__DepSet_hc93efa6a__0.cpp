// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vslot_r5b_top.h for the primary calling header

#include "Vslot_r5b_top__pch.h"
#include "Vslot_r5b_top__Syms.h"
#include "Vslot_r5b_top___024root.h"

#ifdef VL_DEBUG
VL_ATTR_COLD void Vslot_r5b_top___024root___dump_triggers__ico(Vslot_r5b_top___024root* vlSelf);
#endif  // VL_DEBUG

void Vslot_r5b_top___024root___eval_triggers__ico(Vslot_r5b_top___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vslot_r5b_top__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vslot_r5b_top___024root___eval_triggers__ico\n"); );
    // Body
    vlSelf->__VicoTriggered.set(0U, (IData)(vlSelf->__VicoFirstIteration));
#ifdef VL_DEBUG
    if (VL_UNLIKELY(vlSymsp->_vm_contextp__->debug())) {
        Vslot_r5b_top___024root___dump_triggers__ico(vlSelf);
    }
#endif
}

#ifdef VL_DEBUG
VL_ATTR_COLD void Vslot_r5b_top___024root___dump_triggers__act(Vslot_r5b_top___024root* vlSelf);
#endif  // VL_DEBUG

void Vslot_r5b_top___024root___eval_triggers__act(Vslot_r5b_top___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vslot_r5b_top__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vslot_r5b_top___024root___eval_triggers__act\n"); );
    // Body
    vlSelf->__VactTriggered.set(0U, (((IData)(vlSelf->CLK) 
                                      & (~ (IData)(vlSelf->__Vtrigprevexpr___TOP__CLK__0))) 
                                     | ((~ (IData)(vlSelf->RESET_n)) 
                                        & (IData)(vlSelf->__Vtrigprevexpr___TOP__RESET_n__0))));
    vlSelf->__Vtrigprevexpr___TOP__CLK__0 = vlSelf->CLK;
    vlSelf->__Vtrigprevexpr___TOP__RESET_n__0 = vlSelf->RESET_n;
#ifdef VL_DEBUG
    if (VL_UNLIKELY(vlSymsp->_vm_contextp__->debug())) {
        Vslot_r5b_top___024root___dump_triggers__act(vlSelf);
    }
#endif
}

VL_INLINE_OPT void Vslot_r5b_top___024root___nba_sequent__TOP__0(Vslot_r5b_top___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vslot_r5b_top__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vslot_r5b_top___024root___nba_sequent__TOP__0\n"); );
    // Init
    IData/*31:0*/ __Vdly__slot_r5b_top__DOT__u_slot__DOT__phase_acc;
    __Vdly__slot_r5b_top__DOT__u_slot__DOT__phase_acc = 0;
    CData/*3:0*/ __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state;
    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state = 0;
    SData/*15:0*/ __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__last_idx_fetched;
    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__last_idx_fetched = 0;
    CData/*0:0*/ __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__need_header;
    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__need_header = 0;
    CData/*0:0*/ __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_valid;
    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_valid = 0;
    CData/*7:0*/ __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr0;
    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr0 = 0;
    CData/*7:0*/ __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr1;
    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr1 = 0;
    IData/*23:0*/ __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__start_addr_real;
    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__start_addr_real = 0;
    CData/*6:0*/ __Vdly__slot_r5b_top__DOT__u_sdram__DOT__state;
    __Vdly__slot_r5b_top__DOT__u_sdram__DOT__state = 0;
    IData/*23:0*/ __Vdly__slot_r5b_top__DOT__u_sdram__DOT__save_addr;
    __Vdly__slot_r5b_top__DOT__u_sdram__DOT__save_addr = 0;
    CData/*0:0*/ __Vdly__slot_r5b_top__DOT__u_sdram__DOT__cmd_is_read;
    __Vdly__slot_r5b_top__DOT__u_sdram__DOT__cmd_is_read = 0;
    IData/*23:0*/ TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__ADDR;
    TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__ADDR = 0;
    CData/*0:0*/ TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__OE_n;
    TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__OE_n = 0;
    // Body
    __Vdly__slot_r5b_top__DOT__u_sdram__DOT__cmd_is_read 
        = vlSelf->slot_r5b_top__DOT__u_sdram__DOT__cmd_is_read;
    __Vdly__slot_r5b_top__DOT__u_sdram__DOT__save_addr 
        = vlSelf->slot_r5b_top__DOT__u_sdram__DOT__save_addr;
    __Vdly__slot_r5b_top__DOT__u_sdram__DOT__state 
        = vlSelf->slot_r5b_top__DOT__u_sdram__DOT__state;
    __Vdly__slot_r5b_top__DOT__u_slot__DOT__phase_acc 
        = vlSelf->slot_r5b_top__DOT__u_slot__DOT__phase_acc;
    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__last_idx_fetched 
        = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__last_idx_fetched;
    TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__OE_n 
        = vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.OE_n;
    TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__ADDR 
        = vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ADDR;
    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__start_addr_real 
        = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__start_addr_real;
    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr1 
        = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr1;
    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr0 
        = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr0;
    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_valid 
        = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_valid;
    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__need_header 
        = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__need_header;
    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state 
        = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state;
    if ((1U & ((~ (IData)(vlSelf->RESET_n)) | (~ (IData)(vlSelf->bus_reset_n))))) {
        vlSelf->slot_r5b_top__DOT__u_slot__DOT__sample_out_reg = 0U;
        __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state = 0U;
        __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__last_idx_fetched = 0xffffU;
        __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__need_header = 0U;
        __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_valid = 0U;
        __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr0 = 0U;
        __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr1 = 0U;
        __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__start_addr_real = 0x100000U;
        vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__byte_a = 0x80U;
        vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__byte_b = 0x80U;
        TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__ADDR = 0U;
        TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__OE_n = 1U;
    } else {
        vlSelf->slot_r5b_top__DOT__u_slot__DOT__sample_out_reg 
            = ((IData)(vlSelf->key_on) ? (IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__interp_out)
                : 0U);
        if (((IData)(vlSelf->key_on) & (~ (IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__key_on_prev_internal)))) {
            __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__need_header = 1U;
            __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_valid = 0U;
            __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__last_idx_fetched = 0xffffU;
        }
        TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__ADDR = 0U;
        TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__OE_n = 1U;
        if ((8U & (IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state))) {
            if ((4U & (IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state))) {
                __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state = 0U;
            } else if ((2U & (IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state))) {
                if ((1U & (IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state))) {
                    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state = 0U;
                } else if (vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ACK_n) {
                    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__byte_b 
                        = (0xffU & vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.DOUT);
                    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__last_idx_fetched 
                        = (vlSelf->slot_r5b_top__DOT__u_slot__DOT__phase_acc 
                           >> 0x10U);
                    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state = 0U;
                }
            } else if ((1U & (IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state))) {
                if ((0x20U == (IData)(vlSelf->slot_r5b_top__DOT__u_sdram__DOT__state))) {
                    TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__ADDR 
                        = (0xffffffU & ((IData)(1U) 
                                        + (vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__start_addr_real 
                                           + (vlSelf->slot_r5b_top__DOT__u_slot__DOT__phase_acc 
                                              >> 0x10U))));
                    TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__OE_n = 0U;
                }
                if ((1U & (~ (IData)(vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ACK_n)))) {
                    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state = 0xaU;
                }
            } else if (vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ACK_n) {
                vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__byte_a 
                    = (0xffU & vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.DOUT);
                __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state = 9U;
            }
        } else if ((4U & (IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state))) {
            if ((2U & (IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state))) {
                if ((1U & (IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state))) {
                    TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__ADDR 
                        = (0xffffffU & (vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__start_addr_real 
                                        + (vlSelf->slot_r5b_top__DOT__u_slot__DOT__phase_acc 
                                           >> 0x10U)));
                    if ((1U & (~ (IData)(vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ACK_n)))) {
                        __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state = 8U;
                    }
                } else if (vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ACK_n) {
                    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__start_addr_real 
                        = (0xffffffU & ((IData)(0x100000U) 
                                        + ((0x3f0000U 
                                            & ((IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr0) 
                                               << 0x10U)) 
                                           | (((IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr1) 
                                               << 8U) 
                                              | (0xffU 
                                                 & vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.DOUT)))));
                    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_valid = 1U;
                    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state = 0U;
                }
            } else if ((1U & (IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state))) {
                if ((0x20U == (IData)(vlSelf->slot_r5b_top__DOT__u_sdram__DOT__state))) {
                    TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__ADDR 
                        = (0xffffffU & ((IData)(2U) 
                                        + vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_base));
                    TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__OE_n = 0U;
                }
                if ((1U & (~ (IData)(vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ACK_n)))) {
                    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state = 6U;
                }
            } else if (vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ACK_n) {
                __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr1 
                    = (0xffU & vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.DOUT);
                __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state = 5U;
            }
        } else if ((2U & (IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state))) {
            if ((1U & (IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state))) {
                if ((0x20U == (IData)(vlSelf->slot_r5b_top__DOT__u_sdram__DOT__state))) {
                    TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__ADDR 
                        = (0xffffffU & ((IData)(1U) 
                                        + vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_base));
                    TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__OE_n = 0U;
                }
                if ((1U & (~ (IData)(vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ACK_n)))) {
                    __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state = 4U;
                }
            } else if (vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ACK_n) {
                __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr0 
                    = (0xffU & vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.DOUT);
                __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state = 3U;
            }
        } else if ((1U & (IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state))) {
            TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__ADDR 
                = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_base;
            if ((1U & (~ (IData)(vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ACK_n)))) {
                __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state = 2U;
            }
        } else if (((IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__need_header) 
                    & (0x20U == (IData)(vlSelf->slot_r5b_top__DOT__u_sdram__DOT__state)))) {
            __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__need_header = 0U;
            __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state = 1U;
            TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__ADDR 
                = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_base;
            TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__OE_n = 0U;
        } else if (((((IData)(vlSelf->key_on) & (IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_valid)) 
                     & ((vlSelf->slot_r5b_top__DOT__u_slot__DOT__phase_acc 
                         >> 0x10U) != (IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__last_idx_fetched))) 
                    & (0x20U == (IData)(vlSelf->slot_r5b_top__DOT__u_sdram__DOT__state)))) {
            __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state = 7U;
            TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__ADDR 
                = (0xffffffU & (vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__start_addr_real 
                                + (vlSelf->slot_r5b_top__DOT__u_slot__DOT__phase_acc 
                                   >> 0x10U)));
            TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__OE_n = 0U;
        }
    }
    if (vlSelf->RESET_n) {
        if (((IData)(vlSelf->key_on) & (~ (IData)(vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_phase__DOT__key_on_prev)))) {
            __Vdly__slot_r5b_top__DOT__u_slot__DOT__phase_acc = 0U;
        } else if (((IData)(vlSelf->sample_tick) & (IData)(vlSelf->key_on))) {
            __Vdly__slot_r5b_top__DOT__u_slot__DOT__phase_acc 
                = (vlSelf->slot_r5b_top__DOT__u_slot__DOT__phase_acc 
                   + (VL_LTES_III(32, 0U, VL_EXTENDS_II(32,4, (IData)(vlSelf->octave)))
                       ? ((0x400U | (IData)(vlSelf->fnum)) 
                          << (IData)(vlSelf->octave))
                       : ((0x400U | (IData)(vlSelf->fnum)) 
                          >> (0xfU & (- (IData)(vlSelf->octave))))));
        }
        if ((0x20U == (IData)(vlSelf->slot_r5b_top__DOT__u_sdram__DOT__state))) {
            if ((1U & ((~ (IData)(vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.WE_n)) 
                       | (~ (IData)(vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.OE_n))))) {
                __Vdly__slot_r5b_top__DOT__u_sdram__DOT__state 
                    = (0x7fU & ((IData)(1U) + (IData)(vlSelf->slot_r5b_top__DOT__u_sdram__DOT__state)));
                __Vdly__slot_r5b_top__DOT__u_sdram__DOT__save_addr 
                    = vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ADDR;
                __Vdly__slot_r5b_top__DOT__u_sdram__DOT__cmd_is_read 
                    = (1U & (~ (IData)(vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.OE_n)));
                vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ACK_n = 0U;
            }
        } else if ((0x27U == (IData)(vlSelf->slot_r5b_top__DOT__u_sdram__DOT__state))) {
            __Vdly__slot_r5b_top__DOT__u_sdram__DOT__state 
                = (0x7fU & ((IData)(1U) + (IData)(vlSelf->slot_r5b_top__DOT__u_sdram__DOT__state)));
            vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ACK_n = 1U;
            if (vlSelf->slot_r5b_top__DOT__u_sdram__DOT__cmd_is_read) {
                vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.DOUT 
                    = (0xffU & vlSelf->slot_r5b_top__DOT__u_sdram__DOT__save_addr);
            }
        } else if ((0x28U == (IData)(vlSelf->slot_r5b_top__DOT__u_sdram__DOT__state))) {
            __Vdly__slot_r5b_top__DOT__u_sdram__DOT__state = 0x20U;
            __Vdly__slot_r5b_top__DOT__u_sdram__DOT__cmd_is_read = 0U;
        } else {
            __Vdly__slot_r5b_top__DOT__u_sdram__DOT__state 
                = (0x7fU & ((IData)(1U) + (IData)(vlSelf->slot_r5b_top__DOT__u_sdram__DOT__state)));
        }
    } else {
        __Vdly__slot_r5b_top__DOT__u_slot__DOT__phase_acc = 0U;
        __Vdly__slot_r5b_top__DOT__u_sdram__DOT__state = 0x20U;
        vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ACK_n = 1U;
        vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.DOUT = 0U;
        __Vdly__slot_r5b_top__DOT__u_sdram__DOT__cmd_is_read = 0U;
        __Vdly__slot_r5b_top__DOT__u_sdram__DOT__save_addr = 0U;
    }
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__interp_out 
        = ((1U & ((~ (IData)(vlSelf->RESET_n)) | (~ (IData)(vlSelf->bus_reset_n))))
            ? 0U : (VL_LTS_III(18, 0x7fffU, vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_interp__DOT__u_interp__DOT__sum)
                     ? 0x7fffU : (VL_GTS_III(18, 0x38000U, vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_interp__DOT__u_interp__DOT__sum)
                                   ? 0x8000U : (0xffffU 
                                                & vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_interp__DOT__u_interp__DOT__sum))));
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__last_idx_fetched 
        = __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__last_idx_fetched;
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state 
        = __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state;
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__need_header 
        = __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__need_header;
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_valid 
        = __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_valid;
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr0 
        = __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr0;
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr1 
        = __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr1;
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__start_addr_real 
        = __Vdly__slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__start_addr_real;
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__phase_acc 
        = __Vdly__slot_r5b_top__DOT__u_slot__DOT__phase_acc;
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_phase__DOT__key_on_prev 
        = ((IData)(vlSelf->RESET_n) && (IData)(vlSelf->key_on));
    vlSelf->sample_out = vlSelf->slot_r5b_top__DOT__u_slot__DOT__sample_out_reg;
    vlSelf->dbg_fetch_state = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__state;
    vlSelf->dbg_need_header = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__need_header;
    vlSelf->dbg_header_valid = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__header_valid;
    vlSelf->dbg_hdr0 = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr0;
    vlSelf->dbg_hdr1 = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__hdr1;
    vlSelf->dbg_start_addr = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__start_addr_real;
    vlSelf->dbg_byte_a = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__byte_a;
    vlSelf->dbg_byte_b = vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__byte_b;
    vlSelf->phase_acc_out = vlSelf->slot_r5b_top__DOT__u_slot__DOT__phase_acc;
    vlSelf->slot_r5b_top__DOT__u_slot__DOT__u_fetch1__DOT__key_on_prev_internal 
        = ((1U & (~ ((~ (IData)(vlSelf->RESET_n)) | 
                     (~ (IData)(vlSelf->bus_reset_n))))) 
           && (IData)(vlSelf->key_on));
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
    vlSelf->slot_r5b_top__DOT__u_sdram__DOT__save_addr 
        = __Vdly__slot_r5b_top__DOT__u_sdram__DOT__save_addr;
    vlSelf->slot_r5b_top__DOT__u_sdram__DOT__cmd_is_read 
        = __Vdly__slot_r5b_top__DOT__u_sdram__DOT__cmd_is_read;
    vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.OE_n 
        = TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__OE_n;
    vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ADDR 
        = TOP__slot_r5b_top__DOT__Ram_slot__DOT____Vdly__ADDR;
    vlSelf->slot_r5b_top__DOT__u_sdram__DOT__state 
        = __Vdly__slot_r5b_top__DOT__u_sdram__DOT__state;
    vlSelf->dbg_ram_oe_n = vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.OE_n;
    vlSelf->dbg_ram_addr = vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ADDR;
    vlSelf->dbg_ram_timing = (0x20U == (IData)(vlSelf->slot_r5b_top__DOT__u_sdram__DOT__state));
    vlSelf->dbg_ram_ack_n = vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.ACK_n;
    vlSymsp->TOP__slot_r5b_top__DOT__Ram_slot.WE_n = 1U;
}
