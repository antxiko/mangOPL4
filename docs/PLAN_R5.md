# Plan R5 — Ruta segura para escuchar samples reales del YRW801

Documento de planificación detallada. Optimizado para **fiabilidad sobre velocidad**: cada paso es pequeño, validable, y reversible. Asume que retomamos el proyecto desde el estado `b60f4da` (2c.3.R3, slot 0 con TL + EG audible pero leyendo headers, no samples).

**Por qué este plan existe**: el 2026-05-24 se intentó R5 a saco y se rompió HW (cuelgue boot). Lección: cada flash sin disciplina cuesta horas de rollback. Este plan impone disciplina.

---

## Pre-condiciones

- Bitstream actual flasheado: `b60f4da` (R3) → MSX arranca Nextor + MoonBlaster FM + slot 0 Wave audible con TL+EG sobre headers YRW801.
- Submódulo working tree: limpio (sin cambios uncommitted).
- USB del Tang Nano: conectarlo DIRECTO al PC, no vía MSX, antes de cada flash. Tener el cable a mano.

---

## Fase 0 — Diagnóstico OPL3 ↔ OPL4 (1 sesión, ~3h)

**Objetivo**: confirmar si OPL3 contamina el Wave block via CLK_BASE shared / glitches / placement Gowin.

**Hipótesis a falsar**: los paths OPL3 con timing violations (-15 ns en `control_operators`) meten ruido en CLK_BASE que afecta al Wave block antes del CDC a CLK_OPL3.

### Pasos

**F0.1** — Crear flag de compilación `DISABLE_OPL3`. En `main.sv` (o donde está la instancia `u_opl3`), envolver con `ifdef DISABLE_OPL3` un stub que:
- Mantiene puertos C4-C7 leyendo 0xFF (preserva detección de "no MoonSound" en VGMPlay)
- Mantiene IRQ_n=1 permanente
- Output al mixer = 0 (silencio FM)

No tocar nada del wave block. No tocar nada del bus MSX. Solo desactivar OPL3.

**F0.2** — Sintetizar variante `DISABLE_OPL3`. Esperar liberación de recursos (CLS 55% → ~10%) y desaparición de paths timing-violated. Verificar en `timing_paths` que **0 paths con slack negativo**.

**F0.3** — Flash + cold boot. MSX debe arrancar Nextor (no toca OPL3). MoonBlaster FM NO sonará (esperado). Wave block debe funcionar igual o mejor.

**F0.4** — Tests audibles del Wave (los mismos que ya tenemos: pha2c3cc.asc EG, scripts simples key_on con AR=15). Comparar consistencia entre 5 cold-boots.

**F0.5** — Decisión bifurcación:

| Resultado F0 | Conclusión | Acción |
|---|---|---|
| Wave 100% consistente sin OPL3 | OPL3 contamina | Ir a F0.6 (aislar) |
| Wave igual de inconsistente | Bug propio del Wave | Saltar a Fase 1 sin aislamiento |
| Wave roto/diferente | Cambio rompió Wave | Investigar antes de seguir |

**F0.6** (si aplica) — Aislar OPL3 con barrera de FFs.

Estrategia: cualquier señal que cruza de OPL3 a Wave/mixer pasa por un FF extra en CLK_BASE para "limpiar" glitches. En la práctica:
- Sample output del OPL3 (24-bit signed): añadir FF intermedio entre OPL3 output y mixer input.
- Si el OPL3 escribe a regs compartidos: añadir FF a la salida del decoder.
- Considerar `keep` o `syn_keep` constraint en señales boundary.

**F0.7** — Sintetizar con aislamiento + OPL3 habilitado, validar:
- 0 paths timing-violated nuevos
- MoonBlaster FM sigue funcionando
- Wave consistente entre cold-boots (= comportamiento de F0.4)

**Si F0.7 PASS → OPL3 ya no contamina al Wave. Buena base para R5.**

**Si F0.7 FAIL** → considerar opciones más invasivas (mover Wave block a clock dominio separado, partial reconfiguration, etc.) o aceptar contaminación residual y compensar en R5 con tolerancia.

---

