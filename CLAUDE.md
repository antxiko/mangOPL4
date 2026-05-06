# Proyecto: OPL3/OPL4 FPGA sobre WonderTANG para MSX

Documento de memoria y diseño para continuar el desarrollo en Claude Code / VS Code.

Estado: **Fase 1 (FM/OPL3) cerrada, Fase 2 (Wave/PCM) por empezar**. Bitstream con OPL3 funcional en MSX real: MoonBlaster FM, BASIC OUT, VGMPlay con detección MoonSound, reproducciones múltiples sin cuelgues. Repositorio público en `github.com/antxiko/mangOPL4` desde 2026-05-06.

---

## 1. Objetivo final

Implementar la funcionalidad del **Yamaha YMF278 (OPL4)** sobre una **Sipeed Tang Nano 20K** montada en el cartucho **WonderTANG**, de forma que un MSX real lo reconozca como cartucho de sonido compatible con **Sunrise MoonSound** (el estándar de facto del OPL4 en MSX).

Objetivo dividido en dos grandes bloques:

1. **Parte FM (OPL3 / YMF262)**: síntesis FM de 18 canales 2-op o 6 canales 4-op + 6 2-op, compatible con el software MSX existente que ataca los registros OPL3 del MoonSound.
2. **Parte Wavetable (PCM del YMF278)**: 24 canales PCM con ROM YRW801 y SRAM de samples. **Esta parte se aborda después** y requiere escribir el core desde cero porque no existe implementación FPGA pública.

---

## 2. Hardware base

- **Cartucho**: WonderTANG 2.0b (de Luis Felipe Antoniosi, `lfantoniosi/WonderTANG`).
  - Interfaza el bus MSX con una Tang Nano 20K.
  - Gestiona alimentación, slot, inyección de audio a SOUNDIN del MSX (opcional vía JST), salida por DAC interno del Tang Nano.
  - Ya trae lector microSD.
- **FPGA**: GOWIN GW2AR-18 en Tang Nano 20K.
  - ~20.736 LUTs (margen suficiente; el OCM clásico con 12K LEs no podía con OPL4, nosotros tenemos 70% más).
  - PSRAM 64 Mbit (8 MB): suficiente para ROM YRW801 de 2 MB + 1 MB sample RAM + memoria mapper, con holgura.
  - DAC interno + salida I2S disponibles.
- **Toolchain**: Gowin Educative IDE + `openFPGALoader >= v0.10.0`. Ver sección 8 para detalles del entorno de desarrollo.

---

## 3. Decisión arquitectónica principal

**No se parte de cero.** Se forkea un codebase existente que ya resuelve arranque + SD + Nextor + memoria mapper + SCC+, y se le AÑADE el módulo OPL3 decodificando los puertos correspondientes del bus MSX.

### Codebase base elegido: `herraa1/tnCartWonder`

URL: <https://github.com/herraa1/tnCartWonder> (fork adaptado a WonderTANG del tnCart de `buppu3/tnCart`).

**Por qué este y no el `lfantoniosi/WonderTANG` original:**

- Verilog limpio y modular.
- Activamente mantenido, validado por la comunidad MSX.
- Ya trae: MegaFlashROM SCC+ SD, Nextor 2.1, memoria mapper, V9990 emulado (útil para depuración visual con players propios).
- La inclusión del V9990 nos permite escribir software de debug con gráficos ricos sin depender de otros cartuchos.

**Qué NO trae tnCartWonder** (lo que nos toca implementar):

- OPL3 FM (objetivo Fase 1 de este proyecto).
- OPL4 wavetable (objetivo Fase 2, lejano).

### Core OPL3 elegido: `gtaylormb/opl3_fpga`

URL: <https://github.com/gtaylormb/opl3_fpga>.

- Reverse-engineered del Yamaha YMF262 real, SystemVerilog.
- Gowin IDE digiere SystemVerilog.
- **Es la única implementación FPGA pública de OPL3 completa** (JTOPL de Jotego aún NO tiene OPL3 — `jt262.v` está en el roadmap pero no existe).
- Licencia: revisar compatibilidad antes de distribuir.

### Alternativas descartadas (y por qué)

