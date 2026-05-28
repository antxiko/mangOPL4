// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vslot_r5b_top.h for the primary calling header

#include "Vslot_r5b_top__pch.h"
#include "Vslot_r5b_top_RAM_IF.h"

std::string VL_TO_STRING(const Vslot_r5b_top_RAM_IF* obj) {
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vslot_r5b_top_RAM_IF::VL_TO_STRING\n"); );
    // Body
    return (obj ? obj->name() : "null");
}
