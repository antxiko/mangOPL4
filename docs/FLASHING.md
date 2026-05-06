# Flasheo de bitstream + ROMs en la Tang Nano 20K (WonderTANG 2.0b)

Guía rápida para no caer en la trampa de borrar las ROMs al reflashear el bitstream.

## Qué hay en la flash externa SPI (Winbond W25Q64, 8 MB)

| Offset      | Tamaño aprox | Contenido                                      |
|-------------|--------------|------------------------------------------------|
| `0x000000`  | ~900 KB      | Bitstream Gowin (`tnCart_board_wt200b.fs`)     |
| `0x100000`  | 128 KB       | Nextor 2.1.2 MegaFlashSDSCC.1-slot ROM         |
| `0x120000`  | 64 KB        | FM BIOS (de tnCart upstream)                   |

El espacio entre `0x130000` y el final del chip está libre (~6.8 MB).

## Comandos correctos

**SIEMPRE usar `--external-flash`**. Sin esa flag, openFPGALoader puede borrar regiones más amplias de las esperadas y tirarse las ROMs a 0xFF.

Desde Git Bash o cmd, con la Tang Nano 20K conectada por USB-C (ver sección de drivers más abajo si no aparece):

```bash
# 1. Bitstream (offset implícito 0)
openFPGALoader -f -b tangnano20k --external-flash \
  external/tnCartWonder/rtl/impl/pnr/tnCart_board_wt200b.fs

# 2. Nextor ROM
openFPGALoader -f -b tangnano20k --external-flash \
  -o 1048576 \
  tools/flash/Nextor-2.1.2.MegaFlashSDSCC.1-slot.bin

# 3. FM BIOS
openFPGALoader -f -b tangnano20k --external-flash \
  -o 1179648 \
  tools/flash/fmbios.bin
```

`-o` se da en bytes decimales: `1048576 = 0x100000`, `1179648 = 0x120000`.

## Reflasheo solo del bitstream (durante desarrollo)

Cuando solo cambias RTL y ya tienes Nextor + FM ROM en flash, basta con:

```bash
openFPGALoader -f -b tangnano20k --external-flash \
  external/tnCartWonder/rtl/impl/pnr/tnCart_board_wt200b.fs
```

Esto solo borra los sectores 0–~0xE0000 (los que ocupa la binaria del bitstream, ~900 KB), preservando Nextor (en 0x100000+) y FM BIOS (en 0x120000+).

**Si omites `--external-flash`**, openFPGALoader interpreta otra ruta de programación que en algunas versiones acaba borrando regiones que SÍ contienen tus ROMs. Si te pasa, simplemente reflashea las 3 cosas usando los comandos completos de arriba.

## Drivers (Windows)

Tang Nano 20K es FT2232 (FTDI), `VID_0403:PID_6010`. openFPGALoader necesita la interface 0 mapeada a WinUSB vía Zadig:

1. Conectar la Tang Nano 20K.
2. Abrir Zadig (`https://zadig.akeo.ie/`).
3. Options → List All Devices.
4. Seleccionar el primer "USB Serial Converter A" (interface 0).
5. Driver → WinUSB → Replace Driver.
6. **No tocar la interface 1**: esa es el UART (puerto serie COM8), debe quedar con FTDIBUS para depuración por consola.

Verificar:

```bash
openFPGALoader --detect -b tangnano20k
```

Debe devolver `Detected: Winbond W25Q64 128 sectors size: 64Mb`.

## Síntesis (regenerar el bitstream)

Desde la raíz del repo, en una sesión de Bash o cmd:

```bash
"/c/Gowin/Gowin_V1.9.11.03_Education_x64/IDE/bin/gw_sh.exe" tools/flash/synth.tcl
```

El script abre `external/tnCartWonder/rtl/tnCart_board_wt200b.gprj`, ejecuta `run all` y genera `tnCart_board_wt200b.fs` en `external/tnCartWonder/rtl/impl/pnr/`. Ignora el "invalid command name close_project" final — es un fallo cosmético del .tcl pero el bitstream ya está generado para entonces.

## Recuperación si las ROMs se borran

Tienes copia local de las dos ROMs en:

- `tools/flash/Nextor-2.1.2.MegaFlashSDSCC.1-slot.bin`
- `tools/flash/fmbios.bin`

Reflashea con los comandos 2 y 3 de arriba. Tras los 3 flasheos, el MSX debería arrancar Nextor desde la microSD.
