TNCART := /mnt/c/Users/Antxiko/Documents/frutOPL4/external/tnCartWonder/rtl/src
LOCAL  := /mnt/c/Users/Antxiko/Documents/frutOPL4/sim/wave_top
SHARED := /mnt/c/Users/Antxiko/Documents/frutOPL4/sim/wave_slot_full

PKG_SRC := \
	$(TNCART)/wave/ymf278_pkg.sv

RTL_SRC := \
	$(TNCART)/peripheral/ram/ram.sv \
	$(TNCART)/wave/ymf278_regfile.sv \
	$(TNCART)/wave/ymf278_mempointer.sv \
	$(TNCART)/wave/ymf278_phase.sv \
	$(TNCART)/wave/ymf278_fetch1.sv \
	$(TNCART)/wave/ymf278_interp.sv \
	$(TNCART)/wave/ymf278_interp_reg.sv \
	$(TNCART)/wave/ymf278_exp_lut.sv \
	$(TNCART)/wave/ymf278_eg.sv \
	$(TNCART)/wave/ymf278_eg_lut.sv \
	$(TNCART)/wave/ymf278_slot.sv \
	$(TNCART)/wave/ymf278_header_table.sv \
	$(TNCART)/wave/wave_arbiter.sv \
	$(SHARED)/sdram_stub_r5b.sv \
	$(LOCAL)/top_test_top.sv

VFLAGS := --cc --exe --build -j 0 \
	-Wno-WIDTHEXPAND -Wno-WIDTHTRUNC -Wno-ASCRANGE \
	-Wno-MULTIDRIVEN -Wno-LATCH -Wno-UNOPTFLAT \
	-Wno-PINMISSING -Wno-UNUSEDSIGNAL -Wno-UNUSEDPARAM \
	-Wno-CASEINCOMPLETE \
	--unroll-count 512 \
	--top-module top_test_top \
	-CFLAGS "-O2"

# Nota: ymf278_top.sv NO se lista porque top_test_top lo instancia y
# verilator lo encuentra; pero hay que incluirlo explícito:
RTL_SRC += $(TNCART)/wave/ymf278_top.sv

all: obj_dir/Vtop_test_top

obj_dir/Vtop_test_top: tb_top.cpp $(PKG_SRC) $(RTL_SRC)
	verilator $(VFLAGS) $(PKG_SRC) $(RTL_SRC) tb_top.cpp

run: obj_dir/Vtop_test_top
	./obj_dir/Vtop_test_top

clean:
	rm -rf obj_dir

.PHONY: all run clean
