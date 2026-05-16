# Plan Verilator extendido — sesión 4 (2026-05-10)

## Contexto

Tras 3 días iterando fixes en hardware sin éxito, el bug de bus contention
con cartridge_ram + fetch1 instanciado sigue corrompiendo SDRAM. Necesitamos
ground truth en simulación antes de seguir tocando hardware.

## Dato CRÍTICO a investigar: la pérdida de **EXACTAMENTE 256 KILOBYTES**

Tras `konly.asc` (o pha2c2e2.asc) + reset MSX:
- Antes: 1536 KB libres.
- Después: 1280 KB libres.
- **Pérdida: 256 KB exactos. Consistente entre runs.**

256 KB = 0x40000 bytes = 2^18 bytes = 16 banks × 16 KB.

Mapper RAM = 64 banks × 16 KB = 1024 KB. 256 KB = 1/4 del mapper.

**No me cuadra con corrupción simple por OR-collapse**. Mi análisis dice
que fetch1 firing + cartridge_ram simultáneo OR-corrupta bit 20 (= start_addr_sdram=0x100000).
Eso enviaría writes de Z80 a YRW801 area, NO al mapper. El mapper post-reset
debería estar intacto (BIOS reescribe limpio). Pero pierde 256 KB consistentemente.

**Hipótesis pendientes**:
- BIOS hace algo durante scan que detecta corrupción residual de SDRAM en YRW801.
- bit 4 o 5 del bank register de cartridge_ram se corrompe (= 1/4 del rango bank fail).
- Hay algún test en BIOS que escribe a mapper y lee de YRW801 (= aliasing test).
- Algo más sutil que solo aparecerá en simulación.

Esto debe ser lo PRIMERO que verifiquemos en sim — es el dato más concreto.

## Plan de testbench

Crear `sim/wave_contention_smoke/`:

### Files

```
sim/wave_contention_smoke/
├── Makefile
├── sdram_stub.sv          (refinado del actual)
├── fake_cartridge_ram.sv  (NUEVO)
├── contention_top.sv      (NUEVO)
└── tb_contention.cpp      (NUEVO)
```

### `sdram_stub.sv` (refinado)

Mismo modelo del actual pero:
- Memoria mayor (e.g., 1024 KB para mapper completo).
- Track de TODOS los writes: lista (cycle, addr, byte_data) para post-mortem.
- Track de reads: lista (cycle, addr, byte_returned).

### `fake_cartridge_ram.sv`

Mimics el patrón problemático de cartridge_ram REAL:
- Drive `Ram.OE_n=0` (read) o `Ram.WE_n=0` (write) sostenido cuando `msx_in_mcycle=1`.
- Drive `Ram.ADDR = msx_addr_translated` (con bank registers como en real).
- Drive `Ram.DIN` cuando write.
- Captura `Ram.DOUT[7:0]` cuando read.
- NO chequea `Ram.ACK_n` (= reproduce bug fielmente).
- 6-bit bank registers controllables vía testbench (ports F8-FB).

```systemverilog
module fake_cartridge_ram #(
    parameter [23:0] RAM_ADDR_BASE = 0
) (
    input  wire        CLK,
    input  wire        RESET_n,
    input  wire        bus_reset_n,
    input  wire        msx_in_mcycle,
    input  wire [15:0] msx_addr,
    input  wire [7:0]  msx_din,
    input  wire        msx_is_write,
    output reg  [7:0]  msx_dout,
    // Bank reg control
    input  wire        bank_wr_stb,
    input  wire [1:0]  bank_wr_idx,
    input  wire [5:0]  bank_wr_data,
    RAM_IF.HOST        Ram
);
    reg [5:0] bank [0:3];
    // ...
endmodule
```

### `contention_top.sv`

Top que instancia:
- `ymf278_top` (que internamente tiene mempointer + fetch1 + arbiter, según
  qué versión queramos testar).