| Alternativa | Motivo de descarte |
|---|---|
| Solo JTOPL2 (OPL2, `jtopl2.v`) | El MSX nunca tuvo OPL2 nativo. No existe software MSX que ataque OPL2 puro, así que no se puede validar en MSX real. |
| Partir del `lfantoniosi/WonderTANG` original | Menos modular, menos V9990, menos mantenido que tnCartWonder. |
| Escribir OPL3 desde cero | Absurdo habiendo un core probado (gtaylormb). |
| Esperar a que Jotego termine `jt262.v` | Sin fecha, bloquea el proyecto. |

---

## 4. Mapeo de puertos del MoonSound (objetivo a emular)

El MoonSound expone el OPL4 en estos puertos I/O del MSX:

| Puerto | Función |
|---|---|
| `C4h` | FM1 register select (banco 1 del OPL3) — también status read |
| `C5h` | FM1 data |
| `C6h` | FM2 register select (banco 2 del OPL3) |
| `C7h` | FM2 data |
| `7Eh` | Wave register select (parte PCM — Fase 2) |
| `7Fh` | Wave data (parte PCM — Fase 2) |

Los cuatro puertos `C4-C7` son el mapeo nativo del YMF262 (OPL3) con sus dos bancos de registros. **Esto es exactamente lo que `gtaylormb/opl3_fpga` sabe hablar**, solo hay que enrutar el bus MSX hacia esos puertos.

### ⚠️ Conflicto de puertos a considerar

- `C0h`-`C1h` = MSX-AUDIO (Y8950) primario.
- `C2h`-`C3h` = MSX-AUDIO secundario.
- `C4h`-`C7h` = MoonSound por defecto.

El MoonSound real tiene dip switch para mover los puertos y evitar choque con MSX-AUDIO. Si se quiere dar esa compatibilidad, replicar el dip virtual desde el firmware (registro de configuración accesible por TNCROM.COM o similar).

---

## 5. Especificaciones técnicas clave

### Reloj

- OPL4 usa **33.8688 MHz** dividido por 684 → base 49.516 Hz → sample rate **44.1 kHz**.
- Esto difiere del OPL3 original de PC (14.31818 MHz).
- **Hay que generar este reloj con un PLL de la Tang Nano 20K y pasárselo al core gtaylormb.**
- Si se deja al reloj original del OPL3 de PC, habrá desafine de ~2 Hz respecto al MoonSound real. Los tests con VGMs lo detectan.

### Salida de audio

- gtaylormb entrega PCM digital (16-bit estéreo).
- Dos destinos disponibles en la WonderTANG:
  - **DAC interno del Tang Nano 20K**: el que ya usa WonderTANG 2.0b vía pin JST. Suficiente para esta fase.
  - **I2S a DAC externo**: si se quisiera mejor calidad más adelante.
- **SOUNDIN del MSX**: útil para mezclar el audio con el interno del MSX y oírlo por SCART/TV. La WonderTANG ya lo cablea.

### Interfaz de bus MSX

- Ya resuelta en tnCartWonder.
- Solo hay que AÑADIR el decodificador de los puertos `C4-C7` (señales `IORQ_n`, `WR_n`, `RD_n`, `A[7:0]`, `D[7:0]`) y enrutar hacia el wrapper del core OPL3.

---

## 6. Roadmap por fases

### Fase 0 — Preparación del entorno ✅ COMPLETA (2026-04-26)

- [x] Confirmar que la WonderTANG arranca en el MSX con su firmware stock (Nextor + SD).
- [x] Instalar toolchain: Gowin Educative IDE V1.9.11.03 + `openFPGALoader v1.0.0`.
- [x] Forkear `herraa1/tnCartWonder` a `antxiko/tnCartWonder`.
- [x] Clonar fork en local y sintetizar el bitstream "tal cual".
- [x] Flashear bitstream stock y confirmar arranque Nextor → MSX-DOS 2 → ejecución de `.com` desde SD.

### Fase 1 — OPL3 básico funcional ✅ COMPLETA (2026-05-06)

