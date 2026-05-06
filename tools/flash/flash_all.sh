#!/usr/bin/env bash
#
# flash_all.sh — flashea bitstream + Nextor + FM BIOS en un solo paso.
#
# El erase del bitstream solo (con o sin --external-flash) en algunas
# versiones de openFPGALoader pisa la región de Nextor (offset 0x100000)
# por redondeo del rango de erase. Por eso este script SIEMPRE re-flashea
# las 3 imágenes en la flash externa de la Tang Nano 20K — garantizado
# consistente, ~30 segundos en total.
#
# Uso:  bash tools/flash/flash_all.sh
#       (desde la raíz del repo frutOPL4)
#
# Si solo se desea flashear el bitstream tras un synth incremental,
# este script hace lo mismo siempre y se evita la trampa del erase.
#
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BITSTREAM="${REPO_ROOT}/external/tnCartWonder/rtl/impl/pnr/tnCart_board_wt200b.fs"
NEXTOR="${REPO_ROOT}/tools/flash/Nextor-2.1.2.MegaFlashSDSCC.1-slot.bin"
FMBIOS="${REPO_ROOT}/tools/flash/fmbios.bin"

NEXTOR_OFFSET=1048576    # 0x100000
FMBIOS_OFFSET=1179648    # 0x120000

if [[ ! -f "${BITSTREAM}" ]]; then
    echo "ERROR: bitstream no encontrado en ${BITSTREAM}" >&2
    echo "       Sintetiza primero: gw_sh.exe tools/flash/synth.tcl" >&2
    exit 1
fi

for f in "${NEXTOR}" "${FMBIOS}"; do
    if [[ ! -f "${f}" ]]; then
        echo "ERROR: ROM no encontrada en ${f}" >&2
        exit 1
    fi
done

echo "==> Flashing bitstream..."
openFPGALoader -f -b tangnano20k --external-flash "${BITSTREAM}"

echo "==> Flashing Nextor (offset 0x100000)..."
openFPGALoader -f -b tangnano20k --external-flash -o "${NEXTOR_OFFSET}" "${NEXTOR}"

echo "==> Flashing FM BIOS (offset 0x120000)..."
openFPGALoader -f -b tangnano20k --external-flash -o "${FMBIOS_OFFSET}" "${FMBIOS}"

echo ""
echo "==> Done. Reset MSX to boot Nextor."
