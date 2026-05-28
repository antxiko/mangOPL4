// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Model implementation (design independent parts)

#include "Vslot_r5b_top__pch.h"

//============================================================
// Constructors

Vslot_r5b_top::Vslot_r5b_top(VerilatedContext* _vcontextp__, const char* _vcname__)
    : VerilatedModel{*_vcontextp__}
    , vlSymsp{new Vslot_r5b_top__Syms(contextp(), _vcname__, this)}
    , CLK{vlSymsp->TOP.CLK}
    , RESET_n{vlSymsp->TOP.RESET_n}
    , bus_reset_n{vlSymsp->TOP.bus_reset_n}
    , sample_tick{vlSymsp->TOP.sample_tick}
    , octave{vlSymsp->TOP.octave}
    , key_on{vlSymsp->TOP.key_on}
    , dbg_byte_a{vlSymsp->TOP.dbg_byte_a}
    , dbg_byte_b{vlSymsp->TOP.dbg_byte_b}
    , dbg_fetch_state{vlSymsp->TOP.dbg_fetch_state}
    , dbg_ram_timing{vlSymsp->TOP.dbg_ram_timing}
    , dbg_need_header{vlSymsp->TOP.dbg_need_header}
    , dbg_header_valid{vlSymsp->TOP.dbg_header_valid}
    , dbg_hdr0{vlSymsp->TOP.dbg_hdr0}
    , dbg_hdr1{vlSymsp->TOP.dbg_hdr1}
    , dbg_ram_oe_n{vlSymsp->TOP.dbg_ram_oe_n}
    , dbg_ram_ack_n{vlSymsp->TOP.dbg_ram_ack_n}
    , fnum{vlSymsp->TOP.fnum}
    , wave_num{vlSymsp->TOP.wave_num}
    , sample_out{vlSymsp->TOP.sample_out}
    , phase_acc_out{vlSymsp->TOP.phase_acc_out}
    , dbg_start_addr{vlSymsp->TOP.dbg_start_addr}
    , dbg_ram_addr{vlSymsp->TOP.dbg_ram_addr}
    , __PVT__slot_r5b_top__DOT__Ram_slot{vlSymsp->TOP.__PVT__slot_r5b_top__DOT__Ram_slot}
    , rootp{&(vlSymsp->TOP)}
{
    // Register model with the context
    contextp()->addModel(this);
}

Vslot_r5b_top::Vslot_r5b_top(const char* _vcname__)
    : Vslot_r5b_top(Verilated::threadContextp(), _vcname__)
{
}

//============================================================
// Destructor

Vslot_r5b_top::~Vslot_r5b_top() {
    delete vlSymsp;
}

//============================================================
// Evaluation function

#ifdef VL_DEBUG
void Vslot_r5b_top___024root___eval_debug_assertions(Vslot_r5b_top___024root* vlSelf);
#endif  // VL_DEBUG
void Vslot_r5b_top___024root___eval_static(Vslot_r5b_top___024root* vlSelf);
void Vslot_r5b_top___024root___eval_initial(Vslot_r5b_top___024root* vlSelf);
void Vslot_r5b_top___024root___eval_settle(Vslot_r5b_top___024root* vlSelf);
void Vslot_r5b_top___024root___eval(Vslot_r5b_top___024root* vlSelf);

void Vslot_r5b_top::eval_step() {
    VL_DEBUG_IF(VL_DBG_MSGF("+++++TOP Evaluate Vslot_r5b_top::eval_step\n"); );
#ifdef VL_DEBUG
    // Debug assertions
    Vslot_r5b_top___024root___eval_debug_assertions(&(vlSymsp->TOP));
#endif  // VL_DEBUG
    vlSymsp->__Vm_deleter.deleteAll();
    if (VL_UNLIKELY(!vlSymsp->__Vm_didInit)) {
        vlSymsp->__Vm_didInit = true;
        VL_DEBUG_IF(VL_DBG_MSGF("+ Initial\n"););
        Vslot_r5b_top___024root___eval_static(&(vlSymsp->TOP));
        Vslot_r5b_top___024root___eval_initial(&(vlSymsp->TOP));
        Vslot_r5b_top___024root___eval_settle(&(vlSymsp->TOP));
    }
    VL_DEBUG_IF(VL_DBG_MSGF("+ Eval\n"););
    Vslot_r5b_top___024root___eval(&(vlSymsp->TOP));
    // Evaluate cleanup
    Verilated::endOfEval(vlSymsp->__Vm_evalMsgQp);
}

//============================================================
// Events and timing
bool Vslot_r5b_top::eventsPending() { return false; }

uint64_t Vslot_r5b_top::nextTimeSlot() {
    VL_FATAL_MT(__FILE__, __LINE__, "", "%Error: No delays in the design");
    return 0;
}

//============================================================
// Utilities

const char* Vslot_r5b_top::name() const {
    return vlSymsp->name();
}

//============================================================
// Invoke final blocks

void Vslot_r5b_top___024root___eval_final(Vslot_r5b_top___024root* vlSelf);

VL_ATTR_COLD void Vslot_r5b_top::final() {
    Vslot_r5b_top___024root___eval_final(&(vlSymsp->TOP));
}

//============================================================
// Implementations of abstract methods from VerilatedModel

const char* Vslot_r5b_top::hierName() const { return vlSymsp->name(); }
const char* Vslot_r5b_top::modelName() const { return "Vslot_r5b_top"; }
unsigned Vslot_r5b_top::threads() const { return 1; }
void Vslot_r5b_top::prepareClone() const { contextp()->prepareClone(); }
void Vslot_r5b_top::atClone() const {
    contextp()->threadPoolpOnClone();
}

//============================================================
// Trace configuration

VL_ATTR_COLD void Vslot_r5b_top::trace(VerilatedVcdC* tfp, int levels, int options) {
    vl_fatal(__FILE__, __LINE__, __FILE__,"'Vslot_r5b_top::trace()' called on model that was Verilated without --trace option");
}
