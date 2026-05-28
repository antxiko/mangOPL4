// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vslot_r5b_top.h for the primary calling header

#include "Vslot_r5b_top__pch.h"
#include "Vslot_r5b_top_RAM_IF.h"

VL_ATTR_COLD void Vslot_r5b_top_RAM_IF___ctor_var_reset(Vslot_r5b_top_RAM_IF* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vslot_r5b_top__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vslot_r5b_top_RAM_IF___ctor_var_reset\n"); );
    // Body
    vlSelf->ADDR = VL_RAND_RESET_I(24);
    vlSelf->OE_n = VL_RAND_RESET_I(1);
    vlSelf->WE_n = VL_RAND_RESET_I(1);
    vlSelf->DOUT = VL_RAND_RESET_I(32);
    vlSelf->ACK_n = VL_RAND_RESET_I(1);
}
