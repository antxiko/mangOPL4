# Plan Sub-Fase 2c — 24 canales time-shared + envelope DADSR

> Generado 2026-05-07 al cierre de Fase 2b. Documento de referencia para implementación.

## Contexto investigado y decisiones de diseño

### SDRAM y RAM_IF — por qué hace falta arbiter real

Tras leer `sdram.sv` y `ram.sv`:

- **`SDRAM` controller (LEVEL_TRIG=1)** dispara `begin_rd/begin_wr/begin_rfsh` en CADA ciclo en que `OE_n=0/WE_n=0/RFSH_n=0`. Asserta `Ram.ACK_n=0` en `STATE_ACTIVE_ACK` y lo deasserta en `STATE_INACTIVE_ACK` (~7 ciclos).
- **`EXPANSION_RAM`** (`ram.sv` líneas 100-138) NO es un arbiter: combina los hosts con OR sobre `ADDR/DIN/DIN_SIZE` y AND sobre `OE_n/WE_n/RFSH_n`. Si dos hosts assertan al mismo tiempo, `ADDR` resulta es el OR de ambas (basura). El `DOUT/ACK_n/TIMING` se hace fan-out a TODOS los Secondary.
- **Consecuencia**: con accesos esporádicos (memory port a tasa I/O Z80, ~22 µs entre OUTs) la "arbitración" funciona porque casi nunca hay solape. A sample rate (44 kHz × 24 canales = 1.05 M fetches/s, uno cada ~95 CLK = ~890 ns) el solape con `cartridge_ram` (host MEM_MAPPER, accedido cada Z80 M-cycle) será permanente → corruption garantizada.
- **`Ram.TIMING=1`** señaliza solo "SDRAM en STATE_IDLE". Es necesario pero **no suficiente**: si dos hosts ven `TIMING=1` simultáneamente, ambos disparan request en el mismo ciclo y el OR-collapse mezcla sus addresses.

### Decisión 2c.1 — arbiter con priority encoder fijo

**Política elegida**: **priority encoder fijo** con orden `BOOTLOADER > MEGAROM > NEXTOR > FM > RAM > WAVE`. Justificación:

- **Bootloader** corre solo durante boot (carga YRW801) y necesita throughput máximo; bloquea Z80 con WAIT_n. Después queda idle.
- **MEGAROM/NEXTOR/RAM** son hosts del MSX memory mapper: cada miss penaliza Z80 ciclos reales. Tienen que tener prioridad sobre Wave.
- **WAVE (24 canales × 44 kHz)** tiene presupuesto holgadísimo: necesita 1 fetch cada ~890 ns de promedio, y la SDRAM hace transacción en ~7 CLK = ~65 ns. Aunque se desplace ocasionalmente por un acceso del mapper, Wave tiene un FIFO/cache interno que absorbe latencia.
- **Round-robin** añadiría complejidad y CLS sin beneficio: el cuello no es Wave, es el mapper. Round-robin TAMPOCO garantiza determinismo en el peor caso.

**Nuevo módulo `SDRAM_ARBITER`** (sustituye `EXPANSION_RAM` para los 6 hosts a SDRAM). Diseño:

- N RAM_IF.DEVICE input (Secondary[0..5]) + 1 RAM_IF.HOST output (Primary) que va al `SDRAM` controller.
- `grant[5:0]` one-hot indicando qué host está activo.
- Priority encoder combinacional: `grant = encoder({Secondary[0..5].OE_n, Secondary[0..5].WE_n, Secondary[0..5].RFSH_n})`.
- Latcheado en `current_grant[5:0]` cuando `STATE_IDLE && nuevo request`. Mantiene grant hasta que el SDRAM vuelve a `STATE_IDLE`.
- Mux de salida hacia Primary: `Primary.ADDR = Secondary[current_grant].ADDR` (mux real, no OR). Igual para DIN, DIN_SIZE, OE_n, WE_n, RFSH_n.
- Fan-out hacia Secondary: solo el host con grant ve `ACK_n` real y `TIMING=1`. Los demás ven `ACK_n=1`, `TIMING=0`, `DOUT=0`.

### Decisión 2c.2 — interfaz wave/SDRAM: **un solo RAM_IF compartido**

**Decisión**: 1 RAM_IF.HOST único entre `ymf278_top` y SDRAM. El time-sharing entre 24 canales (memory port + playback de cada slot) se hace **internamente** en el wave block.

