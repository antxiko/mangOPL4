# Decisiones técnicas

Log inmutable de decisiones técnicas tomadas durante el proyecto.
Nueva decisión = nueva entrada al final, con fecha. NO se borra ni se edita contenido anterior (si una decisión se revoca, se añade nueva entrada que la supersede y se referencia desde aquí).

## D-001 — Codebase base: `herraa1/tnCartWonder`

**Fecha**: planificación previa (sesión web de Claude)

**Decisión**: usar `herraa1/tnCartWonder` como base, no el `lfantoniosi/WonderTANG` original ni partir de cero.

**Razón**: RTL más limpio y modular, activamente mantenido, validado por la comunidad MSX, incluye V9990 emulado útil para debug visual.

**Alternativas descartadas**: `lfantoniosi/WonderTANG` original (menos modular), partir de cero (descarta meses de trabajo ya validado).

## D-002 — Core OPL3: `gtaylormb/opl3_fpga`

**Fecha**: planificación previa

**Decisión**: usar `gtaylormb/opl3_fpga` (SystemVerilog, reverse-engineered del YMF262 real) como core OPL3.

**Razón**: única implementación FPGA pública completa de OPL3. JTOPL de Jotego solo tiene OPL2 (`jt262.v` está en roadmap pero no existe). Gowin IDE digiere SystemVerilog.

**Alternativas descartadas**: JTOPL2 solo (MSX nunca tuvo OPL2 nativo, no hay software MSX que lo ataque), escribir desde cero (inviable habiendo un core probado), esperar a que Jotego termine `jt262.v` (sin fecha, bloquea el proyecto).

**Pendiente**: revisar compatibilidad de licencias entre `gtaylormb/opl3_fpga` y el resto del codebase (`tnCartWonder`, `tnCart`, JTOPL si se acaba usando para algo) antes de distribuir un bitstream combinado.

## D-003 — Entorno de desarrollo: Windows 11 + WSL2

**Fecha**: planificación previa

**Decisión**: Windows nativo para Gowin/git/flasheo/edición; WSL2 Ubuntu 24.04 para simulación Verilator y scripting Bash.

**Razón**: Gowin Educative IDE corre nativo en Windows; el programador de Gowin y los drivers USB-JTAG funcionan bien en Windows; Verilator es trivial en Ubuntu y engorroso en Windows nativo. VS Code + Claude Code unen ambos mundos.

**Alternativas descartadas**: macOS (Gowin no soportado), Linux puro (Gowin Linux es menos estable + obliga a usar siempre `openFPGALoader` para flasheo).

## D-004 — Orden de fases: 0 → 1 OPL3 → 2 Wavetable

**Fecha**: planificación previa

**Decisión**: Fase 0 (validar arranque stock de tnCartWonder en MSX real) antes de tocar RTL propio. Fase 1: añadir OPL3 al codebase. Fase 2: wavetable PCM (lejana).

**Razón**: sin un MSX arrancando con bitstream stock, no hay banco de pruebas para validar nada nuevo. Sin OPL3 funcional no tiene sentido meter wavetable (que además requiere escribir el core desde cero).

## D-005 — Submódulo `tnCartWonder` apunta a upstream `herraa1` (no a fork del usuario)

**Fecha**: 2026-04-23

**Decisión**: el submódulo `external/tnCartWonder` apunta provisionalmente a `https://github.com/herraa1/tnCartWonder` (URL HTTPS, no SSH).

**Razón**: el usuario ha decidido no crear todavía repos en GitHub propios (ni el del proyecto MangOPL4 ni el fork de tnCartWonder). Para no bloquear el avance se apunta al upstream y se cambiará la URL con `git submodule set-url` cuando se cree el fork privado.

**Implicación**: si se modifican archivos del submódulo localmente, los commits no se podrán pushear al upstream (sin permisos). Antes del primer cambio sustancial al RTL del submódulo hay que crear el fork del usuario y reapuntar el submódulo.

## D-006 — `openFPGALoader` vía MSYS2 (no instalación standalone)

**Fecha**: 2026-04-23

**Decisión**: instalar `openFPGALoader` como paquete MSYS2 (`pacman -S mingw-w64-x86_64-openFPGALoader`), no como release `.zip` standalone como sugería [`SETUP_WINDOWS.md` §5](SETUP_WINDOWS.md).

**Razón**: a partir de v1.1.1 los releases del proyecto solo distribuyen paquetes MSYS2 / Ubuntu, ya no `.zip` Windows simples. MSYS2 ya estaba instalado en este sistema (`C:\msys64\`). La versión empaquetada (0.13.1-1, binario reporta `v1.0.0`) cumple el requisito `>= 0.10.0` que pide `SETUP_WINDOWS.md`.

**Implicación**: para usar `openFPGALoader.exe` desde cualquier terminal Windows hay que tener `C:\msys64\mingw64\bin` en el PATH (ya añadido al User PATH). Si en el futuro se actualiza el binario, basta `pacman -Syu`.
