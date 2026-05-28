// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Symbol table implementation internals

#include "Vslot_r5b_top__pch.h"
#include "Vslot_r5b_top.h"
#include "Vslot_r5b_top___024root.h"
#include "Vslot_r5b_top_RAM_IF.h"

// FUNCTIONS
Vslot_r5b_top__Syms::~Vslot_r5b_top__Syms()
{
}

Vslot_r5b_top__Syms::Vslot_r5b_top__Syms(VerilatedContext* contextp, const char* namep, Vslot_r5b_top* modelp)
    : VerilatedSyms{contextp}
    // Setup internal state of the Syms class
    , __Vm_modelp{modelp}
    // Setup module instances
    , TOP{this, namep}
    , TOP__slot_r5b_top__DOT__Ram_slot{this, Verilated::catName(namep, "slot_r5b_top.Ram_slot")}
{
    // Configure time unit / time precision
    _vm_contextp__->timeunit(-12);
    _vm_contextp__->timeprecision(-12);
    // Setup each module's pointers to their submodules
    TOP.__PVT__slot_r5b_top__DOT__Ram_slot = &TOP__slot_r5b_top__DOT__Ram_slot;
    // Setup each module's pointer back to symbol table (for public functions)
    TOP.__Vconfigure(true);
    TOP__slot_r5b_top__DOT__Ram_slot.__Vconfigure(true);
}
