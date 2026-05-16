TNCART := /mnt/c/Users/Antxiko/Documents/frutOPL4/external/tnCartWonder/rtl/src
LOCAL  := /mnt/c/Users/Antxiko/Documents/frutOPL4/sim/wave_contention_smoke

PKG_SRC := \
	$(TNCART)/wave/ymf278_pkg.sv

RTL_SRC := \
	$(TNCART)/peripheral/ram/ram.sv \
	$(TNCART)/wave/ymf278_regfile.sv \
	$(TNCART)/wave/ymf278_phase.sv \
	$(TNCART)/wave/ymf278_mempointer.sv \
	$(TNCART)/wave/ymf278_interp.sv \
	$(TNCART)/wave/ymf278_fetch1.sv \
	$(TNCART)/wave/wave_arbiter.sv \
	$(TNCART)/wave/ymf278_top.sv \
	$(LOCAL)/sdram_stub.sv \
	$(LOCAL)/fake_cartridge_ram.sv \
	$(LOCAL)/contention_top.sv

VFLAGS := --cc --exe --build -j 0 \
	-Wno-WIDTHEXPAND -Wno-WIDTHTRUNC -Wno-ASCRANGE \
	-Wno-MULTIDRIVEN -Wno-LATCH -Wno-UNOPTFLAT \
	-Wno-PINMISSING -Wno-UNUSEDSIGNAL -Wno-UNUSEDPARAM \
	-Wno-CASEINCOMPLETE -Wno-WIDTHCONCAT -Wno-BLKLOOPINIT \
	--top-module contention_top \
	-CFLAGS "-O2"

all: obj_dir/Vcontention_top

obj_dir/Vcontention_top: tb_contention.cpp $(PKG_SRC) $(RTL_SRC)
	verilator $(VFLAGS) $(PKG_SRC) $(RTL_SRC) tb_contention.cpp

run: obj_dir/Vcontention_top
	./obj_dir/Vcontention_top

clean:
	rm -rf obj_dir

.PHONY: all run clean
