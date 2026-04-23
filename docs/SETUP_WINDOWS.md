# Setup del entorno de desarrollo — Windows 11 + AMD Ryzen

Documento de instalación paso a paso, partiendo de **Windows 11 recién instalado, nada más**.
Orientado al proyecto OPL3/OPL4 sobre WonderTANG, pero gran parte es reutilizable para cualquier proyecto FPGA Tang Nano + MSX.

---

## 0. Filosofía del setup

- **Windows nativo** para: edición, git, síntesis Gowin, flasheo de la Tang Nano, emuladores MSX.
- **WSL2 (Ubuntu)** para: simulación RTL con Verilator, scripts Bash, comparación de WAVs.
- **VS Code + Claude Code** como orquestador de todo (el mismo VS Code abre código de Windows y de WSL2).
- Todo lo que se pueda, gratuito y open source. Gowin IDE y el compilador de Yamaha son excepciones porque no hay alternativa.

**Orden recomendado** (hay dependencias entre pasos):

1. Windows básicos (PowerShell moderno, Terminal, Git).
2. WSL2 + Ubuntu.
3. VS Code + extensiones.
4. Gowin IDE + drivers USB.
5. openFPGALoader.
6. Toolchain de simulación en WSL2 (Verilator, GTKWave, etc.).
7. Software de debug y validación (VGMPlay, openMSX, etc.).
8. (Opcional) KiCad para Fase 3.

---

## 1. Preparación del sistema Windows

### 1.1 Windows Terminal (si no viene ya)

Windows 11 lo trae por defecto. Si no, instalar desde Microsoft Store.

**Verificación**: abrir "Terminal" desde menú inicio. Debe aparecer una terminal con pestañas.

### 1.2 PowerShell 7 (moderno)

El PowerShell 5.1 que viene por defecto está anticuado. Instalar PowerShell 7:

```powershell
# En Windows Terminal (PowerShell 5.1 inicialmente)
winget install --id Microsoft.PowerShell --source winget
```

Tras instalar, abrir Windows Terminal → ajustes → perfil por defecto → "PowerShell" (el nuevo, no "Windows PowerShell").

### 1.3 Git for Windows

Descarga: <https://git-scm.com/download/win>

Durante el instalador:

- **Default editor**: "Use Visual Studio Code as Git's default editor" (aunque VS Code aún no esté instalado, se configurará cuando lo esté).
- **Initial branch name**: "Override the default branch name for new repositories" → `main`.
- **Line ending conversions**: **"Checkout as-is, commit as-is"** (crítico para trabajar con archivos Verilog que están en LF). Esto configura `core.autocrlf=input`.
- **Terminal emulator**: "Use MinTTY" (por defecto).
- **Credential helper**: "Git Credential Manager".

**Configurar identidad global**:

```powershell
# En Windows Terminal (PowerShell)
git config --global user.name "Jokin Miragaia"
git config --global user.email "jokin.miragaia@gmail.com"
git config --global core.autocrlf input
git config --global init.defaultBranch main
```

**Verificación**:

```powershell
git --version
# Debe mostrar algo como: git version 2.45.x.windows.1
```

### 1.4 SSH keys para GitHub

```powershell
# Generar clave
ssh-keygen -t ed25519 -C "jokin.miragaia@gmail.com"
# Pulsar Enter para aceptar ruta por defecto, poner passphrase si se quiere.

# Arrancar agente SSH automáticamente
Get-Service -Name ssh-agent | Set-Service -StartupType Manual
Start-Service ssh-agent
ssh-add $env:USERPROFILE\.ssh\id_ed25519

# Copiar clave pública al portapapeles
Get-Content $env:USERPROFILE\.ssh\id_ed25519.pub | clip
```

Pegar en GitHub → Settings → SSH and GPG keys → New SSH key.

**Verificación**:

```powershell
ssh -T git@github.com
# Debe decir: "Hi <usuario>! You've successfully authenticated..."
```

---

## 2. WSL2 con Ubuntu 24.04

### 2.1 Habilitar WSL2

```powershell
# En PowerShell ABIERTA COMO ADMINISTRADOR
wsl --install -d Ubuntu-24.04
```