- [x] Añadido `gtaylormb/opl3_fpga` como submódulo (fork `antxiko/opl3_fpga`).
- [x] Simulación con Verilator (`sim/opl3_smoke/`): pico fundamental 440.0 Hz exactos validado contra config FNUM/BLOCK.
- [x] Wrapper MSX `cartridge_opl3.sv`: decodificación C4-C7, BUSDIR_n, shadow regs para read-back, stub Wave port (7F=0x20) para detección MoonSound, IRQ generator pulso+gap+repeat con watchdog.
- [x] Reloj OPL3 a 33.5625 MHz (CLK_TMDS_S /4 vía CLKDIV) con CLK_DIV_COUNT=678 → sample rate 49.502 kHz (-0.03% vs MoonSound real, inaudible).
- [x] Salida PCM mezclada con SCC+, OPLL, etc. en el mixer existente (atenuadores ext+int).
- [x] Síntesis (Logic 45%, CLS ~55%) + flasheo + test en MSX real:
  - MoonBlaster FM ✅ (es el caso "fácil", no usa Timer1)
  - BASIC OUT directo a C5/C7 ✅
  - VGMPlay-MSX (Grauw) detecta MoonSound y reproduce VGMs OPL3 ✅
  - Reproducciones múltiples sin cuelgue ✅
- [ ] Conflicto de puertos con MSX-AUDIO (C0-C3) — **diferido**: ningún MSX del usuario tiene MSX-AUDIO real. Documentado.

**Bugs descubiertos y resueltos en Fase 1** (capturados en commits del fork `antxiko/opl3_fpga`):
1. **Gowin trunca `real * real`** (warn EX3791 size 64→32): `CLK_FREQ * TIMER_TICK_INTERVAL` da valor erróneo, ralentiza Timer1 ~30x. Fix: precomputar `TIMER1_TICK_COUNT=2685` y `TIMER2_TICK_COUNT=10740` como `localparam int` en `opl3_pkg_mangopl4.sv`.
2. **`irq_rst` sticky en gtaylormb**: en YMF262 real es STROBE (write 1 → clear flags → auto a 0), pero el código upstream lo deja como FF persistente. Tras la primera escritura del IRQ handler queda atascado en 1 y cancela todo overflow futuro. Fix: `irq_rst <= 0` default en always_ff de `timers.sv`.
3. **`assert property` no digerible por Gowin** en `control_operators.sv`: envuelto en `// synthesis translate_off/on`.
4. **IRQ wiring**: pulso INT_n único de 100 µs colgaba MSX (BIOS handler ~50 µs no acaba a tiempo), pulso 5 µs sin re-arme dejaba sin reproducción tras un IRQ perdido. Fix definitivo: máquina pulso 5 µs + gap 50 µs + repeat continuo mientras `irq_active=1`, con watchdog que tras 32 pulsos sin ack entra en `gave_up` (INT_n alto permanente).
5. **Backdoor `force_clear_flags`**: sin él, tras exit "natural" de VGMPlay (Timer1 sigue corriendo, ft1 sticky, handler desinstalado) la próxima ejecución no detecta MoonSound. Cuando watchdog se rinde, el wrapper drivea `gave_up=1` que fuerza internamente `st1=0`, `st2=0`, `ft1=0`, `ft2=0` en el core. Cero coste en LUTs, rompe el deadlock sin tocar el bus.

### Fase 2 — Wavetable OPL4 (futuro, NO es el objetivo inmediato)

- Escribir desde cero el core del wavetable (24 canales PCM, envelope generators, interpolación, panning, mezcla).
- Cargar ROM YRW801 en PSRAM (aportada por el usuario — no distribuible por copyright).
- Exponer en puertos `7Eh`-`7Fh`.
- Mezclar con salida FM.

### Fase 3 — Cartucho propio con FPGA integrada (lejano, fuera del alcance inmediato)

Convertir el conjunto "WonderTANG + Tang Nano 20K insertado" en un único cartucho MSX de diseño propio, con el **GW2AR-18 soldado directamente** en el PCB del cartucho junto con su alimentación, cristales, flash de bitstream, PSRAM, level shifters y salida de audio.

Es un proyecto hardware completamente distinto al actual (diseño electrónico, KiCad, BOM, fabricación, ensamblaje SMD fino), con sus propios trade-offs a decidir en su momento:

- ¿Multiplexar A0-A7/D0-D7 con MSEL como hace la WonderTANG, o rutear pines sin multiplexar aprovechando que el GW2AR-18 pelado tiene más pines disponibles?
- 74LVC245 como buffers (opción probada) vs level shifters bidireccionales dedicados (TXB/TXS0108 — evitar, dan problemas con bus Z80 rápido).
- MAX98357A I2S Class-D (como v2.0b) vs DAC 1-bit sintetizado por el FPGA con filtro RC (como v1.02d — menos ruido).
- PSRAM/HyperRAM del Tang Nano integrada vs SDRAM discreta.
- Programador onboard (BL616 o FT2232H) vs pines JTAG expuestos para programador externo.
- Encapsulado del GW2AR-18: QN88 QFN 0.5mm pitch — **requiere horno de reflow o hot air + stencil**, no suelda a mano.

**Preparación cuando llegue el momento:**

- Crear documento `docs/CUSTOM_CARTRIDGE.md` con justificación, decisiones de diseño, BOM, esquemático KiCad, consideraciones de fabricación y revisiones.
- Decidir si va en este mismo repo (carpeta `hardware/`) o en repo separado (`cartucho-propio`).
- Aprovechar que el esquemático del Tang Nano 20K es open source (publicado por Sipeed) como punto de partida.
- Usar `docs/TANG_MSX_INTERFACE.md` como referencia conceptual de cómo debe funcionar la interfaz con el bus MSX.

---

## 7. Cuestiones abiertas / riesgos

- **Licencias**: gtaylormb/opl3_fpga vs GPL de tnCart (de jtopl si se acaba usando). Revisar compatibilidad antes de distribuir un bitstream combinado.
- **YRW801 ROM**: copyright Yamaha. Cualquier release pública exige que el usuario aporte su propio dump. No está legalmente distribuible.
- **Recursos FPGA**: caben OPL3 + Wave + todo lo de tnCart, pero al añadir OPL3 hay que verificar síntesis con `timing report` y `resource utilization` de Gowin para no romper Fmax.
- **Compatibilidad con distintos MSX**: la WonderTANG da problemas con algunos Philips; tnCart tiene reportes de artefactos gráficos en algunas máquinas. Probar en distintos equipos.

---

## 8. Entorno de desarrollo

**Plataforma**: Windows 11 sobre AMD Ryzen.

### Herramientas a instalar

| Herramienta | Uso | Notas de instalación |
|---|---|---|
| Gowin Educative IDE (v1.9.9 o superior) | Síntesis, place & route, generación de bitstream `.fs` | Nativo Windows. Descargar de <https://www.gowinsemi.com/> (requiere registro). Reiniciar tras instalar drivers. |
| openFPGALoader (>= v0.10.0) | Flasheo del bitstream en Tang Nano 20K | Binarios Windows en <https://github.com/trabucayre/openFPGALoader/releases>. Alternativa: usar el programador de Gowin IDE (a veces falla en Linux, en Windows suele ir bien). |
| Git for Windows | Control de versiones, submódulos | <https://git-scm.com/download/win>. Incluye Git Bash (útil para comandos Unix-style). |
| VS Code + Claude Code | Editor + IA assistant | Convención: `CLAUDE.md` en la raíz del repo. |
| WSL2 (Ubuntu 22.04 o 24.04) | **Simulación con Verilator** | Verilator en Windows nativo es engorroso. En WSL2 es trivial: `sudo apt install verilator`. |
| Visual Studio Code extensions | Verilog-HDL, TerosHDL, o similar | Highlighting y linting de Verilog/SystemVerilog. |

### Flujo típico de trabajo

1. **Edición y git**: VS Code en Windows nativo.
2. **Simulación (Verilator)**: terminal WSL2 → el repo está montado en `/mnt/c/Users/<user>/...` accesible desde Ubuntu.
3. **Síntesis (Gowin IDE)**: Windows nativo. Carga el proyecto desde `C:\...\nombre_proyecto`.
4. **Flasheo**: Windows nativo con openFPGALoader o el programador de Gowin IDE, vía USB a la Tang Nano 20K montada en la WonderTANG (que a su vez estará conectada al MSX, o a un adaptador USB-C para flasheo sin el MSX encendido).

### Ventajas del setup Windows

