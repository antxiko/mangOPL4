# MangOPL4 — Roadmap y estado del proyecto

Documento canónico del estado actual y lo que queda por hacer. Útil para retomar el proyecto con cabeza fresca o para decidir si pausarlo.

**Última actualización**: 2026-05-25.

---

## Resumen ejecutivo

- **Fase 0** (preparación entorno): cerrada 2026-04-26.
- **Fase 1** (FM/OPL3): **cerrada 2026-05-06**. Bitstream funcional en MSX real. MoonBlaster FM + VGMPlay OPL3 + BASIC OUT sin regresión.
- **Fase 2** (Wave/PCM): **en curso, ~30% hecho**. Slot 0 monofónico audible con TL + EG. Multi-slot, headers YRW801, pan, LFO, atenuadores Wave/FM globales NO empezados.
- **Fase 3** (cartucho propio): no empezada, lejana, no es objetivo inmediato.

**Bitstream HW actual flasheado**: submódulo `8ad9a03`, padre `b60f4da` = "2c.3.R3" (slot 0 con TL + EG en CLK_OPL3).

---

## Arquitectura actual (qué hay funcionando)

```
MSX bus (Z80, 3.58 MHz)
  ↓
WonderTANG 2.0b (level shifters, mux, alimentación)
  ↓
Tang Nano 20K (GW2AR-18 FPGA)
  ├── OPL3 FM (gtaylormb/opl3_fpga, en CLK_OPL3 33.5625 MHz)
  │     Puertos C4-C7, 18 canales 2-op / 6×4-op, IRQ funcional
  ├── Wave block (Fase 2 en curso)
  │     Puertos 7E/7F, slot 0 con phase + fetch1 + interp + TL + EG
  ├── tnCartWonder stack (sin modificar)
  │     Nextor 2.1, megarom SCC+, memory mapper, V9990 emulado
  └── SDRAM 8MB
        ├── 0x000000-0x0FFFFF  Memory mapper (1 MB)
        ├── 0x100000-0x2FFFFF  YRW801 cargado al boot (2 MB)
        ├── 0x300000-0x47FFFF  Sample RAM main (1.5 MB)
        ├── 0x480000-0x6FFFFF  Megarom (2.5 MB)
        ├── 0x700000-0x71FFFF  Nextor (128 KB)
        ├── 0x720000-0x723FFF  FM-BIOS (16 KB)
        └── 0x780000-0x7FFFFF  Sample RAM ext (512 KB)
```

---

## Fase 0 — Preparación del entorno ✓

Entorno: Gowin Educative IDE V1.9.11.03 + openFPGALoader v1.0.0 + Verilator 5.020 (WSL2) + VS Code. Tang Nano programado, Nextor arranca desde SD.

---

## Fase 1 — OPL3 FM ✓ CERRADA 2026-05-06

Slot 0 OPL3 funcional en MSX real. Repositorio público en `github.com/antxiko/mangOPL4` desde el mismo día.

**Validado en HW**:
- MoonBlaster FM
- VGMPlay-MSX (Grauw) — detecta MoonSound + reproduce VGMs OPL3
- BASIC OUT directo a C5/C7
- Reproducciones múltiples sin cuelgue
- Recupera tras reset MSX

**Bugs resueltos y documentados** (ver `docs/PHASE_2C_PLAN.md` y commits del fork `antxiko/opl3_fpga`):
- Gowin trunca `real * real` (warn EX3791) → fix con `localparam int TIMER1_TICK_COUNT=2685`
- `irq_rst` sticky en gtaylormb (debe ser strobe, no FF persistente)
- `assert property` no digerible por Gowin → envuelto en `// synthesis translate_off/on`
- IRQ wiring: pulso 5 µs + gap 50 µs + repeat hasta ack o watchdog (32 pulsos)
- Backdoor `force_clear_flags` para romper deadlock tras exit "natural" de VGMPlay

**Pendiente diferido** (no bloquea):
- Conflicto puertos con MSX-AUDIO C0-C3 (ningún MSX del usuario tiene MSX-AUDIO real)
- Atenuadores F8/F9 globales FM/Wave del MoonSound real

---

## Fase 2 — Wave/PCM YMF278B (EN CURSO)

Objetivo: 24 canales PCM con ROM YRW801, sample RAM, envelope DADSR, pan L/R, LFO, mixer FM+Wave.

### Sub-fases CERRADAS

