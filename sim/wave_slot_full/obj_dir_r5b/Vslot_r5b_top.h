// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Primary model header
//
// This header should be included by all source files instantiating the design.
// The class here is then constructed to instantiate the design.
// See the Verilator manual for examples.

#ifndef VERILATED_VSLOT_R5B_TOP_H_
#define VERILATED_VSLOT_R5B_TOP_H_  // guard

#include "verilated.h"

class Vslot_r5b_top__Syms;
class Vslot_r5b_top___024root;
class Vslot_r5b_top_RAM_IF;


// This class is the main interface to the Verilated model
class alignas(VL_CACHE_LINE_BYTES) Vslot_r5b_top VL_NOT_FINAL : public VerilatedModel {
  private:
    // Symbol table holding complete model state (owned by this class)
    Vslot_r5b_top__Syms* const vlSymsp;

  public:

    // PORTS
    // The application code writes and reads these signals to
    // propagate new values into/out from the Verilated model.
    VL_IN8(&CLK,0,0);
    VL_IN8(&RESET_n,0,0);
    VL_IN8(&bus_reset_n,0,0);
    VL_IN8(&sample_tick,0,0);
    VL_IN8(&octave,3,0);
    VL_IN8(&key_on,0,0);
    VL_OUT8(&dbg_byte_a,7,0);
    VL_OUT8(&dbg_byte_b,7,0);
    VL_OUT8(&dbg_fetch_state,3,0);
    VL_OUT8(&dbg_ram_timing,0,0);
    VL_OUT8(&dbg_need_header,0,0);
    VL_OUT8(&dbg_header_valid,0,0);
    VL_OUT8(&dbg_hdr0,7,0);
    VL_OUT8(&dbg_hdr1,7,0);
    VL_OUT8(&dbg_ram_oe_n,0,0);
    VL_OUT8(&dbg_ram_ack_n,0,0);
    VL_IN16(&fnum,9,0);
    VL_IN16(&wave_num,8,0);
    VL_OUT16(&sample_out,15,0);
    VL_OUT(&phase_acc_out,31,0);
    VL_OUT(&dbg_start_addr,23,0);
    VL_OUT(&dbg_ram_addr,23,0);

    // CELLS
    // Public to allow access to /* verilator public */ items.
    // Otherwise the application code can consider these internals.
    Vslot_r5b_top_RAM_IF* const __PVT__slot_r5b_top__DOT__Ram_slot;

    // Root instance pointer to allow access to model internals,
    // including inlined /* verilator public_flat_* */ items.
    Vslot_r5b_top___024root* const rootp;

    // CONSTRUCTORS
    /// Construct the model; called by application code
    /// If contextp is null, then the model will use the default global context
    /// If name is "", then makes a wrapper with a
    /// single model invisible with respect to DPI scope names.
    explicit Vslot_r5b_top(VerilatedContext* contextp, const char* name = "TOP");
    explicit Vslot_r5b_top(const char* name = "TOP");
    /// Destroy the model; called (often implicitly) by application code
    virtual ~Vslot_r5b_top();
  private:
    VL_UNCOPYABLE(Vslot_r5b_top);  ///< Copying not allowed

  public:
    // API METHODS
    /// Evaluate the model.  Application must call when inputs change.
    void eval() { eval_step(); }
    /// Evaluate when calling multiple units/models per time step.
    void eval_step();
    /// Evaluate at end of a timestep for tracing, when using eval_step().
    /// Application must call after all eval() and before time changes.
    void eval_end_step() {}
    /// Simulation complete, run final blocks.  Application must call on completion.
    void final();
    /// Are there scheduled events to handle?
    bool eventsPending();
    /// Returns time at next time slot. Aborts if !eventsPending()
    uint64_t nextTimeSlot();
    /// Trace signals in the model; called by application code
    void trace(VerilatedVcdC* tfp, int levels, int options = 0);
    /// Retrieve name of this model instance (as passed to constructor).
    const char* name() const;

    // Abstract methods from VerilatedModel
    const char* hierName() const override final;
    const char* modelName() const override final;
    unsigned threads() const override final;
    /// Prepare for cloning the model at the process level (e.g. fork in Linux)
    /// Release necessary resources. Called before cloning.
    void prepareClone() const;
    /// Re-init after cloning the model at the process level (e.g. fork in Linux)
    /// Re-allocate necessary resources. Called after cloning.
    void atClone() const;
};

#endif  // guard