- `fake_cartridge_ram` en otro slot.
- `EXPANSION_RAM` REAL de tnCart (USE_FF=0, COUNT=2) merged.
- `sdram_stub`.

Expone como puertos al testbench:
- mempointer control (reg_wr_stb, reg_addr, reg_data, reg_rd_done, key_on, ...).
- fake_cartridge_ram control (msx_in_mcycle, msx_addr, msx_din, ...).
- Debug observables (SDRAM mem, save_addr, etc.).

### `tb_contention.cpp`

#### Helpers
```cpp
struct WriteLog { uint64_t cycle; uint32_t sdram_addr; uint8_t data; };
std::vector<WriteLog> intended_z80_writes;
std::vector<WriteLog> sdram_actual_writes;
```

Después del test, comparar.

#### Scenarios

**S1 (sanity)**: solo wavemem (mempointer fires, no fetch1, no Z80 M-cycles).
PASS esperado.

**S2 (cartridge_ram solo)**: Z80 M-cycles aleatorios, NO fetch1. mempointer
fires intercalado. PASS esperado (sin fetch1, sin contention).

**S3 (REPRODUCIR EL BUG)**:
- key_on=1 (fetch1 fires).
- Z80 simulator drives M-cycles continuos (BASIC FOR loop pattern).
- Pre-rellena mapper con valor conocido (e.g., 0xAA en cada banco).
- Ejecuta unos cuantos miles de Z80 cycles.
- Después: lee mapper, compara con expected.
- Cuenta cuántos bytes están corruptos. Cuántos banks. Pattern.
- **Imprime los addresses corruptos exactos** — buscar pattern bit-level.

**S4 (256KB analysis)**:
- Como S3 pero más extenso.
- Después de la corrupción, simula el flow de BIOS RAM scan: escribir 0x55 / 0xAA en cada banco, leer back, contar banks que pasan.
- Si BIOS scan detecta solo 48 de 64 banks (= 256 KB perdidos), confirmamos
  que reproducimos el síntoma exacto.

**S5 (validate fix)**: aplicar fix candidato, re-correr S3+S4. Esperado PASS.

## Hipótesis a probar en S5 (en orden)

1. **Arbiter `Primary.TIMING` en set condition** (probada en hardware → wavemem regression).
2. **bus_merq_n gate en fetch1 (sin sync)** — vuelta al original sin sync FF.
3. **Sync GLOBAL de Bus.MERQ_n a top level** — usado por TODOS (cartridge_ram, mempointer, fetch1) — requiere modificar tnCart upstream.
4. **WAIT_n driver bien hecho** — cartridge_ram modificado para chequear ACK_n.
5. **Cache BSRAM** — fetch_engine NO accede SDRAM directo. Eliminar contention de raíz.

## Estimación

- ~30 min: Makefile + ajustar sdram_stub.
- ~45 min: fake_cartridge_ram.
- ~45 min: contention_top + plumbing EXPANSION_RAM.
- ~30 min: tb_contention.cpp con S1-S5 framework.
- ~30 min: debug Verilator + S1+S2 PASS.
- ~30 min: S3 reproducir bug (= ground truth).
- ~30 min: S4 confirmar pattern 256KB.
- ~30+ min: iterar hipótesis fixes en S5 hasta PASS.

Total: 4-5 h. Vale la pena vs 3 días tirando bitstreams.

## Estado final esperado

Mañana, al terminar la sesión:
- Sim que reproduce el bug 256 KB exacto.
- Fix validado en sim.
- 1 (uno) synth + flash al final, NO 5+.
- 2c.2.e.2b cerrado de verdad.

## Lecciones

- Verilator setup inicial es trabajo, pero MUY rentable cuando bug es sutil.
- 3 días iterando hardware ≫ 4 horas Verilator + 1 flash.
- "El arte está en los detalles" del usuario fue una llamada a NO seguir
  por la rama equivocada (= seguir tocando hardware sin entender).