Esto hace: habilitar feature de Windows "Subsistema de Windows para Linux", habilitar "Plataforma de máquina virtual", instalar kernel WSL2, descargar e instalar Ubuntu 24.04.

**Reiniciar el PC cuando lo pida.**

Tras reiniciar, se abrirá automáticamente Ubuntu pidiendo crear usuario y password. Elegir:

- Usuario: `jokin` (o el que prefieras).
- Password: el que quieras (se pedirá para `sudo`).

### 2.2 Actualizar Ubuntu

```bash
# En terminal WSL2 Ubuntu
sudo apt update && sudo apt upgrade -y
```

### 2.3 Instalar paquetes base en Ubuntu

```bash
# En terminal WSL2 Ubuntu
sudo apt install -y \
    build-essential \
    git \
    curl \
    wget \
    unzip \
    python3 \
    python3-pip \
    python3-venv \
    ca-certificates \
    gnupg \
    lsb-release
```

### 2.4 Configurar Git en WSL2 con la misma identidad

```bash
# En terminal WSL2 Ubuntu
git config --global user.name "Jokin Miragaia"
git config --global user.email "jokin.miragaia@gmail.com"
git config --global init.defaultBranch main
# OJO: aquí NO core.autocrlf. En Linux siempre LF.
git config --global core.autocrlf input
```

### 2.5 Compartir claves SSH entre Windows y WSL2

```bash
# En terminal WSL2 Ubuntu
mkdir -p ~/.ssh
cp /mnt/c/Users/<tu_usuario_windows>/.ssh/id_ed25519 ~/.ssh/
cp /mnt/c/Users/<tu_usuario_windows>/.ssh/id_ed25519.pub ~/.ssh/
chmod 600 ~/.ssh/id_ed25519
chmod 644 ~/.ssh/id_ed25519.pub
```

**Verificación**:

```bash
ssh -T git@github.com
```

---

## 3. VS Code + extensiones + Claude Code

### 3.1 VS Code

Descarga: <https://code.visualstudio.com/>

Durante el instalador, activar **todas** las casillas opcionales:

- "Add 'Open with Code' action to Windows Explorer file context menu".
- "Add 'Open with Code' action to Windows Explorer directory context menu".
- "Register Code as an editor for supported file types".
- "Add to PATH" (crítico).

### 3.2 Extensiones esenciales

Abrir VS Code → Extensiones (Ctrl+Shift+X) → instalar:

| Extensión | ID | Para qué |
|---|---|---|
| **WSL** | `ms-vscode-remote.remote-wsl` | Editar archivos dentro de WSL2 desde VS Code Windows |
| **Verilog-HDL / SystemVerilog / Bluespec SystemVerilog** | `mshr-h.veriloghdl` | Syntax highlighting y linting de Verilog/SV |
| **TerosHDL** | `teros-technology.teroshdl` | Soporte avanzado HDL: state machines visual, linting avanzado |
| **GitLens** | `eamodio.gitlens` | Historial y blame de git integrados |
| **Markdown Preview Enhanced** | `shd101wyy.markdown-preview-enhanced` | Ver bien los `.md` del proyecto |
| **Better Comments** | `aaron-bond.better-comments` | Comentarios TODO/FIXME destacados en código |
| **Claude Code** | `Anthropic.claude-code` | IA assistant integrado |

### 3.3 Configurar Claude Code

Tras instalar la extensión:

1. Click en el icono de Claude Code en la barra lateral.
2. "Sign in to Claude".
3. Navegador se abre → autenticar con cuenta Anthropic.
4. Volver a VS Code, ya debería estar autenticado.

**Convención importante**: VS Code abre el proyecto desde la carpeta raíz. Si hay un `CLAUDE.md` en la raíz, Claude Code lo lee automáticamente como contexto.

### 3.4 Verificación WSL + VS Code

```bash
# En terminal WSL2 Ubuntu
cd ~
mkdir test-wsl-vscode
cd test-wsl-vscode
code .
```

Esto debe abrir VS Code en modo "WSL" (se ve una barra verde abajo a la izquierda indicando "WSL: Ubuntu-24.04"). Desde ahí, VS Code edita archivos de WSL2 transparentemente.

---

## 4. Gowin IDE (FPGA synthesis)

