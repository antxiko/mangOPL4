# Kickoff — De la planificación a Claude Code en VS Code

Este documento es el **puente** entre la sesión de planificación en chat web (donde se generaron `CLAUDE.md`, `docs/TANG_MSX_INTERFACE.md` y `docs/SETUP_WINDOWS.md`) y la primera sesión de trabajo real con Claude Code en VS Code.

Leer este documento antes de abrir Claude Code la primera vez.

---

## 1. ¿Qué va a pasar exactamente?

Claude Code es un cliente de Claude distinto al chat web:

- **No tiene memoria entre sesiones**. Cada vez que abres una sesión nueva, empieza de cero.
- **Sí lee automáticamente el `CLAUDE.md` que esté en la raíz del repo abierto**. Eso es su contexto permanente.
- **Puede leer otros archivos del repo**, pero solo los que se le indiquen explícitamente o los que él mismo abra al ir navegando.
- **Puede ejecutar comandos** en la terminal integrada de VS Code (PowerShell en Windows o Bash en WSL2, según cuál esté activa).
- **Puede crear, modificar y borrar archivos** del repo.
- **Puede hacer commits y push de git** si se le pide.

Por tanto, para que Claude Code retome el proyecto exactamente donde lo dejamos, necesita tres cosas:

1. Los **documentos de planificación** accesibles en el repo (ya los tienes).
2. Un **prompt inicial** claro que le diga qué leer y qué NO hacer aún.
3. Un **mecanismo de continuidad** entre sesiones (commits limpios + un `docs/PROGRESS.md` vivo).

---

## 2. Prerequisitos antes de abrir Claude Code

Antes de la primera sesión de Claude Code, tener hecho:

- [ ] Entorno Windows 11 montado según `docs/SETUP_WINDOWS.md` (checklist final del §10 de ese documento).
- [ ] WonderTANG 2.0b en mano (o pedida y llegando).
- [ ] Un MSX real para pruebas, verificado que funciona.
- [ ] Cuenta GitHub creada y SSH configurado.
- [ ] **Repo privado** creado en GitHub con nombre `mangopl4` (Settings → Private al crear). Todavía no tiene contenido.
- [ ] Fork de `herraa1/tnCartWonder` en tu GitHub (NO clonar todavía — eso lo hará Claude Code en la primera sesión, paso a paso).
- [x] Nombre del proyecto decidido: **MangOPL4**. Ver `CLAUDE.md` §11.
- [ ] Los tres documentos (`CLAUDE.md`, `TANG_MSX_INTERFACE.md`, `SETUP_WINDOWS.md`, `KICKOFF.md`) guardados en una carpeta temporal accesible — los moveremos al repo en la primera sesión.

---

## 3. Estructura objetivo del repo

Así debe quedar el repo tras la primera sesión de Claude Code:

```
mangopl4/
├── CLAUDE.md                       ← contexto permanente para Claude Code
├── README.md                       ← descripción pública del proyecto
├── LICENSE                         ← decidir licencia (probablemente GPL-3.0 por compatibilidad con gtaylormb)
├── .gitignore                      ← Gowin, VS Code, bitstreams intermedios
├── .gitmodules                     ← submódulos (tnCartWonder, opl3_fpga)
├── docs/
│   ├── KICKOFF.md                  ← este documento
│   ├── TANG_MSX_INTERFACE.md       ← capa física + protocolo bus MSX
│   ├── SETUP_WINDOWS.md            ← entorno de desarrollo
│   ├── PROGRESS.md                 ← estado vivo del proyecto (se actualiza cada sesión)
│   └── DECISIONS.md                ← log de decisiones técnicas tomadas durante el proyecto
├── external/                       ← submódulos externos
│   ├── tnCartWonder/               ← git submodule
│   └── opl3_fpga/                  ← git submodule (en Fase 1)
├── rtl/                            ← RTL propio del wrapper MSX + integraciones
│   └── (pendiente, se creará en Fase 1)
├── sim/                            ← testbenches Verilator
│   └── (pendiente, se creará en Fase 1)
└── tools/                          ← scripts auxiliares (Python, bash)
    └── (pendiente)
```

---

## 4. Prompt inicial para Claude Code (copia-pega)

Abrir VS Code → carpeta del proyecto (vacía al principio) → Claude Code (icono lateral) → nueva sesión → pegar este mensaje literal:

---