## Fase 1 — Sim mejorado (1 sesión, ~4h)

**Objetivo**: tener un sim Verilator que NO dé falsos positivos. Hasta que el sim modele el boot real, NO se flashea nada del Wave.

### Por qué hace falta esto

El 2026-05-24 el sim `wave_slot_arbiter` PASS en A+B+C, pero el HW se colgó al boot. Causa: sim no modelaba (a) bootloader cargando YRW801 a SDRAM, (b) regfile en estado random al power-on, (c) Z80 ejecutando código real (Nextor + BASIC) con su patrón de bus complejo.

### Pasos

**F1.1** — Crear `sim/wave_full_boot/`. Top que instancia:
- `ymf278_top` real (no solo slot)
- `cartridge_ram` real (= host MSX problemático)
- `sdram_top_arbiter` + `wave_arbiter` reales
- `sdram_stub_8mb`: stub de SDRAM con 8 MB, capaz de cargar contenido desde fichero binario (`yrw801.rom`)
- `z80_nextor_stub`: stub que reproduce el patrón Nextor durante boot (lots of reads a region mapper) + ejecución BASIC (OUT statements espaciados)
- `bootloader_stub`: stub que escribe YRW801 a SDRAM 0x100000 ANTES de marcar reset OFF

**F1.2** — Test 1 (regression): reproducir scenario actual R3. Slot 0 con start_addr=0x100000, key_on, debe leer bytes 0x01..0x10 del YRW801 (= primer header). Si el sim entrega esos bytes, el sim modela bien el path básico.

**F1.3** — Test 2 (boot safety): simular un cold boot completo:
- Reset assertion
- 50000 cycles de bootloader copiando YRW801
- Reset deassertion (regfile aún no escrito, key_on=0 por reset)
- 100000 cycles de Z80 stub corriendo
- Verificar: fetch1 NUNCA arserta OE_n con key_on=0. cartridge_ram NUNCA lee garbage (= save_addr correcto).

Si Test 2 PASS, sim captura boot safety. Si FAIL → el bug que rompió HW ayer también se reproduce en sim, y podemos iterar.

**F1.4** — Test 3 (R5 ready): cargar YRW801 real (.rom file leído por el stub via `$readmemh` o equivalente). Escribir `wave_num=0` al regfile (con timing realista, post-boot). Verificar que fetch1 lee desde start_addr del sample 0 (= primer instrumento del YRW801, addr definida por header).

Si Test 3 PASS → sim modela R5 completo. Si FAIL → identificar bug, NO flashear todavía.

---

## Fase 2 — R5 incremental sobre sim sólido (2-3 sesiones)

Cada sub-paso: cambio mínimo → sim PASS → flash → test HW → commit. NO acumular cambios entre flashes.

### R5.a — BSRAM header table (sin loader todavía)

**Cambio mínimo**: añadir `ymf278_header_table.sv` con BSRAM de 4608 bytes (2 blocks BSRAM). Init con valores hardcoded conocidos (e.g. todos los samples apuntan a 0x100200 para test). 

**Validación sim**: lookup wave_num=0 → output start_addr=0x100200. Lookup wave_num=1 → idem (todos iguales en este sub-paso).

**Validación HW**: bitstream sintetiza, MSX boot OK, sin regresión en R3.

### R5.b — Header lookup en slot.sv

**Cambio mínimo**: add input `wave_num [8:0]` al slot. Slot hace lookup en header_table BSRAM → output start_addr → passes a fetch1.

**Validación sim**: cambiar wave_num input → fetch1 ve start_addr distinto.

**Validación HW**: bitstream OK, hardcoded wave_num=0 desde top.sv → slot lee start_addr=0x100200 (= primer sample real del YRW801 con headers que vamos a poblar después). Audible: **primer sample real escuchado**, aunque sea siempre el mismo.

### R5.c — Reg wave_num en regfile

**Cambio mínimo**: añadir reg 0x08+slot (9-bit wave_num) al regfile. Slot.sv recibe wave_num desde regfile.

**Validación sim**: write reg 0x08 con wave_num=5 → slot lee header del sample 5 → fetch1 addr cambia.