Razones:
- 24 RAM_IF == 24 hosts en el SDRAM_ARBITER. Eso **hincha el OR-collapse** (24 muxes de 24 bits) y multiplica por 24 la lógica del arbiter. Probablemente +10% CLS solo en el arbiter.
- El priority encoder dentro del wave block es trivial: round-robin entre 24 slots a sample rate ya es time-sharing natural — un slot por sample-tick × 24 ticks por sample period (44 kHz/24 = 1.84 kHz por slot, sobra latencia).
- El memory port (Z80 OUT/IN) tiene PRIORIDAD ABSOLUTA dentro del wave block sobre los fetches de playback. El Z80 espera 22 µs entre OUTs; los slots toleran perfectamente que se les desplace 1 µs.

### Decisión 2c.3 — state file de 24 canales: **BSRAM dual-port**

Cuentas:
- Por slot necesitamos guardar: `phase_acc[31:0]` + `env_state[2:0]` + `env_level[8:0]` + `key_on_prev[0]` + `start_addr[23:0]` + `end_addr[23:0]` + `loop_addr[23:0]` ≈ ~120 bits/slot × 24 = ~2880 bits.
- **Shift register chain (FF)**: ~3000 FFs solo para state, además de los muxes para escribir/leer en fase. CLS muy alto.
- **BSRAM dual-port** (mismo patrón que `mem_multi_bank.sv` del core gtaylormb): 1 bloque dual-port BSRAM (Tang Nano 20K tiene 46 BSRAMs de 18 Kbit cada uno = 828 Kbit). Cabe sobradamente.
- Ganador claro: **BSRAM dual-port**. Patrón exacto del `state_mem`/`env_int_mem` de `envelope_generator.sv` (líneas 186-202 y 227-243 del core gtaylormb).

Estructura propuesta: 1 BSRAM por "campo" lógico (phase_acc, env_state, env_level, etc.), cada uno indexado por slot_num (5 bits, depth 24). Total ~5-6 BSRAMs para todo el state.

### Decisión sobre el cache local actual

El cache de 256 bytes en `ymf278_mempointer.sv` **se elimina por completo** en 2c.2 (cuando el playback empiece a leer SDRAM real vía arbiter). En 2c.1 sigue ahí para que el commit no rompa MoonBlaster Wave.

---

## Sub-paso 2c.1 — SDRAM arbiter real con priority

**Goal**: introducir `SDRAM_ARBITER` que sustituye al `EXPANSION_RAM` actual entre los hosts y la SDRAM, sin cambiar ningún host. Audio sigue saliendo del cache local de 256 bytes.

### Archivos a CREAR

- `external/tnCartWonder/rtl/src/peripheral/ram/sdram_arbiter.sv` — arbiter priority-encoded. Mismo modport que `EXPANSION_RAM` (DEVICE para N hosts, HOST para SDRAM). Internamente:
  - `grant[5:0]` one-hot indicando qué host está activo.
  - Priority encoder combinacional.
  - Latcheado en `current_grant` cuando `STATE_IDLE && nuevo request`. Mantiene grant hasta que el SDRAM vuelve a `STATE_IDLE`.
  - Mux de salida hacia Primary: `Primary.ADDR = Secondary[current_grant].ADDR` (mux real, no OR).
  - Fan-out hacia Secondary: solo el host con grant ve `ACK_n` real y `TIMING=1`.

### Archivos a MODIFICAR

- `external/tnCartWonder/rtl/src/main.sv` — sustituir `EXPANSION_RAM #(.COUNT(RAM_COUNT))` por `SDRAM_ARBITER #(.COUNT(RAM_COUNT))`. Mismas conexiones.

### Riesgos técnicos

- **Bug histórico #2 (re-asertar OE_n/WE_n)**: el FSM de `ymf278_mempointer` ya solo asserta 1 ciclo. Hay que garantizar que el arbiter latchee el grant en flanco DEL MISMO ciclo en que `Secondary[i].OE_n=0`. **Solución**: usar `current_grant` solo durante la transacción y dejar el mux puramente combinacional con priority encoder en idle.
- **Bug histórico #1 (output dangling)**: NO añadir signals al top de `cartridge_opl3.sv`.
- **Boot regression**: el bootloader carga YRW801 a SDRAM. Si la priority del bootloader funciona mal, YRW801 queda corrupto. Test smoke de YRW801 (lectura del primer byte vía memory port) confirma carga correcta.

### Validación en MSX real

1. Boot Nextor + MSX-DOS 2 + `dir` funcional.
2. MoonBlaster FM canción completa (regresión OPL3).
3. VGMPlay OPL3 sin cuelgues.
4. MoonBlaster Wave detect 2 MB.
5. `wavedump.asc` muestra header YRW801 (smoke de bootloader).
6. `waveplay.asc` "ronquido" (cache local sigue funcionando).
7. `wavemem.asc` regresión memory port.

### Recursos esperados