### 4.1 Registro y descarga

1. Ir a <https://www.gowinsemi.com/en/support/download_eda/>.
2. Crear cuenta (gratuita, requiere correo y teléfono).
3. Descargar **Gowin EDA Educational Edition**. La versión recomendada por tnCart es **V1.9.9.03 Education** (si hay una más reciente también vale, pero probar con esa si hay problemas).
4. Descargar licencia gratuita desde <https://www.gowinsemi.com/en/support/license/>. La educational licence se envía por correo, y se renueva anualmente.

### 4.2 Instalación

Ejecutar el instalador como administrador. **Importante**:

- Instalar en ruta corta sin espacios: `C:\Gowin\Gowin_V1.9.9.03_x64\`.
- NO instalar en `C:\Program Files\...` (da problemas con algunos comandos).
- Durante la instalación, activar la opción de **instalar drivers USB** (FTDI y adicionales). Esto evita tener que instalarlos después.
- Reiniciar el PC tras la instalación.

### 4.3 Licencia

Copiar el archivo `.lic` recibido por correo a una ubicación estable, por ejemplo `C:\Gowin\license\gowin.lic`.

Arrancar Gowin IDE (`gw_ide.exe` o buscar "Gowin FPGA Designer" en inicio):

- Help → Manage License → seleccionar ruta del `.lic`.
- Reiniciar el IDE.

**Verificación**: al arrancar, la barra de título debe mostrar "GOWIN FPGA Designer - Education" sin mensajes de error de licencia.

### 4.4 Drivers USB de la Tang Nano 20K

La Tang Nano 20K lleva un chip **BL616** como USB-to-JTAG/UART. En Windows 11 suele reconocerse automáticamente como dispositivo COM al conectarlo por USB-C, y aparece también como interface JTAG.

**Verificación**:

- Conectar la Tang Nano 20K por USB-C (sin WonderTANG por ahora, solo la Tang sola).
- Abrir Administrador de dispositivos.
- Debe aparecer:
  - Un "USB Serial Device (COM xx)" bajo Puertos (COM y LPT).
  - Un "JTAG Debugger" o similar bajo USB devices.

Si NO aparece el JTAG, instalar Zadig:

1. Descargar Zadig: <https://zadig.akeo.ie/>
2. Options → List All Devices.
3. Localizar el Tang Nano (aparece con VID/PID `0403:6010` o similar).
4. Instalar driver `WinUSB`.
5. Reconectar USB.

---

## 5. openFPGALoader (herramienta de flasheo alternativa)

El programador nativo de Gowin IDE funciona bien en Windows, pero `openFPGALoader` es más rápido, scriptable y no requiere abrir el IDE entero para flashear.

### 5.1 Descarga

Release Windows: <https://github.com/trabucayre/openFPGALoader/releases>

Buscar el `.zip` con el nombre tipo `openFPGALoader-vX.Y.Z-windows.zip`.

### 5.2 Instalación

1. Descomprimir en `C:\tools\openFPGALoader\` (o ruta similar corta).
2. Añadir esa ruta al PATH:

```powershell
# En PowerShell como administrador
[Environment]::SetEnvironmentVariable(
    "Path",
    $env:Path + ";C:\tools\openFPGALoader",
    [EnvironmentVariableTarget]::Machine
)
```

3. **Cerrar y reabrir Windows Terminal** para que recoja el PATH nuevo.

### 5.3 Verificación

```powershell
openFPGALoader --version
# Debe mostrar algo como: openFPGALoader v0.12.1
```

Con la Tang Nano conectada:

```powershell
openFPGALoader --detect -b tangnano20k
# Debe detectar el FPGA GW2AR-18
```

---

## 6. Toolchain de simulación en WSL2

Esta es la pieza crítica del proyecto. Toda la validación del RTL antes de flashear pasa por aquí.

### 6.1 Verilator

```bash
# En terminal WSL2 Ubuntu
sudo apt install -y verilator
verilator --version
# Debe mostrar: Verilator 5.x
```

Ubuntu 24.04 trae Verilator 5.022 que es más que suficiente.

### 6.2 GTKWave (visualizador de waveforms)

```bash
# En terminal WSL2 Ubuntu
sudo apt install -y gtkwave
```

Para abrir desde Windows una waveform generada en WSL2, se puede:

- Instalar GTKWave para Windows también: <http://gtkwave.sourceforge.net/> (hay builds Windows en SourceForge).
- O ejecutarlo desde WSL2 con X server en Windows (`VcXsrv` o el WSLg integrado en Windows 11).

**Windows 11 tiene WSLg** (soporte nativo de GUI Linux) preinstalado, así que:

```bash
gtkwave dump.vcd
```

Desde WSL2 debería abrir GTKWave como si fuera una app nativa de Windows. Si no, instalar VcXsrv.

### 6.3 Icarus Verilog (complemento a Verilator)

Icarus es más simple para testbenches rápidos; Verilator para simulaciones largas con C++ de soporte.

```bash
sudo apt install -y iverilog
iverilog -V
```

### 6.4 Yosys + nextpnr (opcional, síntesis libre)

No son imprescindibles porque usamos Gowin IDE para la síntesis final, pero útiles para experimentación:

```bash
sudo apt install -y yosys
# nextpnr tiene fork específico para Gowin:
# https://github.com/YosysHQ/nextpnr
# Se compila desde source. Dejar para más adelante si hace falta.
```

### 6.5 Python + librerías para análisis

```bash
# En terminal WSL2 Ubuntu
python3 -m venv ~/wondertang-venv
source ~/wondertang-venv/bin/activate
pip install --upgrade pip
pip install numpy scipy matplotlib soundfile cocotb
```

Uso:

- **numpy/scipy/matplotlib**: análisis de WAVs, FFTs para comparar espectros OPL3 emulado vs real.
- **soundfile**: leer/escribir WAVs.
- **cocotb**: co-simulación Verilog con testbenches en Python (alternativa moderna a testbenches Verilog).

Para activar el venv en cada sesión:

```bash
source ~/wondertang-venv/bin/activate
```

O añadirlo al `~/.bashrc` si se va a usar siempre.

### 6.6 Herramientas auxiliares útiles en WSL2

```bash
sudo apt install -y \
    tree \
    htop \
    tmux \
    vim \
    jq \
    ripgrep \
    fd-find