| Sub-paso | Submódulo | Resumen |
|---|---|---|
| **2b.1-2b.4** | hasta `617e182` | RAM_IF foundation, memory port (regs 02-06), cache local 256B, bootloader carga YRW801 a SDRAM 0x100000 |
| **2c.1** | `ecad5e2` | sdram_top_arbiter dual-bus (priority A>B) — resuelve bus contention cartridge_ram vs wave block |
| **2c.2.a-f** | hasta `93b57aa` | phase generator bit-exact YMF278B, fetch1 standalone, interp lineal, **playback YRW801 audible 1 slot** |
| **2c.3.aa** | `ead81bc` | refactor slot 0 a módulo aislado `ymf278_slot.sv` |
| **2c.3.ab** | `a29c5d0` | `ymf278_interp_reg` wrapper (FF cerca DSP, fixea timing lottery 14 ns → 9 ns) |
| **2c.3.R2** | `0815ce5` | TL atenuación slot 0 en CLK_OPL3 + 2-FF sync per bit |
| **2c.3.R3** | `8ad9a03` | EG envelope generator slot 0 (AR/D1R/DL/D2R/RR, FSM ATT/DEC1/DEC2/REL) |

**Validado HW R2+R3** (2026-05-22): 4 cold-boots consistentes, `pha2c3cc.asc` con barridos AR=15/8/4/2/0 todos discriminantes audibles, reset MSX recupera audio, sin regresión FM.

### Lo que QUEDA por hacer en Fase 2

| Sub-paso | Descripción | Dificultad estimada |
|---|---|---|
| **R4** | Multi-slot (8 → 24 slots): replicar pipeline EG+TL+mul para slots 1-23, mixer accumulator | ALTA — recursos FPGA + arbiter + timing |
| **R5** | Header lookup YRW801: per-slot start/loop/end leídos de headers (12 bytes/sample × 384 samples) al boot | MEDIA — requiere boot-time table fetch + arbitración |
| **R6** | Pan L/R: 2 multipliers per slot (volPan) | BAJA |
| **R7** | LFO: vibrato + tremolo per slot | MEDIA |
| **R8** | 12-bit packed samples (formato YRW801 real, no solo 8-bit unsigned) | MEDIA |
| **R9** | Atenuadores F8/F9 globales FM/Wave del MoonSound | BAJA |
| **R10** | MoonBlaster Wave end-to-end test | TEST — depende R4-R9 |

### Intentos abandonados (NO repetir sin re-diseño)

- **R1 — Refactor serial 1-slot-per-cycle** (inspirado openMSX): 7 iteraciones v1-v7, slot_serial nunca convergió. Pulsos OE_n se perdían en arbiter. Causa raíz nunca entendida. Ver `memory/project_r1_serial_failed.md`. Plan B (BSRAM cache) o reusar fetch1 son alternativas.
- **Pipeline 8-stage time-shared** (sub-pasos 2c.3.a-j): critical path imposible en Stage 6 (EG+LUT+DSP combinacional, WNS -19.9 ns). Reemplazado por arquitectura paralela (R2/R3) con state local en FFs.
- **R5 v0+v1+v2 fix fetch1 pulse loss** (2026-05-24): "fix" PASS en sim/wave_slot_arbiter pero ROMPIÓ HW (OE_n=0 sostenido bloqueó bus → cuelgue boot determinista). Ver `memory/feedback_fix_no_validado_sim_simple.md`. Si se revisita: timeout+retry, NO OE_n sostenido; modelar boot sequence en sim ANTES de flashear.

---

## Fase 3 — Cartucho propio (LEJANA, fuera del alcance inmediato)

Cuando llegue: documento `docs/CUSTOM_CARTRIDGE.md` (no existe todavía). Decisiones a tomar:
- Multiplexado A0-A7/D0-D7 vs pines pelados.
- 74LVC245 vs level shifters bidireccionales.
- MAX98357A I2S Class-D vs DAC 1-bit + filtro RC.
- PSRAM/HyperRAM integrada vs SDRAM discreta.
- Encapsulado QFN 0.5mm pitch requiere reflow oven o hot air.

---

## Infraestructura de simulación disponible

Verilator funcional desde WSL2. Reusable para futuras iteraciones.

| Sim | Path | Qué cubre |
|---|---|---|
| `sdram_arbiter_smoke` | `sim/sdram_arbiter_smoke/` | sdram_top_arbiter dual-bus (3 testcases, validó 2c.1) |
| `wave_arbiter_smoke` | `sim/wave_arbiter_smoke/` | wave_arbiter v5 (mempointer + fetch1 stub, validó Gowin CE-style FF workaround) |
| `wave_slot_full` | `sim/wave_slot_full/` | slot completo (ymf278_slot + fetch1 + SDRAM stub) sin arbitración. Validó R5 v0+v1 cache logic |
| `wave_slot_arbiter` | `sim/wave_slot_arbiter/` | slot + ambos arbiters + Z80 stub + mempointer stub. 4 scenarios (control / Z80 90% duty / mempointer pulse / caos). Reproduce starvation HW |

**Limitaciones detectadas** (2026-05-24): sim no modela secuencia de boot (bootloader cargando YRW801, regfile power-on state). Un fix que pasa sim puede romper HW boot. Lección dura.

**Comando típico**:
```bash
wsl -e bash -c "cd /mnt/c/Users/Antxiko/Documents/frutOPL4/sim/wave_slot_arbiter && make && ./obj_dir/Vslot_arbiter_top"
```

---

## Bugs conocidos / deudas técnicas

