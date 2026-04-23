# MangOPL4

Implementación FPGA del Yamaha **YMF278B (OPL4 / MoonSound)** sobre Sipeed Tang Nano 20K montado en cartucho **WonderTANG v2.0b**, para uso en MSX real.

**Estado**: planificación / pre-Fase 0. Ninguna línea de RTL escrita todavía.

## Documentación

Antes de tocar nada, leer en este orden:

1. [`CLAUDE.md`](CLAUDE.md) — plan general, decisiones arquitectónicas, roadmap por fases.
2. [`docs/KICKOFF.md`](docs/KICKOFF.md) — onboarding y flujo de sesiones con Claude Code.
3. [`docs/TANG_MSX_INTERFACE.md`](docs/TANG_MSX_INTERFACE.md) — capa física y protocolo del bus MSX.
4. [`docs/SETUP_WINDOWS.md`](docs/SETUP_WINDOWS.md) — entorno de desarrollo Windows 11 + WSL2.

Vivos:

- [`docs/PROGRESS.md`](docs/PROGRESS.md) — pendiente, se crea al completar el primer hito de Fase 0.
- [`docs/DECISIONS.md`](docs/DECISIONS.md) — pendiente, se crea junto a `PROGRESS.md`.

## Estructura del repo

```
mangopl4/
├── CLAUDE.md          contexto permanente para Claude Code
├── README.md
├── .gitignore
├── .gitattributes     fuerza EOL=LF en archivos HDL
├── docs/              documentación de planificación + estado vivo
├── external/          submódulos (tnCartWonder, opl3_fpga…) — vacío todavía
├── rtl/               wrapper MSX y RTL propio — vacío todavía
├── sim/               testbenches Verilator — vacío todavía
└── tools/             scripts auxiliares — vacío todavía
```

## Licencia

Por decidir. Probablemente **GPL-3.0** por compatibilidad con el core [`gtaylormb/opl3_fpga`](https://github.com/gtaylormb/opl3_fpga) que se integrará en Fase 1.

## Repos relacionados

- Codebase base: [`herraa1/tnCartWonder`](https://github.com/herraa1/tnCartWonder) (fork de [`buppu3/tnCart`](https://github.com/buppu3/tnCart) adaptado a WonderTANG).
- Hardware: [`lfantoniosi/WonderTANG`](https://github.com/lfantoniosi/WonderTANG).
- Core OPL3: [`gtaylormb/opl3_fpga`](https://github.com/gtaylormb/opl3_fpga).
