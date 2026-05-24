TNCART := /mnt/c/Users/Antxiko/Documents/frutOPL4/external/tnCartWonder/rtl/src
LOCAL  := /mnt/c/Users/Antxiko/Documents/frutOPL4/sim/wave_slot_full

PKG_SRC := \
	$(TNCART)/wave/ymf278_pkg.sv

# ram.sv tiene RAM_IF interface + EXPANSION_RAM modules
RTL_SRC := \
	$(TNCART)/peripheral/ram/ram.sv \
	$(TNCART)/wave/ymf278_phase.sv \
	$(TNCART)/wave/ymf278_fetch1.sv \
	$(TNCART)/wave/ymf278_interp.sv \
	$(TNCART)/wave/ymf278_interp_reg.sv \
	$(TNCART)/wave/ymf278_slot.sv \
	$(LOCAL)/sdram_stub_yrw.sv \
	$(LOCAL)/slot_full_top.sv

VFLAGS := --cc --exe --build -j 0 \
	-Wno-WIDTHEXPAND -Wno-WIDTHTRUNC -Wno-ASCRANGE \
	-Wno-MULTIDRIVEN -Wno-LATCH -Wno-UNOPTFLAT \
	-Wno-PINMISSING -Wno-UNUSEDSIGNAL -Wno-UNUSEDPARAM \
	-Wno-CASEINCOMPLETE \
	--top-module slot_full_top \
	-CFLAGS "-O2"

all: obj_dir/Vslot_full_top

obj_dir/Vslot_full_top: tb_slot.cpp $(PKG_SRC) $(RTL_SRC)
	verilator $(VFLAGS) $(PKG_SRC) $(RTL_SRC) tb_slot.cpp

run: obj_dir/Vslot_full_top
	./obj_dir/Vslot_full_top

clean:
	rm -rf obj_dir

.PHONY: all run clean