### Arquitectónicos

1. **`wave_arbiter` OR-collapse de ADDR cuando A+B asertan concurrentes**. Reproducido en sim wave_slot_arbiter Scenario D. No crítico ahora (mempointer raramente coincide con fetch1 en slot 0 único), pero bloqueante para R4 (multi-slot). Solución: gate B cuando A active, como hace `sdram_top_arbiter`.

2. **`fetch1` starvation con Z80 alta carga**: identificado en sim, NO arreglado en HW. Pulso OE_n=0 1-cycle puede perderse si arbiter mutea wave block. En HW funciona "por suerte" porque Z80 duty real no es 90%. Cualquier fix futuro debe modelar boot sequence + tener timeout+retry, NO sostener OE_n=0.

3. **`cartridge_ram` upstream NO chequea ACK_n**. Resuelto a nivel sistema con `sdram_top_arbiter` (2c.1) pero el patrón sigue ahí. Si se moderniza el upstream, validar regresión completa.

### Gowin synth

4. **CE-style FF con modport read** (`else if (Primary.TIMING) ff <= ...`) se sintetiza mal. Workaround: D input como mux completo. Documentado en `wave_arbiter.sv` v5.

5. **`real * real` truncado** (warn EX3791): no usar tipos `real` en synth. Precomputar como `localparam int`.

6. **OPL3 control_operators paths timing** (~21 worst paths, WNS variable): pre-existentes desde Fase 1, no afectan audio FM. NO tocar — son sweepeo de clock-enables `n175_41`/`n633_41`/`n129_5`.

7. **OPL3 latches inferred** en `control_operators.sv:294,466` y `envelope_generator.sv:218`: warnings de Fase 1, no afectan funcionalidad.

### Audio observaciones

8. **"PCM un poco bajito"** reportado tras R2, nunca A/B test discriminante hecho. Posible: 2-FF sync per bit del sample data 16-bit (multi-bit CDC) causa partial metastability ocasional. Si reaparece: considerar 3-FF sync o handshake.

9. **Default detección MoonSound** en port 7F devuelve 0x20 (stub). Real YMF278 devuelve chip ID en algún reg específico. VGMPlay actual detecta OK, pero MoonSound test exhaustivo podría fallar.

---

## Cómo retomar el proyecto

### Flujo de trabajo

```
Cambio Verilog (slot.sv / fetch1.sv / etc.)
  ↓
Sim Verilator (~5-10 seg compilar + run)
  ↓ PASS
[Considerar qué casos HW NO modela el sim]
  ↓
Synth Gowin (~5 min): tools/flash/synth.tcl
  ↓
Flash: bash tools/flash/flash_all.sh
  (USB-C del Tang Nano DIRECTO al PC, no vía MSX)
  (SIEMPRE --external-flash via flash_all.sh, nunca openFPGALoader -f directo)
  ↓
Test MSX real con .asc en SD
  (línea 1 SIEMPRE "1 SCREEN 0:WIDTH 80")
  (nombre fichero 8.3, FAT12 no VFAT)
  (unix2dos para CRLF tras escribir)
```

### Rollback safe

| Bitstream | Submódulo | Padre | Estado |
|---|---|---|---|
| `93b57aa` | 2c.2.f | `7814679` | 1 voz audible, sin EG/TL. Baseline funcional. |
| `2b0cc4f` | 2c.3.f | `ab55fe0` | Pipeline 24 slots time-shared (ABANDONADO por timing) |
| `a29c5d0` | 2c.3.ab | `0c37856` | Refactor + timing fix Wave |
| **`8ad9a03`** | **2c.3.R3** | **`b60f4da`** | **ACTUAL flasheado: slot 0 + TL + EG en CLK_OPL3** |

Para rollback: `git -C external/tnCartWonder checkout <commit> -- rtl/impl/pnr/tnCart_board_wt200b.fs && bash tools/flash/flash_all.sh`.

### Próxima sesión sugerida (cuando vuelva el ánimo)

1. **R4 micro-step**: hacer slot 1 (duplicar slot 0 en paralelo) sin tocar fetch1/SDRAM. Mixer 2-slot accumulator. Valida que el patrón R2/R3 escala.
2. **Alternativa**: dejar Fase 2 en R3 (1 slot funcional) y considerar si el proyecto está en buen punto para pausar. R3 es ya un cartucho MSX con OPL3 funcional + 1 canal Wave de demostración.

---

## Identidad del proyecto

- **Nombre**: MangOPL4 (mango + OPL4). En commits/paths: `mangopl4` minúsculas.
- **Repo padre**: `github.com/antxiko/mangOPL4` (público desde 2026-05-06).
- **Repo submódulo**: `github.com/antxiko/tnCartWonder` (fork de herraa1/tnCartWonder).
- **Local**: `C:\Users\Antxiko\Documents\frutOPL4\`.
- **YRW801 ROM**: copyright Yamaha. Distribución pública requiere que el usuario aporte su propio dump. NO incluido en repo.
