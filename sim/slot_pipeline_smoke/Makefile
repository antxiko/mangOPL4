TNCART := /mnt/c/Users/Antxiko/Documents/frutOPL4/external/tnCartWonder/rtl/src
LOCAL  := /mnt/c/Users/Antxiko/Documents/frutOPL4/sim/slot_pipeline_smoke

PKG_SRC := \
	$(TNCART)/wave/ymf278_pkg.sv

RTL_SRC := \
	$(TNCART)/wave/ymf278_slot_state.sv \
	$(TNCART)/wave/ymf278_exp_lut.sv \
	$(TNCART)/wave/ymf278_eg_lut.sv \
	$(TNCART)/wave/ymf278_eg.sv \
	$(TNCART)/wave/ymf278_slot_pipeline.sv \
	$(LOCAL)/pipeline_smoke_top.sv

VFLAGS := --cc --exe --build -j 0 \
	-Wno-WIDTHEXPAND -Wno-WIDTHTRUNC -Wno-ASCRANGE \
	-Wno-MULTIDRIVEN -Wno-LATCH -Wno-UNOPTFLAT \
	-Wno-PINMISSING -Wno-UNUSEDSIGNAL -Wno-UNUSEDPARAM \
	-Wno-CASEINCOMPLETE -Wno-WIDTHCONCAT -Wno-BLKLOOPINIT \
	--top-module pipeline_smoke_top \
	-CFLAGS "-O2"

all: obj_dir/Vpipeline_smoke_top

obj_dir/Vpipeline_smoke_top: tb_pipeline.cpp $(PKG_SRC) $(RTL_SRC)
	verilator $(VFLAGS) $(PKG_SRC) $(RTL_SRC) tb_pipeline.cpp

run: obj_dir/Vpipeline_smoke_top
	./obj_dir/Vpipeline_smoke_top

clean:
	rm -rf obj_dir

.PHONY: all run clean