```

---

## 7. Software de validación y debug del proyecto

### 7.1 VGMPlay (generar referencias OPL3)

Reproduce ficheros `.vgm` con precisión de chip, exportando a WAV. Lo vamos a usar como **referencia bit-exacta** contra la que comparar la salida de nuestro RTL simulado.

Descarga Windows: <https://vgmrips.net/packs/player>

Extraer en `C:\tools\vgmplay\`.

**Uso típico**:

```powershell
# En PowerShell, desde la carpeta de VGMPlay
.\vgmplay.exe -w cancion.vgm
# Genera cancion.wav
```

### 7.2 openMSX (emulador MSX con MoonSound emulado)

Permite probar software MSX que usa el OPL3 del MoonSound sin hardware real, y exportar audio a WAV para comparación.

Descarga: <https://openmsx.org/>

Instalar con el paquete `openMSX-catapult` que incluye el GUI.

**Configurar MoonSound**:

1. Abrir openMSX.
2. Añadir una máquina MSX2+ con disco duro virtual.
3. Añadir cartucho: `scc+ moonsound` (algo así).
4. Cargar software de test: Moonblaster FM, Meridian, etc.

### 7.3 blueMSX (alternativa a openMSX)

Emulación MoonSound a veces más precisa que openMSX. Útil como segunda referencia.

Descarga: <https://www.bluemsx.com/>

### 7.4 Audacity (comparar WAVs visualmente)

Para ver espectros, restar waveforms, detectar dónde diverge nuestra emulación de la referencia.

Descarga: <https://www.audacityteam.org/>

**Workflow típico**:

1. Generar WAV con VGMPlay (referencia).
2. Generar WAV con nuestro testbench Verilator (lo que hace nuestro RTL).
3. Abrir ambos en Audacity en pistas separadas.
4. Invertir una pista, sumar ambas = diferencia.
5. Si la diferencia es silencio, bit-exact. Si hay algo, ahí están los bugs.

### 7.5 HxD (hex editor Windows)

Para inspeccionar bitstreams, ROMs de MSX, dumps de PSRAM, etc.

Descarga: <https://mh-nexus.de/en/hxd/>

### 7.6 7-Zip

Para manejar `.tar.gz`, `.bz2`, y otros formatos que los repos retro suelen usar.

Descarga: <https://www.7-zip.org/>

---

## 8. Software para futuras fases

### 8.1 KiCad (para Fase 3 — cartucho propio)

No instalar todavía, pero dejarlo anotado. Cuando llegue el momento:

Descarga: <https://www.kicad.org/download/windows/>

Para la Fase 3 se abrirá el esquemático KiCad de la WonderTANG (carpeta `kicad/` del repo de Antoniosi) como punto de partida del diseño propio.

### 8.2 sigrok + PulseView (analizador lógico software)

Si se consigue un analizador lógico barato (Saleae clon ~20€, o tarjeta FX2LP), sirve para capturar señales reales del slot MSX y comparar contra lo esperado. Muy útil para depurar timings.

Descarga: <https://sigrok.org/wiki/Downloads>

---

## 9. Estructura de carpetas recomendada

### En Windows (para síntesis Gowin)

```
C:\wt\                              ← ruta corta deliberada
  wondertang-opl4\                  ← repo del proyecto
    rtl\                            ← submódulo tnCartWonder / RTL propio
    docs\                           ← CLAUDE.md, TANG_MSX_INTERFACE.md, etc.
    sim\                            ← testbenches (aunque se corran desde WSL2)
    tools\                          ← scripts auxiliares