- Logic 47% → ~49%
- CLS 61% → ~63%
- BSRAM/DSP sin cambios

### Esfuerzo

**Medio**. ~1-2 días.

---

## Sub-paso 2c.2 — Phase generator + interp lineal + 1 canal SDRAM real

**Goal**: phase accumulator 32-bit (16.16) + interpolación lineal + slot 0 leyendo de SDRAM real. Audio limpio waveplay sin "ronquido".

### Archivos a CREAR

- `external/tnCartWonder/rtl/src/wave/ymf278_phase.sv` — phase generator. `phase_inc` calculado de `fnum/octave`. Verificar fórmula contra `YMF278.cc` función `Slot::step()`.
- `external/tnCartWonder/rtl/src/wave/ymf278_interp.sv` — interp lineal. 1 DSP signed 17×17.
- `external/tnCartWonder/rtl/src/wave/ymf278_fetch1.sv` — fetcher SDRAM dedicado al slot 0. Mini-cache 2 samples. Solo 8-bit unsigned y 16-bit signed.

### Archivos a MODIFICAR

- `ymf278_pkg.sv` — `SAMPLE_RATE_HZ`, `CLK_OPL3_HZ`, `OPL3_PER_SAMPLE`, phase widths.
- `ymf278_mempointer.sv` — **eliminar cache local** (sample_cache, sample_index, tick_counter, etc). Vuelve a ser solo memory port. RAM_IF interno.
- `ymf278_top.sv` — instanciar phase + fetch1 + interp. Mini-arbiter interno mempointer>fetch1.

### Riesgos técnicos

- DSP signed 17×17 — typical bug `signed`/`unsigned`.
- Phase wrapping (loop_addr) — hardcoded simple en 2c.2.
- Overlap mempointer/fetch1 (~50 ns extra latency, dentro de margen).

### Validación

1. Regresión todos tests 2c.1.
2. MoonBlaster Wave con 1 instrumento simple → A4 limpio (440 Hz).
3. Pitch correcto (C4→C5 duplica freq).
4. Escala chromática audible.
5. Memory port write/read-verify sin regresión.

### Recursos esperados

- Logic 49% → ~52%
- CLS 63% → ~66%
- BSRAM ~32% (mini-cache)
- DSP 13% → ~16% (+1 interp)

### Esfuerzo

**Grande**. ~3-4 días + Verilator bench de pitch.

---

## Sub-paso 2c.3 — Pipeline time-shared 24 canales (sin envelope)

**Goal**: 24 canales independientes pueden disparar samples. Sin envelope (volumen fijo).

### Archivos a CREAR

- `ymf278_slot_state.sv` — BSRAMs dual-port (phase_acc, key_on_prev, playing) × 24 entradas. Patrón directo de `mem_multi_bank.sv`.
- `ymf278_slot_pipeline.sv` — pipeline 8 stages × 24 slots = ~31 ciclos/slot. Stages: read state → read regs → calc phase → fetch → wait ACK → interp → vol → mix-accumulate → write state.
- `ymf278_fetch24.sv` — fetcher serializado. Mini-cache compartido (2 samples × 24 slots = 48 entradas BSRAM dual-port).

### Archivos a MODIFICAR

- `ymf278_pkg.sv` — `SLOT_COUNT=24`, `CYCLES_PER_SLOT=31`, layout slot regs (start, end, loop, fnum, oct, format, vol).
- `ymf278_top.sv` — instanciar nuevos módulos.
- `ymf278_regfile.sv` — port indexado por slot_num (latcheado en stage 1 para evitar fanout 24:1 en cada stage).

### Riesgos técnicos

- **CLS explosion del regfile**: muxes 24:1 × N bytes. Mitigación: latch interno en stage 1.
- Latencia fetcher con 24 slots: peor caso 240 ciclos / 760 = 31% del presupuesto.
- Mix overflow: 24 × 16-bit = 21-bit signed.

### Validación

1. Regresión.
2. MoonBlaster Wave con set completo (8-16 slots simultáneos): canción reconocible.
3. Stress 24 slots simultáneos: sin dropouts.
4. Memory port concurrente con playback: writes no corrompen samples.

### Recursos esperados

- Logic 52% → ~60%
- CLS 66% → ~75%
- BSRAM 32% → ~42%
- DSP 16% → ~25%

### Esfuerzo

**Grande**. ~4-5 días + Verilator exhaustive.

---

## Sub-paso 2c.4 — Envelope DADSR + tablas EG

**Goal**: envelope generator 5 fases (Delay, Attack, Decay, Sustain, Release) por slot. Click-free.

### Archivos a CREAR

