// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vslot_r5b_top.h for the primary calling header

#include "Vslot_r5b_top__pch.h"
#include "Vslot_r5b_top_RAM_IF.h"
#include "Vslot_r5b_top__Syms.h"

void Vslot_r5b_top_RAM_IF___ctor_var_reset(Vslot_r5b_top_RAM_IF* vlSelf);

Vslot_r5b_top_RAM_IF::Vslot_r5b_top_RAM_IF(Vslot_r5b_top__Syms* symsp, const char* v__name)
    : VerilatedModule{v__name}
    , vlSymsp{symsp}
 {
    // Reset structure values
    Vslot_r5b_top_RAM_IF___ctor_var_reset(this);
}

void Vslot_r5b_top_RAM_IF::__Vconfigure(bool first) {
    if (false && first) {}  // Prevent unused
}

Vslot_r5b_top_RAM_IF::~Vslot_r5b_top_RAM_IF() {
}
