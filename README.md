# MangOPL4

Implementación FPGA del **Yamaha YMF278B (OPL4 / MoonSound)** sobre Sipeed Tang Nano 20K montado en cartucho **WonderTANG v2.0b**, para uso en MSX real.

Construido sobre [`herraa1/tnCartWonder`](https://github.com/herraa1/tnCartWonder) (fork de [`buppu3/tnCart`](https://github.com/buppu3/tnCart) adaptado a la WonderTANG, con MegaFlashROM SCC+, Nextor 2.1, memoria mapper y emulación V9990). MangOPL4 añade encima el cartucho **OPL3 (MoonSound FM)** mapeado en C4-C7h, integrando el core [`gtaylormb/opl3_fpga`](https://github.com/gtaylormb/opl3_fpga).

## Estado

**Fase 1 en validación**. El bitstream con OPL3 integrado arranca Nextor en MSX real, el audio path está confirmado (escala C4-C5 audible en BASIC con volumen normal), detección por status read OK. Pendiente cerrar la detección con timer overflow para que players estilo VGMPlay reconozcan el chip.

Roadmap completo: ver [`CLAUDE.md`](CLAUDE.md) §6 y [`docs/PROGRESS.md`](docs/PROGRESS.md).

## Arquitectura

```
MSX bus (C4-C7h)
       │
       ▼
┌──────────────────┐    ┌─────────────────┐
│ CARTRIDGE_OPL3   │───▶│ opl3 (gtaylormb) │
│ (wrapper MSX)    │    │ 33.5625 MHz     │
└──────────┬───────┘    └─────────────────┘
           │ PCM 24→10 bit signed (gain 64x)
           ▼
   SOUND_MIXER ──▶ MAX98357A I2S DAC ──▶ jack 3.5mm + SOUNDIN MSX
```

El reloj de 33.5625 MHz se obtiene con `CLKDIV /4` desde el PLL TMDS existente — los 2 PLL del GW2AR-18 ya estaban al 100%, así que se aprovecha un output libre. Error -0.9% respecto a los 33.8688 MHz nominales del MoonSound real, compensado en `opl3_pkg_mangopl4.sv` con `CLK_DIV_COUNT=678` (sample rate 49.502 kHz vs 49.516 kHz, error inaudible).

## Documentación

Antes de tocar nada, leer en este orden:

1. [`CLAUDE.md`](CLAUDE.md) — plan general, decisiones arquitectónicas, roadmap por fases.
2. [`docs/KICKOFF.md`](docs/KICKOFF.md) — onboarding y flujo de sesiones con Claude Code.
3. [`docs/TANG_MSX_INTERFACE.md`](docs/TANG_MSX_INTERFACE.md) — capa física y protocolo del bus MSX (incluye apéndices A/B con `board_wt200b.cst` real y discrepancias frente al §16 conceptual).
4. [`docs/SETUP_WINDOWS.md`](docs/SETUP_WINDOWS.md) — entorno de desarrollo Windows 11 + WSL2.

Vivos:

- [`docs/PROGRESS.md`](docs/PROGRESS.md) — estado actual, hitos completados/pendientes, log de sesiones.
- [`docs/DECISIONS.md`](docs/DECISIONS.md) — decisiones técnicas con su justificación.

## Estructura del repo

```
mangOPL4/
├── CLAUDE.md             contexto permanente para Claude Code
├── README.md
├── .gitignore
├── .gitattributes        EOL=LF en HDL/docs, .asc binario (CRLF MSX BASIC)
├── .gitmodules
├── docs/                 documentación de planificación + estado vivo
├── external/
│   ├── tnCartWonder/     submódulo → antxiko/tnCartWonder
│   └── opl3_fpga/        submódulo → antxiko/opl3_fpga (LGPL-3.0)
├── rtl/                  reservado para RTL propio del parent (vacío)
├── sim/
│   └── opl3_smoke/       testbench Verilator del core OPL3
└── tools/
    ├── flash/            bitstreams y BIOS staged para flashear
    └── msx/              tests BASIC reproducibles en MSX real (.asc)
```

## Forks utilizados

Este parent repo apunta a forks propios de los proyectos upstream para poder añadir cambios necesarios para MangOPL4:

- **[`antxiko/tnCartWonder`](https://github.com/antxiko/tnCartWonder)**: añade `CARTRIDGE_OPL3`, decodificación C4-C7h, PLL/CLKDIV para 33.5625 MHz, V9990 deshabilitado para hacer hueco al OPL3, y constraint SDC del nuevo reloj.
- **[`antxiko/opl3_fpga`](https://github.com/antxiko/opl3_fpga)**: envuelve la única `assert property` del core en `// synthesis translate_off/on` para que Gowin V1.9.11.03 no aborte.

## Licencia

Probablemente **LGPL-3.0** por compatibilidad con el core OPL3 de gtaylormb (LGPL-3.0). El codebase base de tnCartWonder está bajo BSD-3-Clause, compatible con LGPL.

Notas sobre archivos no distribuibles:

- **YRW801 ROM** (Fase 2, wavetable): copyright Yamaha, no se distribuye.
- **Nextor kernel**: distribuido aparte por Konamiman.
- **FM BIOS**: viene con tnCartWonder.

## Repos relacionados

- Hardware base: [`lfantoniosi/WonderTANG`](https://github.com/lfantoniosi/WonderTANG)
- Codebase upstream: [`herraa1/tnCartWonder`](https://github.com/herraa1/tnCartWonder) (fork de [`buppu3/tnCart`](https://github.com/buppu3/tnCart))
- Core OPL3: [`gtaylormb/opl3_fpga`](https://github.com/gtaylormb/opl3_fpga)
- Core OPL2 referencia: [`jotego/jtopl`](https://github.com/jotego/jtopl) (no se usa)
- HW de referencia MoonSound: [`cristianoag/wozblaster`](https://github.com/cristianoag/wozblaster)
