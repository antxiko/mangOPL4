# Progreso del proyecto

Estado vivo del avance. Se actualiza al final de cada sesión de trabajo.

## Resumen ejecutivo

- **Fase actual**: Fase 0 — Preparación del entorno
- **Última sesión**: 2026-04-23
- **Siguiente hito**: bitstream stock de tnCartWonder sintetizado y flasheado, MSX arrancando Nextor desde microSD

## Hitos completados

- [x] Planificación inicial en sesión web de Claude (`CLAUDE.md` + `docs/`)
- [x] Repo local creado con documentos de planificación (commit `02678e1`)
- [x] `tnCartWonder` añadido como submódulo apuntando al upstream `herraa1/tnCartWonder@d3a54f1` (commit `3de2cec`)
- [x] git global configurado con identidad del plan
- [x] `openFPGALoader v1.0.0` instalado vía MSYS2 y añadido al User PATH
- [x] Inventario de entorno completo (qué falta vs qué hay)
- [x] Mapeo arquitectónico de `tnCartWonder/wt200b` documentado (top, mixer, I2S, decodificación, PLLs)
- [x] `board_wt200b.cst` real anexado a `docs/TANG_MSX_INTERFACE.md` (Apéndice A) + tabla de discrepancias con §16 (Apéndice B)
- [x] 7-Zip y Audacity instalados (winget)
- [ ] WSL2 + Ubuntu 24.04 instalado (bloqueado: requiere PowerShell admin del usuario)
- [ ] Verilator + GTKWave + iverilog en WSL2 (depende de ↑)
- [ ] Gowin Educative IDE V1.9.9.03+ instalado y licenciado (bloqueado: requiere registro humano + descarga ~3 GB + licencia por correo)
- [ ] Fork de `tnCartWonder` en cuenta GitHub del usuario (decisión: pospuesto)
- [ ] Repo `mangopl4` creado en GitHub privado (decisión: pospuesto)
- [ ] Bitstream stock sintetizado en Gowin IDE
- [ ] Bitstream flasheado en Tang Nano 20K
- [ ] MSX arranca Nextor desde microSD
- [ ] MSX ejecuta un `.com` desde microSD
- [ ] VGMPlay + openMSX (referencias para Fase 1, no urgente)

## Log de sesiones

### 2026-04-23 — Sesión 1 (kickoff)

**Lo hecho**:

- Lectura de los cuatro documentos de planificación.
- Verificación del entorno Windows: git OK, MSYS2 ya instalado, `gh` CLI presente, VS Code OK. WSL2 no, Gowin IDE no, openFPGALoader no.
- `openFPGALoader v1.0.0` instalado vía `pacman -S mingw-w64-x86_64-openFPGALoader`. Añadido `C:\msys64\mingw64\bin` al User PATH (PowerShell, scope User, sin admin).
- git global: `user.name "Jokin Miragaia"`, `user.email "jokin.miragaia@gmail.com"`, `core.autocrlf=input`, `init.defaultBranch=main`. Host key de github.com aceptado en `~/.ssh/known_hosts`.
- Repo local creado en `C:\wt\mangopl4\` con estructura de [`KICKOFF.md` §3](KICKOFF.md): `CLAUDE.md` en raíz, `docs/`, `external/`, `rtl/`, `sim/`, `tools/`. `.gitignore` (Gowin/Verilator/VS Code) y `.gitattributes` (LF forzado en HDL/docs).
- Cuatro documentos de planificación movidos a sus rutas. Commit inicial `02678e1 docs: add project planning documents from web session`.
- `tnCartWonder` añadido como submódulo con URL HTTPS upstream (sin SSH, sin fork del usuario): commit `3de2cec feat: add tnCartWonder as submodule (upstream herraa1)`.
- Inspección arquitectónica del codebase wt200b (vía agente Explore): identificado top-level (`tnCart_board_wt200b_top.sv`), mixer (`SOUND_MIXER` en `peripheral/sound/sound.sv`, ancho 10 bits), I2S (`I2S_AUDIO_TX`/`DAC_16BIT`, ≈48 kHz), patrón de integración de cartridges (`SOUND_IF` + `SOUND_ATTENUATOR`), bus decoder (`BOARD_REV1_BUS` con state machine de 9 ciclos), PLLs (`u_pll_base` 107.4 MHz, `u_pll_tmds` 134.25 MHz; ninguno en 33.8688 MHz).
- `board_wt200b.cst` anexado a `docs/TANG_MSX_INTERFACE.md` como Apéndice A. Apéndice B documenta discrepancias críticas con la descripción conceptual del §16 (naming `_33` → `CART_*`, 3 chips multiplex en lugar de 2, bus de datos no multiplexado, etc.).
- 7-Zip y Audacity instalados con winget en background.

**Problemas / pendientes**:

- WSL2 + Ubuntu no se puede instalar desde el sandbox (requiere PowerShell admin). Pendiente: usuario debe lanzar `wsl --install -d Ubuntu-24.04` desde PowerShell elevada y reiniciar PC. Después: configurar Ubuntu siguiendo [`SETUP_WINDOWS.md` §2](SETUP_WINDOWS.md).
- Gowin Educative IDE no automatizable: registro en gowinsemi.com + descarga ~3 GB + solicitud de licencia educativa por correo + instalación + activación. Pendiente: usuario debe seguir [`SETUP_WINDOWS.md` §4](SETUP_WINDOWS.md).
- SSH keys de GitHub no generadas/añadidas. Pendiente cuando se quiera hacer push.
- No se ha creado repo remoto en GitHub (decisión explícita del usuario).
- Discrepancia entre §16 conceptual de `TANG_MSX_INTERFACE.md` y la realidad wt200b documentada en Apéndice B. El §16 sigue siendo útil como introducción al concepto de multiplexado, pero algunos detalles (nombres, número de chips, sample rate) son del esquema antiguo de tnCart rev1.

**Decisiones tomadas en esta sesión** (ver `DECISIONS.md`):

- D-005: submódulo `tnCartWonder` apunta a upstream `herraa1` con HTTPS (no SSH, no fork).
- D-006: `openFPGALoader` instalado vía MSYS2 en lugar del `.zip` standalone.

**Siguiente paso (al volver)**:

1. Usuario instala WSL2 + Ubuntu 24.04 (desde PowerShell admin) y Gowin Educative IDE (manual).
2. Cuando ambos estén listos: abrir `external/tnCartWonder/rtl/tnCart_board_wt200b.gprj` en Gowin IDE, sintetizar bitstream stock, conectar Tang Nano 20K + WonderTANG 2.0b, flashear con `openFPGALoader -b tangnano20k impl/pnr/<bitstream>.fs`, comprobar arranque Nextor → MSX-DOS desde microSD en MSX real.
3. Tras validar el bitstream stock: marcar Fase 0 completada, abrir Fase 1 con la integración del core OPL3.
