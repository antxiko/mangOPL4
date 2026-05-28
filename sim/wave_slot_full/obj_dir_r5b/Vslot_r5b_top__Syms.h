// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Symbol table internal header
//
// Internal details; most calling programs do not need this header,
// unless using verilator public meta comments.

#ifndef VERILATED_VSLOT_R5B_TOP__SYMS_H_
#define VERILATED_VSLOT_R5B_TOP__SYMS_H_  // guard

#include "verilated.h"

// INCLUDE MODEL CLASS

#include "Vslot_r5b_top.h"

// INCLUDE MODULE CLASSES
#include "Vslot_r5b_top___024root.h"
#include "Vslot_r5b_top_RAM_IF.h"

// SYMS CLASS (contains all model state)
class alignas(VL_CACHE_LINE_BYTES)Vslot_r5b_top__Syms final : public VerilatedSyms {
  public:
    // INTERNAL STATE
    Vslot_r5b_top* const __Vm_modelp;
    VlDeleter __Vm_deleter;
    bool __Vm_didInit = false;

    // MODULE INSTANCE STATE
    Vslot_r5b_top___024root        TOP;
    Vslot_r5b_top_RAM_IF           TOP__slot_r5b_top__DOT__Ram_slot;

    // CONSTRUCTORS
    Vslot_r5b_top__Syms(VerilatedContext* contextp, const char* namep, Vslot_r5b_top* modelp);
    ~Vslot_r5b_top__Syms();

    // METHODS
    const char* name() { return TOP.name(); }
};

#endif  // guard