- Gowin IDE corre nativo y sin capas de traducción.
- El programador de Gowin IDE funciona bien en Windows (en Linux hay que recurrir sí o sí a openFPGALoader).
- Drivers USB-JTAG de la Tang Nano 20K se instalan sin problema.

### Precauciones

- **Finales de línea**: configurar Git con `core.autocrlf=input` para no introducir CRLF en archivos Verilog que tnCartWonder tiene en LF.
- **Rutas largas**: Windows a veces se queja. Si el proyecto da errores crípticos, mover a una ruta corta tipo `C:\wt\`.
- **WSL2 ↔ Windows**: editar siempre desde el mismo sitio (no editar un archivo en Windows mientras WSL2 lo tiene abierto en vim/nano). Lo más simple: editar en VS Code Windows, simular en WSL2.

---

## 9. Preferencias de trabajo (para Claude Code)

- Respuestas **en español**, técnicas, directas, sin relleno.
- **Contenido completo de los archivos, no snippets parciales.**
- Instrucciones paso a paso indicando **en qué ventana de Terminal / qué directorio** se ejecuta cada comando.
- El usuario **no tiene formación formal de programación**; sí tiene experiencia construyendo sistemas complejos con asistencia guiada. Verilog/SystemVerilog es nuevo para él: explicar conceptos de RTL sin dar por hecho nada.
- Preferencia de stack general: Node.js + SQLite + Docker (no aplica aquí, es HDL).

---

## 10. Referencias

### Repos

- tnCartWonder (base): <https://github.com/herraa1/tnCartWonder>
- tnCart (upstream): <https://github.com/buppu3/tnCart>
- WonderTANG original: <https://github.com/lfantoniosi/WonderTANG>
- gtaylormb/opl3_fpga: <https://github.com/gtaylormb/opl3_fpga>
- jotego/jtopl (referencia, no se usa): <https://github.com/jotego/jtopl>
- Wozblaster (HW de referencia MoonSound): <https://github.com/cristianoag/wozblaster>

### Documentación interna del proyecto

- `docs/TANG_MSX_INTERFACE.md` — Capa física y protocolo del bus MSX visto desde el FPGA (topología eléctrica, 74LVC245, multiplexado MSEL, ciclo Z80 I/O, sincronización, decodificación C4-C7, audio I2S, gotchas). **Lectura obligatoria antes de escribir RTL que toque el bus MSX.**
- `docs/CUSTOM_CARTRIDGE.md` — (pendiente, solo cuando se arranque la Fase 3).

### Documentación técnica externa

- YMF278B Application Manual (Yamaha): buscar PDF, referencia completa del chip.
- MoonSound Technical Documentation: <https://wiki.preterhuman.net/MoonSound_Technical_Documentation>
- OPL4.txt de Remco Schrijvers: <http://faq.msxnet.org/opl4.html>
- openMSX `ymf278.cc`: referencia para traducir a RTL la parte wavetable (Fase 2).
- VGMPlay `ymf278b.c`: segunda referencia para la parte wavetable.

### Test VGMs / software MSX

- Meridian (MoonSound FM support).
- Moonblaster FM (MBFM).
- VGMPlay MSX (Grauw).
- Nop's players.
- MoonTest (<https://github.com/Wierzbowsky/MoonTest>) para test de RAM del wavetable.

---

## 11. Identidad del proyecto

### Nombre

**MangOPL4** — mango (fruta) + OPL4 (chip objetivo). Decidido el <fecha de kickoff>.

**Convenciones de uso**:

- En commits, rutas de archivo y nombres técnicos: `mangopl4` (minúsculas, sin separadores ni caracteres especiales).
- En prosa, logos y documentación: `MangOPL4`.
- Pronunciación: "mango-pi-ele-cuatro" o "mango-OPL4".
- Repositorio local: `C:\Users\Antxiko\Documents\frutOPL4\`.
- Repositorio remoto: `github.com/antxiko/mangOPL4`.

### Visibilidad del repo

**PÚBLICO desde 2026-05-06** (Fase 1 cerrada). Antes era privado para trabajar sin presión externa. La parte de YRW801 (Fase 2) seguirá tratándose con cuidado: el dump del ROM no se incluirá en el repo (lo aporta el usuario), igual que hacen Wozblaster y similares.
