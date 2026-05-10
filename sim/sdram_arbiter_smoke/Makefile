TNCART := /mnt/c/Users/Antxiko/Documents/frutOPL4/external/tnCartWonder/rtl/src
LOCAL  := /mnt/c/Users/Antxiko/Documents/frutOPL4/sim/sdram_arbiter_smoke

RTL_SRC := \
	$(TNCART)/peripheral/ram/ram.sv \
	$(TNCART)/peripheral/ram/sdram_top_arbiter.sv \
	$(LOCAL)/sdram_stub.sv \
	$(LOCAL)/mock_a_cart.sv \
	$(LOCAL)/mock_b_wave.sv \
	$(LOCAL)/arbiter_smoke_top.sv

VFLAGS := --cc --exe --build -j 0 \
	-Wno-WIDTHEXPAND -Wno-WIDTHTRUNC -Wno-ASCRANGE \
	-Wno-MULTIDRIVEN -Wno-LATCH -Wno-UNOPTFLAT \
	-Wno-PINMISSING -Wno-UNUSEDSIGNAL -Wno-UNUSEDPARAM \
	-Wno-CASEINCOMPLETE -Wno-WIDTHCONCAT -Wno-BLKLOOPINIT \
	--top-module arbiter_smoke_top \
	-CFLAGS "-O2"

all: obj_dir/Varbiter_smoke_top

obj_dir/Varbiter_smoke_top: tb_arbiter.cpp $(RTL_SRC)
	verilator $(VFLAGS) $(RTL_SRC) tb_arbiter.cpp

run: obj_dir/Varbiter_smoke_top
	./obj_dir/Varbiter_smoke_top

clean:
	rm -rf obj_dir

.PHONY: all run clean
