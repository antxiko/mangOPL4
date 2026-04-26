CORE := /mnt/c/wt/mangopl4/external/opl3_fpga/fpga
INC  := +incdir+$(CORE)/modules/

PKG_SRC := $(CORE)/modules/top_level/pkg/opl3_pkg.sv

RTL_SRC := \
	$(CORE)/modules/top_level/src/opl3.sv \
	$(CORE)/modules/channels/src/dac_prep.sv \
	$(CORE)/modules/channels/src/channels.sv \
	$(CORE)/modules/channels/src/control_operators.sv \
	$(CORE)/modules/clks/src/clk_div.sv \
	$(CORE)/modules/clks/src/reset_sync.sv \
	$(CORE)/modules/i2s/src/i2s.sv \
	$(CORE)/modules/operator/src/operator.sv \
	$(CORE)/modules/operator/src/calc_phase_inc.sv \
	$(CORE)/modules/operator/src/calc_rhythm_phase.sv \
	$(CORE)/modules/operator/src/phase_generator.sv \
	$(CORE)/modules/operator/src/vibrato.sv \
	$(CORE)/modules/operator/src/envelope_generator.sv \
	$(CORE)/modules/operator/src/ksl_add_rom.sv \
	$(CORE)/modules/operator/src/calc_envelope_shift.sv \
	$(CORE)/modules/operator/src/tremolo.sv \
	$(CORE)/modules/operator/src/opl3_log_sine_lut.sv \
	$(CORE)/modules/operator/src/opl3_exp_lut.sv \
	$(CORE)/modules/timers/src/timers.sv \
	$(CORE)/modules/timers/src/timer.sv \
	$(CORE)/modules/misc/src/afifo.v \
	$(CORE)/modules/misc/src/edge_detector.sv \
	$(CORE)/modules/misc/src/mem_simple_dual_port_async_read.sv \
	$(CORE)/modules/misc/src/mem_simple_dual_port.sv \
	$(CORE)/modules/misc/src/mem_multi_bank.sv \
	$(CORE)/modules/misc/src/mem_multi_bank_reset.sv \
	$(CORE)/modules/misc/src/pipeline_sr.sv \
	$(CORE)/modules/misc/src/synchronizer.sv \
	$(CORE)/modules/misc/src/leds.sv \
	$(CORE)/modules/host_if/src/host_if.sv \
	$(CORE)/modules/host_if/src/trick_sw_detection.sv

VFLAGS := --cc --exe --build -j 0 \
	-Wno-WIDTHEXPAND -Wno-WIDTHTRUNC -Wno-ASCRANGE \
	-Wno-MULTIDRIVEN -Wno-LATCH -Wno-UNOPTFLAT \
	-Wno-PINMISSING -Wno-UNUSEDSIGNAL -Wno-UNUSEDPARAM \
	--top-module opl3 \
	-CFLAGS "-O2"

all: obj_dir/Vopl3

obj_dir/Vopl3: tb_opl3.cpp $(PKG_SRC) $(RTL_SRC)
	verilator $(VFLAGS) $(INC) $(PKG_SRC) $(RTL_SRC) tb_opl3.cpp

run: obj_dir/Vopl3
	./obj_dir/Vopl3 out.wav 24000

clean:
	rm -rf obj_dir out.wav

.PHONY: all run clean