```

### En WSL2 (para simulación)

WSL2 accede al repo de Windows vía `/mnt/c/wt/wondertang-opl4/`. **No clonar dos veces**.

```bash
# En terminal WSL2 Ubuntu
cd /mnt/c/wt/wondertang-opl4
```

⚠️ **Performance de I/O entre WSL2 y NTFS**: acceder a archivos de `/mnt/c/` desde WSL2 es más lento que a archivos nativos de `~/`. Para simulaciones largas (>1 minuto), puede merecer la pena copiar temporalmente a `~/sim-tmp/` en WSL2, simular, y copiar resultados de vuelta. Pero para el 95% de casos, trabajar desde `/mnt/c/` es suficientemente rápido.

---

## 10. Checklist final de validación del setup

Antes de considerar el entorno listo:

- [ ] `git --version` en PowerShell.
- [ ] `ssh -T git@github.com` autentica correctamente.
- [ ] `wsl --list --verbose` muestra Ubuntu-24.04 en estado Running, versión 2.
- [ ] `verilator --version` en WSL2 Ubuntu devuelve 5.x.
- [ ] `gtkwave --version` en WSL2 Ubuntu responde.
- [ ] VS Code abre y tiene todas las extensiones de §3.2 instaladas.
- [ ] VS Code se conecta a WSL2 y abre archivos de Ubuntu sin problemas.
- [ ] Claude Code está autenticado en VS Code.
- [ ] Gowin IDE arranca sin errores de licencia.
- [ ] Al conectar la Tang Nano 20K (sola, sin WonderTANG), Windows la detecta como dispositivo USB Serial + JTAG.
- [ ] `openFPGALoader --detect -b tangnano20k` detecta el FPGA.
- [ ] VGMPlay arranca en Windows y reproduce un `.vgm` de prueba.
- [ ] openMSX arranca y tiene machine MSX2+ con MoonSound emulado.

Si todo esto funciona, estás listo para clonar el fork de tnCartWonder y empezar la Fase 0 del roadmap.

---

## 11. Troubleshooting común

### "Gowin IDE pide licencia todo el rato"

La educational se renueva anualmente. Al año, repedir licencia en la web de Gowin con el mismo email.

### "openFPGALoader no detecta la Tang Nano"

1. Verificar que NO esté Gowin IDE abierto usando el dispositivo.
2. Desconectar y reconectar el USB.
3. Si persiste, abrir Zadig y reinstalar driver WinUSB.

### "Verilator da errores crípticos al compilar RTL de tnCart"

tnCart usa SystemVerilog. Asegurarse de que Verilator es **versión 5.x** (no 4.x de Ubuntu viejos). Si `apt` da solo 4.x, compilar desde fuente:

```bash
sudo apt install -y autoconf flex bison help2man
git clone https://github.com/verilator/verilator.git
cd verilator
git checkout stable
autoconf
./configure
make -j$(nproc)
sudo make install
```

### "VS Code en WSL2 es lentísimo"

Primera apertura es lenta porque instala el servidor remoto en WSL2. Tras eso, cerrar y volver a abrir. A partir de la segunda vez es rápido.

### "Los finales de línea se cambian a CRLF al hacer commit"

Verificar `git config --global core.autocrlf` → debe ser `input`, no `true`. Si está mal, arreglar:

```powershell
git config --global core.autocrlf input
```

Y en archivos ya rotos:

```bash
# En WSL2, dentro del repo
dos2unix archivo_problemático.v
```

### "openMSX no ve el MoonSound"

La emulación de MoonSound en openMSX requiere tener la ROM YRW801 extraída. openMSX tiene un asistente: Settings → hardware → moonsound → install ROM.

---

## 12. Ruta de aprendizaje complementaria

Como Verilog/SystemVerilog es nuevo, recursos recomendados para ir en paralelo al proyecto:

### Gratuitos online

- **HDLBits**: <https://hdlbits.01xz.net/> — ejercicios interactivos de Verilog, del "hola mundo" al "procesador RISC-V". Imprescindible.
- **NandGame**: <https://nandgame.com/> — conceptos de lógica digital desde cero, muy visual.
- **Lushay Labs Tang Nano tutorials**: <https://learn.lushaylabs.com/category/gowin-fpga/> — específicos de Tang Nano 9K pero aplicables al 20K.
- **Chisel / Chipyard** (más avanzado): generar Verilog desde Scala.

### Libros

- "FPGA Prototyping by SystemVerilog Examples" de Pong P. Chu.
- "Digital Design and Computer Architecture: ARM Edition" de Harris & Harris.

### Específicos del proyecto

- Código del core gtaylormb/opl3_fpga: leer `doc/` de ese repo antes de integrar.
- Código del fmbios.rom de tnCart para entender cómo el BIOS MSX habla con OPLL (análogo a cómo hablaremos con OPL3).

---

## 13. Estimación de espacio en disco

| Componente | Tamaño aprox. |
|---|---|
| Gowin IDE | 3 GB |
| Ubuntu WSL2 + paquetes básicos | 4 GB |
| Verilator, GTKWave, etc. | 1 GB |
| VS Code + extensiones | 500 MB |
| openMSX + blueMSX | 300 MB |
| VGMPlay + WAVs de prueba | 500 MB |
| Repo tnCartWonder + submódulos | 500 MB |
| Builds de síntesis intermedios | 2-5 GB |
| Emuladores ROMs | 200 MB |
| **Total estimado** | **~15 GB** |

Dejar al menos **30 GB libres** en `C:\` para trabajar cómodo.

---

## 14. Recursos útiles de referencia

- Wiki Tang Nano 20K: <https://wiki.sipeed.com/hardware/en/tang/tang-nano-20k/nano-20k.html>
- Documentación Gowin GW2AR: <https://www.gowinsemi.com/en/product/detail/38/>
- Foro MSX Resource Center: <https://www.msx.org/forum>
- Subreddit r/FPGA: útil para dudas generales.
- Discord de MiSTer FPGA: gente que hace cores similares.

---

## 15. Mantenimiento del entorno

- **Gowin IDE**: actualizar cada 6 meses si sale nueva versión estable.
- **Verilator**: Ubuntu lo actualiza por apt, pero para versiones más recientes hay que compilar desde fuente (ver §11).
- **VS Code + extensiones**: auto-update.
- **WSL2**: `wsl --update` periódicamente desde PowerShell.

---

## 16. Resumen en tres frases

1. **Windows nativo** para Gowin, git, edición, flasheo, emuladores.
2. **WSL2 Ubuntu** para Verilator y todo lo que sea simulación y scripting Unix.
3. **VS Code + Claude Code** como pegamento que une los dos mundos y lee los `CLAUDE.md` / `docs/*.md` del proyecto como contexto.

Con esto montado, no toca volver a instalar nada durante meses.
