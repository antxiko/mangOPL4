TNCART := /mnt/c/Users/Antxiko/Documents/frutOPL4/external/tnCartWonder/rtl/src
LOCAL  := /mnt/c/Users/Antxiko/Documents/frutOPL4/sim/wave_arbiter_smoke

PKG_SRC := \
	$(TNCART)/wave/ymf278_pkg.sv

# Nota: ram.sv contiene tanto el package RAM como la interface RAM_IF
# y los módulos EXPANSION_RAM/BYPASS_RAM. Lo necesitamos para el RAM_IF.
RTL_SRC := \
	$(TNCART)/peripheral/ram/ram.sv \
	$(TNCART)/wave/ymf278_regfile.sv \
	$(TNCART)/wave/ymf278_mempointer.sv \
	$(TNCART)/wave/wave_arbiter.sv \
	$(LOCAL)/sdram_stub.sv \
	$(LOCAL)/arbiter_smoke_top.sv

VFLAGS := --cc --exe --build -j 0 \
	-Wno-WIDTHEXPAND -Wno-WIDTHTRUNC -Wno-ASCRANGE \
	-Wno-MULTIDRIVEN -Wno-LATCH -Wno-UNOPTFLAT \
	-Wno-PINMISSING -Wno-UNUSEDSIGNAL -Wno-UNUSEDPARAM \
	-Wno-CASEINCOMPLETE \
	--top-module arbiter_smoke_top \
	-CFLAGS "-O2"

all: obj_dir/Varbiter_smoke_top

obj_dir/Varbiter_smoke_top: tb_arbiter.cpp $(PKG_SRC) $(RTL_SRC)
	verilator $(VFLAGS) $(PKG_SRC) $(RTL_SRC) tb_arbiter.cpp

run: obj_dir/Varbiter_smoke_top
	./obj_dir/Varbiter_smoke_top

clean:
	rm -rf obj_dir

.PHONY: all run clean