```
Hola. Soy Jokin Miragaia. Vamos a continuar un proyecto que planifiqué previamente en una sesión web de Claude. El proyecto se llama **MangOPL4** (mango + OPL4). El contexto completo está documentado.

CONTEXTO BREVE: MangOPL4 es la implementación de un OPL3/OPL4 (chip Yamaha YMF278, conocido en MSX como MoonSound) sobre una Tang Nano 20K montada en un cartucho WonderTANG v2.0b, para que funcione como cartucho de sonido en un MSX real. El repo es privado de momento (se abrirá cuando Fase 1 esté validada).

TU PRIMERA TAREA: leer completos y en este orden los siguientes documentos que tengo en una carpeta temporal. Te iré pegando su contenido uno a uno. NO hagas nada más hasta que los hayas leído todos.

1. CLAUDE.md — plan general, decisiones, roadmap
2. docs/TANG_MSX_INTERFACE.md — capa física y protocolo del bus MSX
3. docs/SETUP_WINDOWS.md — mi entorno Windows 11 + WSL2
4. docs/KICKOFF.md — este documento de onboarding

Cuando los hayas leído, confírmame por escrito que entiendes:
- Cuál es el objetivo de la Fase 1 (no el objetivo final, solo la Fase 1).
- Por qué elegimos gtaylormb/opl3_fpga como core y no JTOPL.
- Qué es el multiplexado MSEL y por qué importa para el RTL.
- Cuál es exactamente la primera tarea concreta (Fase 0) que debemos abordar hoy.
- Qué NO debes hacer en esta sesión.

REGLAS DE TRABAJO (las tengo documentadas en CLAUDE.md §9, pero te las resumo):
- Respuestas en español, técnicas, directas, sin relleno.
- Contenido completo de archivos, no snippets parciales.
- Instrucciones paso a paso indicando EN QUÉ TERMINAL (PowerShell Windows o Bash WSL2) se ejecuta cada comando.
- No tengo formación formal de programación; Verilog/SystemVerilog es nuevo para mí. Explica conceptos cuando toquen.
- Respeta los documentos de planificación como fuente de verdad. Si quieres desviarte del plan, argumentalo y pide permiso antes.
- Commits pequeños y con mensajes descriptivos. Nunca commit sin avisar.
- No modifiques los documentos de `docs/` sin permiso explícito (salvo `PROGRESS.md` que se actualiza al final de cada sesión).

Cuando hayas leído todo y confirmado lo anterior, espera instrucciones. No empieces a trabajar aún.
```

---

## 5. Flujo de la primera sesión con Claude Code

Tras pegar el prompt anterior y que Claude Code confirme comprensión:

### Paso 1 — Inicializar el repo

Pedirle:

> Crea la estructura base del repo. Estamos en `C:\wt\mangopl4\`. La estructura esperada es la de §3 de `KICKOFF.md`. Aún no clones submódulos — solo prepara carpetas, archivos vacíos, .gitignore adecuado para Gowin+VSCode+Verilog, y un README.md inicial. Los documentos de planificación los colocaré yo manualmente en sus rutas correctas. Inicializa git local pero no hagas el primer commit todavía.

### Paso 2 — Mover los documentos manualmente

Tú colocas:

- `CLAUDE.md` → raíz del repo
- `TANG_MSX_INTERFACE.md` → `docs/`
- `SETUP_WINDOWS.md` → `docs/`
- `KICKOFF.md` → `docs/`

### Paso 3 — Primer commit y push

Pedirle:

> He colocado los cuatro documentos en sus rutas. Verifica que están bien, revisa el `.gitignore` que creaste, y prepara el primer commit con mensaje `docs: add project planning documents from web session`. Después, enlaza el repo con mi fork en GitHub (que ya he creado como `<usuario>/mangopl4`) y haz push a `main`.

### Paso 4 — Verificar entorno

Pedirle:

> Antes de tocar nada más, vamos a validar que mi entorno Windows + WSL2 está correcto. Ejecuta los checks del §10 de `SETUP_WINDOWS.md` y dime qué pasa con cada uno. Para cada check que falle, diagnóstico y solución.

### Paso 5 — Fase 0: arrancar tnCartWonder stock

Pedirle que siga el roadmap del `CLAUDE.md` §6 Fase 0:

> Empezamos la Fase 0 del roadmap. Añade `herraa1/tnCartWonder` como submódulo git en `external/tnCartWonder/`. Después, guíame paso a paso para abrir el proyecto en Gowin IDE, sintetizar el bitstream stock de WonderTANG 2.0b, y flashearlo. Mi WonderTANG está conectada a un MSX <modelo> con una microSD <tamaño> ya formateada.

### Paso 6 — Primer hito validado

Cuando se confirme que el MSX arranca Nextor → SD → MSX-DOS, pedirle que:

- Cree el primer entry en `docs/PROGRESS.md` dejando constancia del hito.
- Cree `docs/DECISIONS.md` con las decisiones ya tomadas en la planificación (listadas en CLAUDE.md).
- Haga commit con mensaje tipo `feat: validate stock tnCartWonder bitstream on MSX`.

---

## 6. Flujo típico de sesiones posteriores

Como Claude Code no tiene memoria entre sesiones, al abrir una nueva:

### Prompt de re-arranque (más corto que el inicial)

```
Hola. Continuamos el proyecto MangOPL4. Antes de nada:

1. Lee `CLAUDE.md` (raíz) para el contexto general.
2. Lee `docs/PROGRESS.md` para ver dónde lo dejamos.
3. Lee `docs/DECISIONS.md` para recordar las decisiones ya tomadas.
4. Revisa los últimos 5 commits con `git log --oneline -5` para ver qué se hizo en la última sesión.

Cuando hayas hecho eso, confírmame en qué fase/tarea estamos y cuál es el siguiente paso natural según el roadmap. Espera instrucciones antes de empezar.
```

### Final de cada sesión

Antes de cerrar sesión, pedirle:

> Actualiza `docs/PROGRESS.md` con lo que hemos hecho hoy. Si ha habido alguna decisión técnica nueva, añádela también a `docs/DECISIONS.md`. Luego, commit con un mensaje descriptivo que resuma el avance, y push a `main`.

Así, cuando abras la siguiente sesión, el proyecto se "recuerda a sí mismo" vía los archivos.

---

## 7. Qué hacer si Claude Code se desvía del plan

A veces Claude Code intenta "mejorar" o "simplificar" cosas por su cuenta. Si lo hace:

- Recuérdale que el plan está en `CLAUDE.md` y `docs/` y es la fuente de verdad.
- Pídele que **argumente la desviación por escrito** antes de implementarla.
- Si la desviación tiene sentido, **actualiza el documento primero** (CLAUDE.md o DECISIONS.md) y luego ejecuta el cambio.
- Si no tiene sentido, vuelve al plan.

**El orden correcto siempre es: Documentación → Código. Nunca al revés.**

---

## 8. Plantilla inicial de `docs/PROGRESS.md`

Cuando Claude Code cree el archivo por primera vez, debe tener esta estructura:

```markdown
# Progreso del proyecto

Estado vivo del avance. Se actualiza al final de cada sesión de trabajo.

## Resumen ejecutivo

- **Fase actual**: Fase 0 — Preparación del entorno
- **Última sesión**: <fecha>
- **Siguiente hito**: bitstream stock de tnCartWonder flasheado y arrancando Nextor en el MSX

## Hitos completados

- [x] Planificación inicial en sesión web de Claude
- [x] Entorno Windows 11 + WSL2 validado
- [x] Repo creado con documentos de planificación
- [ ] Fork de tnCartWonder como submódulo
- [ ] Bitstream stock sintetizado en Gowin IDE
- [ ] Bitstream flasheado en Tang Nano 20K
- [ ] MSX arranca Nextor desde microSD
- [ ] MSX ejecuta un .com desde microSD

## Log de sesiones

### <fecha> — Sesión 1

- (qué se hizo)
- (problemas encontrados)
- (decisiones tomadas)
- (siguiente paso)
```

---

## 9. Plantilla inicial de `docs/DECISIONS.md`

```markdown
# Decisiones técnicas

Log inmutable de decisiones técnicas tomadas durante el proyecto.
Nueva decisión = nueva entrada al final, con fecha. NO se borra ni se edita contenido anterior.

## D-001 — Codebase base: tnCartWonder

**Fecha**: <fecha de planificación>
**Decisión**: usar `herraa1/tnCartWonder` como base, no el `lfantoniosi/WonderTANG` original.
**Razón**: RTL más limpio y modular, activamente mantenido, incluye V9990 emulado útil para debug.
**Alternativas descartadas**: WonderTANG original (menos modular), partir de cero (absurdo).

## D-002 — Core OPL3: gtaylormb/opl3_fpga

**Fecha**: <fecha de planificación>
**Decisión**: usar `gtaylormb/opl3_fpga` en SystemVerilog.
**Razón**: es la única implementación FPGA pública completa de OPL3. JTOPL no lo tiene aún.
**Alternativas descartadas**: JTOPL2 solo (MSX no usa OPL2 puro), escribir desde cero (inviable).

## D-003 — Entorno de desarrollo: Windows 11 + WSL2

