# Proyecto: OPL3/OPL4 FPGA sobre WonderTANG para MSX

Documento de memoria y diseño para continuar el desarrollo en Claude Code / VS Code.

Estado: **planificación (ninguna línea de RTL escrita todavía)**.

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

### Fase 0 — Preparación del entorno (antes de tocar RTL)

- [ ] Confirmar que la WonderTANG arranca en el MSX con su firmware stock (Nextor + SD).
- [ ] Instalar toolchain: Gowin Educative IDE + `openFPGALoader >= v0.10.0`.
- [ ] Forkear `herraa1/tnCartWonder` a cuenta propia.
- [ ] Clonar fork en local y sintetizar el bitstream "tal cual" para verificar que el flujo de build funciona.
- [ ] Flashear ese bitstream stock y reconfirmar arranque Nextor → MSX-DOS 2 → ejecución de `.com` desde SD.

### Fase 1 — OPL3 básico funcional (objetivo de este proyecto)

- [ ] Añadir `gtaylormb/opl3_fpga` como submódulo git en el fork.
- [ ] **Simulación con Verilator ANTES de tocar hardware**:
  - Montar testbench que alimente VGMs de OPL3 al core.
  - Comparar WAV generado contra salida de openMSX/VGMPlay.
  - Iterar hasta que la diferencia sea mínima (idealmente bit-exacta).
- [ ] Escribir wrapper MSX en Verilog: decodificación `C4-C7`, handshake con el bus, glue al core.
- [ ] Generar reloj de 33.8688 MHz con PLL Gowin.
- [ ] Integrar salida PCM del core al DAC interno del Tang Nano (mezclando con el audio existente de tnCart: SCC+, OPLL, etc.).
- [ ] Síntesis completa, flasheo, test en MSX real con VGM player de MoonSound y con Meridian / Moonblaster FM.
- [ ] Gestionar conflicto de puertos con MSX-AUDIO (al menos documentar, idealmente registro de configuración).

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
- Repositorio local: `C:\wt\mangopl4\`.
- Repositorio remoto: `github.com/<usuario>/mangopl4`.

### Visibilidad del repo

**Privado de momento**. Razones:

- Los documentos contienen detalles del entorno de desarrollo y decisiones que aún pueden cambiar.
- Se quiere trabajar sin presión externa durante las primeras fases.
- El YRW801 ROM (Fase 2) tiene copyright Yamaha y es mejor tener el repo privado mientras se define cómo gestionarlo.

**Cuándo pasar a público**: cuando la Fase 1 (OPL3) esté validada funcionando en MSX real y se quiera abrir a la comunidad MSX para feedback. Se revisará en su momento qué archivos pueden quedar dentro del repo público (bitstreams, RTL propio) y cuáles no (cualquier derivado de YRW801, licencias de terceros con restricciones).