**Validación HW**: script BASIC `OUT &H7E, &H08: OUT &H7F, 5` → suena el sample 5 (todavía todos apuntan a 0x100200 porque no hay loader, pero verifica el path completo).

### R5.d — Cache invalidate en wave_num change

**Cambio mínimo**: fetch1 invalida cache (`last_idx_fetched <= 16'hFFFF`) en flanco subiente de key_on (R5 v1 original) **Y** en cualquier cambio de start_addr_sdram_in (detectar via FF de comparación).

**Validación sim**: escribir wave_num=A, key_on, oír A. key_off. Escribir wave_num=B, key_on, oír B (no quedar pillado con bytes de A).

**Validación HW**: switch wave_num entre key_ons, audible cambio.

### R5.e — Boot-time header loader

**Cambio mínimo y CRÍTICO**: FSM en `ymf278_top` que tras `bus_reset_n=1` (y tras esperar flag `yrw801_loaded` del bootloader principal), lee 4608 bytes desde SDRAM 0x100000 y los escribe a la BSRAM header_table.

**Importante**:
- Esta FSM compite con cartridge_ram + Z80 al boot. **Debe gatear por `Ram.TIMING && bus_merq_n=1`** (patrón conservador, NO el "fix" peligroso de ayer).
- Si tarda mucho (e.g. 5 seg) está bien — solo es al boot.
- Una vez completa, marca un flag `headers_ready` y no vuelve a tocar SDRAM.

**Validación sim**: con sim de Fase 1 corriendo:
- Reset deassert + bootloader carga YRW801 simulado
- Header loader arranca → 4608 escrituras a BSRAM
- `headers_ready=1` después de N cycles
- Lookup wave_num=0 → addr del header sample 0 (= 0x100C00 según layout YRW801 real)

**Validación HW**: cold boot. Esperar ~5 seg post boot Nextor. Script BASIC con varios wave_num → suena un **piano distinto** vs **bajo distinto** vs **guitarra distinto**.

### R5.f — Tests audibles ricos

Script `r5_instrument_test.asc`:
- Loop por wave_num = 0, 32, 64, 96, 128, ... (cada 32 samples)
- key_on, 1 sec, key_off, pausa 0.5 sec
- Cada uno debe sonar diferente

Si todos suenan correctos → R5 cerrado.

---

## Fase 3 — R6 (pan) y R4 (multi-slot)

Pendiente, fuera del scope de este plan. Cuando R5 esté cerrado, re-evaluar.

---

## Disciplina anti-cagada (regla irreducible)

Antes de cada `flash_all.sh`:

1. ✅ **Sim PASS** del sub-paso correspondiente.
2. ✅ **Listar 3 escenarios HW que el sim NO modela**. Si alguno toca boot/power-on, parar y mejorar sim primero.
3. ✅ **Timing report sin paths nuevos negativos** en clock-domain crítico.
4. ✅ **Submódulo limpio** o con cambios revisados (no commits acumulados sin testar).
5. ✅ **USB Tang Nano accessible** (no enchufado al MSX).
6. ✅ **Test HW preparado** (.asc en SD, esperado claro).
7. ✅ **Rollback plan**: saber qué `.fs` flashear si va mal (`git -C external/tnCartWonder checkout 8ad9a03 -- rtl/impl/pnr/tnCart_board_wt200b.fs && bash tools/flash/flash_all.sh`).

Si cualquiera de los 7 NO está → no flashear.

---

## Estimación de tiempo (honesta)

- Fase 0 (diagnóstico OPL3): 3-5h
- Fase 1 (sim mejorado): 4-6h
- Fase 2 (R5 incremental, 6 sub-pasos): 2-3h por sub-paso = 12-18h total
- Buffer para imprevistos: +30%
- **Total**: ~25-35h de trabajo focused, distribuido en 5-7 sesiones de 4h.

Si no hay energía/tiempo para esto, **pausar el proyecto en R3 es completamente válido**: es un cartucho MSX funcional con OPL3 + slot 0 Wave audible. No hace falta llegar a samples reales para que el proyecto esté en buen punto.