**Fecha**: <fecha de planificación>
**Decisión**: Windows nativo para Gowin/git/flasheo, WSL2 Ubuntu para simulación Verilator.
**Razón**: Gowin IDE corre nativo en Windows; Verilator es mucho más fácil en Ubuntu.
**Alternativas descartadas**: macOS (Gowin corre mal), Linux puro (requeriría VM para herramientas Windows específicas).

## D-004 — Orden de abordaje: Fase 0 → Fase 1 OPL3 → Fase 2 Wavetable

**Fecha**: <fecha de planificación>
**Decisión**: validar arranque stock antes de tocar RTL. Luego OPL3. Wavetable lejano.
**Razón**: sin MSX arrancando no hay banco de pruebas. Sin OPL3 validado no tiene sentido meter wavetable.

## (siguientes se añadirán durante el desarrollo)
```

---

## 10. Consejos operacionales para trabajar con Claude Code

### Terminales en VS Code

- En Windows: VS Code abre PowerShell por defecto.
- Para WSL2: `Ctrl + Shift + P` → "Terminal: Create New Terminal (With Profile)" → elegir "Ubuntu (WSL)".
- **Pista importante para Claude Code**: cuando dé instrucciones, que indique siempre cuál de las dos terminales usar.

### Editar archivos de WSL2 desde VS Code Windows

1. `F1` → "WSL: Connect to WSL".
2. Desde ahí, VS Code opera como si estuviera en Ubuntu. Claude Code funciona igual, pero los comandos ejecutan en bash.
3. Para volver a Windows: `F1` → "Remote: Close Remote Connection".

**Recomendación para este proyecto**: mantener VS Code en **modo Windows** (no WSL) y usar la terminal WSL2 solo para simulación. Así Gowin IDE y openFPGALoader son accesibles desde el mismo VS Code.

### Commits frecuentes

Pedir a Claude Code que haga commit tras cada cambio completo y probado, no tras cada 10 archivos. Mensajes estilo Conventional Commits:

```
feat: add MSX bus decoder wrapper for C4-C7 ports
fix: correct MSEL swap for WonderTANG v2.0b
docs: update PROGRESS.md with session 3 results
test: add Verilator testbench for OPL3 register write
```

### Backups

GitHub ya es backup. Adicionalmente:

- Hacer un push al final de cada sesión (el prompt de cierre ya lo incluye).
- Si algo explota, siempre se puede `git reset --hard origin/main` y recuperar.

---

## 11. Qué hacer si estás atascado

1. **Parar**. No insistir con Claude Code si da vueltas.
2. Abrir una sesión **nueva** del chat web de Claude (claude.ai), pegar `CLAUDE.md`, `PROGRESS.md` y el problema concreto. Pedir consejo de alto nivel.
3. Volver a Claude Code con un **prompt claro y acotado** del paso específico a dar.
4. Si el problema es de hardware (MSX se cuelga, FPGA no flashea, audio con ruido), revisar `TANG_MSX_INTERFACE.md` §17 (gotchas documentados) antes que nada.

---

## 12. Checklist de salida del kickoff

Cuando hayas completado esto, estás listo para trabajar en serio:

- [ ] Los cuatro documentos (`CLAUDE.md`, `TANG_MSX_INTERFACE.md`, `SETUP_WINDOWS.md`, `KICKOFF.md`) están en sus rutas correctas en el repo local.
- [ ] El repo está inicializado, conectado a GitHub, primer commit hecho.
- [ ] Claude Code ha leído los cuatro documentos y confirmado comprensión.
- [ ] `docs/PROGRESS.md` y `docs/DECISIONS.md` inicializados.
- [ ] Entorno Windows + WSL2 validado con el checklist de `SETUP_WINDOWS.md`.
- [ ] WonderTANG conectada al MSX, MSX encendido, funcionando (aunque sea con firmware viejo o cualquier otro cartucho).

A partir de aquí, el `CLAUDE.md` roadmap §6 manda: Fase 0 → Fase 1 → Fase 2.

---

## 13. Un último recordatorio

Este proyecto tiene tres retos simultáneos:

1. **Aprender Verilog/SystemVerilog** desde cero.
2. **Entender el bus MSX** a nivel eléctrico y de protocolo.
3. **Reverse-engineering** de un chip de 1995 para replicarlo en FPGA.

Cualquiera de los tres solo ya es curva dura. Los tres a la vez es ambicioso. **Iterar pequeño, validar pronto, no saltarse la Fase 0**.

Y si te vuelves a enredar, pausa, respira, y recuerda: el MoonSound original se diseñó en los 90 con 128 KB de RAM y un microcontrolador. Lo que intentas hacer es perfectamente posible. Solo hay que hacerlo **una cosa cada vez**.

Buena suerte, buen señor.
