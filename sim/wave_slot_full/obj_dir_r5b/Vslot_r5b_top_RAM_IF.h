// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design internal header
// See Vslot_r5b_top.h for the primary calling header

#ifndef VERILATED_VSLOT_R5B_TOP_RAM_IF_H_
#define VERILATED_VSLOT_R5B_TOP_RAM_IF_H_  // guard

#include "verilated.h"


class Vslot_r5b_top__Syms;

class alignas(VL_CACHE_LINE_BYTES) Vslot_r5b_top_RAM_IF final : public VerilatedModule {
  public:

    // DESIGN SPECIFIC STATE
    CData/*0:0*/ OE_n;
    CData/*0:0*/ WE_n;
    CData/*0:0*/ ACK_n;
    IData/*23:0*/ ADDR;
    IData/*31:0*/ DOUT;

    // INTERNAL VARIABLES
    Vslot_r5b_top__Syms* const vlSymsp;

    // CONSTRUCTORS
    Vslot_r5b_top_RAM_IF(Vslot_r5b_top__Syms* symsp, const char* v__name);
    ~Vslot_r5b_top_RAM_IF();
    VL_UNCOPYABLE(Vslot_r5b_top_RAM_IF);

    // INTERNAL METHODS
    void __Vconfigure(bool first);
};

std::string VL_TO_STRING(const Vslot_r5b_top_RAM_IF* obj);

#endif  // guard