- `ymf278_eg_lut.sv` — tabla rates [0..63]. Patrón referencia: `calc_envelope_shift.sv` del core gtaylormb.
- `ymf278_eg.sv` — envelope generator time-shared 24 slots. Estado: `eg_state[2:0]`, `eg_level[8:0]`. Lectura/escritura BSRAM en stages.
- `ymf278_exp_lut.sv` — exp LUT para conversión log→linear de la atenuación (o reusar `opl3_exp_lut` si licencia lo permite).

### Archivos a MODIFICAR

- `ymf278_slot_pipeline.sv` — insertar stages EG entre interp y mix-accumulate.
- `ymf278_pkg.sv` — layout regs EG por slot (AR, D1R, DL, D2R, RR, KSR, TL, DAMP, LD).
- `ymf278_top.sv` — instanciar `ymf278_eg`. Conectar key_on per-slot.

### Riesgos técnicos

- **EG sticky** (bug histórico Fase 1): inicializar a RELEASE.
- AR=15 attack instantáneo: replicar fielmente.
- DSP saturation atenuación: posible reuso del DSP del interp.

### Validación

1. Regresión.
2. Notas largas (violín, flauta) sostienen sin recortarse.
3. Decay/release audible (snare percusivo).
4. Side-by-side con MoonSound real (si disponible).
5. MoonBlaster Wave demo 1 minuto sin glitches.

### Recursos esperados

- Logic 60% → ~65%
- CLS 75% → ~82%
- BSRAM 42% → ~48%
- DSP 25% → ~28%

### Esfuerzo

**Grande**. ~4-5 días + bench Verilator con vectores capturados de openMSX.

---

## Sub-paso 2c.5 — Mixer L/R 22-bit + integración

**Goal**: mixer estéreo 22-bit, integrado con cartridge_opl3 (FM+Wave+gain). MoonBlaster Wave end-to-end con dinámica correcta.

### Archivos a CREAR

- `ymf278_mixer.sv` — acumulador L/R. Sliding sum por slot-tick. Dummy-pan en 2c.5 (mono); pan real va en 2d.

### Archivos a MODIFICAR

- `ymf278_top.sv` — instanciar mixer. Salida `wave_sample_l/r` en lugar de mono.
- `cartridge_opl3.sv` — `mono_q` mezcla `(FM_L + FM_R + Wave_L + Wave_R) >> 2`. Recalibrar `GAIN_BITS` (probablemente bajar a 4).
- `ymf278_pkg.sv` — `MIX_WIDTH=22`.

### Riesgos técnicos

- Saturación: 24 × 16-bit = 21-bit signed. 22-bit con 1 bit headroom.
- GAIN_BITS recalibration: medir con osciloscopio peaks.
- MoonTest debe pasar 0 errores.

### Validación

1. Regresión todos tests previos.
2. MoonTest 0 errores en 2 MB Sample RAM.
3. MoonBlaster Wave demo MWV completo varios minutos.
4. Mezcla FM+Wave (Meridian MWM) sin distorsión.
5. Side-by-side cualitativo con MoonSound real.
6. VGMPlay + MoonBlaster Wave concurrentes sin cuelgue.

### Recursos esperados

- Logic 65% → ~67%
- CLS 82% → ~85% (apretado para 2d/2e)
- BSRAM 48%
- DSP 28%

**Headroom restante para 2d/2e**: 33% logic, **15% CLS**, 52% BSRAM, 72% DSP. Si CLS > 88%, replantear 2d antes de seguir.

### Esfuerzo

**Medio**. ~2-3 días.

---

## Resumen dependencias y orden

```
2c.1 (arbiter) ──► 2c.2 (1 slot SDRAM) ──► 2c.3 (24 slots) ──► 2c.4 (envelope) ──► 2c.5 (mixer)
```

Ningún sub-paso es opcional ni reordenable. El arbiter (2c.1) es prerequisito absoluto.

## Recursos proyectados al cierre de 2c.5

| Métrica | 2b.4 (actual) | 2c.5 (proyectado) | Headroom para 2d/2e |
|---|---|---|---|
| Logic | 47% | ~67% | 33% |
| CLS | 61% | ~85% | **15%** (apretado) |
| BSRAM | 31% | ~48% | 52% |
| DSP | 13% | ~28% | 72% |

## Esfuerzo total estimado

~14-19 días, varias sesiones de trabajo.

## Out of scope (queda para 2d/2e)

- 12-bit packed samples (formato YRW801 mayoritario).
- LFO (vibrato + tremolo).
- Pan estéreo real.
- Atenuadores F8/F9 (FM/Wave volume control software).
- DSP echo/reverb.
- Persistencia Sample RAM tras reset.
