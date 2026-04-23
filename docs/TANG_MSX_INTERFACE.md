# Interfaz Tang Nano 20K ↔ MSX a través de la WonderTANG

Documento técnico de referencia para el proyecto OPL3/OPL4 sobre WonderTANG.
Cubre la capa física/eléctrica y el protocolo del bus MSX tal y como se ve desde el FPGA.

---

## 1. Propósito de este documento

Cuando se escribe RTL que habla con el bus del MSX desde un FPGA, **el 80% de los bugs crípticos viven en la capa física y de sincronización**, no en la lógica funcional del periférico. Un OPL3 que "casi" funciona, o que funciona en un MSX y en otro se cuelga, casi siempre tiene su problema en alguno de los siguientes sitios:

- Señales asíncronas no sincronizadas correctamente al dominio del FPGA (metaestabilidad).
- Timings de bus del Z80 mal respetados (el cartucho responde demasiado tarde o libera el bus demasiado pronto).
- `BUSDIR_n` mal gestionado (conflicto en el bus de datos del MSX).
- Tri-state mal implementado (FPGA compite con el Z80 por el bus cuando no toca).
- Diferencias de pin mapping entre revisiones de la WonderTANG.
- Problemas eléctricos: niveles de voltaje, fuentes envejecidas, chips falsificados.

Este documento agrupa todo eso.

---

## 2. Topología eléctrica general

Entre el slot MSX y el FPGA del Tang Nano 20K **no hay conexión directa**. La WonderTANG interpone dos buffers 74LVC245 (referencias `U2` y `U4`) que aíslan el bus del MSX del FPGA y hacen de adaptadores de nivel de voltaje:

```
┌──────────┐                                          ┌──────────────┐
│          │  5V TTL        ┌───────────────┐  3.3V  │              │
│  Slot    │◄──────────────►│  74LVC245 x2  │◄──────►│  GW2AR-18    │
│  MSX     │                │  (VCC = 3.3V) │        │  (Tang Nano) │
│          │                └───────────────┘        │              │
└──────────┘                   U2 + U4                └──────────────┘
     ▲                              ▲                        ▲
  5V TTL                     entradas 5V-tolerant       3.3V LVCMOS
     │                              │                   (NO 5V-tolerant)
     └─── LDO 3.3V al resto ───────┘
```

### Niveles de voltaje (críticos)

| Dominio | Voltaje | Familia lógica |
|---|---|---|
| Bus del slot MSX | 5V | TTL |
| Bus detrás del buffer (lado FPGA) | 3.3V | LVCMOS |
| Núcleo del FPGA | 1.2V | (LDO interno del Tang Nano) |

**El GW2AR-18 NO es 5V-tolerante en sus pines de I/O.** Conectar el bus del MSX directo al FPGA lo destruye. Los 74LVC245 son obligatorios.

---

## 3. Los buffers 74LVC245 en detalle

### Por qué 74LVC245 y no otro

- **Input 5V-tolerant aunque VCC sea 3.3V**: esta es la característica crítica. Un 74HC245 o un 74LS245 requieren VCC=5V; si se usan como traductores inyectarían 5V al FPGA.
- **VOH típico a VCC=3.3V ≈ 3.1V**: cuando el FPGA manda información hacia el MSX, los 3.1V de salida son perfectamente interpretables como nivel alto TTL por el MSX (umbral VIH=2.0V).
- **Octal (8 bits)**: por eso hacen falta dos chips para cubrir todo el bus + control.

### Reparto de señales entre U2 y U4

Aproximadamente (el esquemático exacto hay que verificarlo):

- **U2**: señales de control direccionales (CLOCK, IORQ_n, RD_n, WR_n, MREQ_n, SLTSL_n, M1_n, RESET_n) — todas MSX → FPGA.
- **U4**: bus multiplexado A/D (8 bits, bidireccional) + señales FPGA → MSX (BUSDIR_n, INT_n, WAIT_n).

### ⚠️ Falsificaciones en el mercado

Reportes confirmados en el repo oficial de la WonderTANG: **AliExpress y UTSource venden 74LS245 rebrandeados como 74LVC245**. Los síntomas son:

- Funciona unos minutos y luego falla.
- Arranca en algunos MSX y no en otros sin patrón claro.
- Teclado deja de responder (problema típico con falsificados en U2).

**Comprar solo en DigiKey, Mouser, Adafruit, o distribuidores oficiales.** Si se compra en otro sitio, verificar que el marcaje es coherente (logo del fabricante nítido, date code realista, ausencia de decoloración por reballing).

---

## 4. Multiplexado MSEL (truco clave del diseño)

El Tang Nano 20K tiene ~40 pines GPIO accesibles desde la placa WonderTANG. Un cartucho MSX "completo" necesita:

- 16 líneas de bus de direcciones (A0-A15)
- 8 líneas de bus de datos (D0-D7)
- ~8 líneas de control
- = ~32 señales, muy cerca del límite

Para no quedarse sin pines, **la WonderTANG multiplexa A0-A7 y D0-D7 en un mismo bus de 8 bits del FPGA**, controlado por dos señales `MSEL0_33` y `MSEL1_33` que actúan sobre los `OE` y `DIR` de los 74LVC245:

```
                  ┌──── MSEL0_33 ──→ controla U? (dirección A0-A7)
      FPGA ───────┤
                  └──── MSEL1_33 ──→ controla U? (datos D0-D7, bidireccional)
                        │
                        8 bits compartidos del FPGA = BUS_33[7:0]
```

### Implicaciones para el RTL

- **No se puede leer dirección y dato a la vez** desde el lado FPGA. Hay que secuenciar: poner MSEL=01 para leer dirección, luego MSEL=10 para leer/escribir dato.
- Esto introduce latencia controlada por el propio RTL, pero siempre dentro de la ventana de bus del MSX (~560 ns).

### Swap de MSEL entre revisiones

El orden de los bits de `MSEL0/MSEL1` está invertido entre tnCart original (rev1) y las distintas revisiones de la WonderTANG. `tnCartWonder` existe precisamente para swappearlos según la placa destino.

| Placa | Orden MSEL |
|---|---|
| tnCart rev1 (placa original) | MSEL0=addr, MSEL1=data |
| WonderTANG v1.01c | invertido respecto a tnCart rev1 |
| WonderTANG v1.02d | invertido respecto a tnCart rev1 |
| WonderTANG v2.0b | invertido + cambios adicionales de pin mapping |

Si se toca el RTL pensando que `MSEL0` controla el bus de datos cuando en realidad controla el de direcciones en esa placa concreta, se pasan horas depurando fantasmas. **Verificar siempre el swap contra el esquemático de la placa física.**

---

## 5. Señales de control (directas, sin multiplexar)

Estas cada una va a su pin propio del Tang Nano vía U2:

| Señal MSX | Dirección | Nombre RTL | Función |
|---|---|---|---|
| `CLOCK` (3.579545 MHz) | MSX → FPGA | `CLOCK_33` | Reloj del Z80, referencia de timing |
| `IORQ_n` | MSX → FPGA | `IORQ_33` | I/O request activo (bajo) |
| `RD_n` | MSX → FPGA | `RD_33` | Read strobe |
| `WR_n` | MSX → FPGA | `WR_33` | Write strobe |
| `MREQ_n` | MSX → FPGA | `MREQ_33` | Memory request (crítico para MegaRAM/SCC+; no usado por OPL3) |
| `SLTSL_n` | MSX → FPGA | `SLTSL_33` | Slot select (no usado por OPL3, usado por MegaRAM/SCC+) |
| `M1_n` | MSX → FPGA | `M1_33` | Ciclo 1 del Z80 (útil para distinguir interrupt ack) |
| `RESET_n` | MSX → FPGA | `RESET_33` | Reset global |
| `BUSDIR_n` | FPGA → MSX | `BUSDIR_33` | Dirección del transceiver del bus del MSX |
| `INT_n` | FPGA → MSX | `INT_33` | Interrupción al Z80 (OJO: bug conocido — ver §7) |
| `WAIT_n` | FPGA → MSX | `WAIT_33` | Wait states (rara vez necesario) |

### Señales NO necesarias para un cartucho I/O puro como el OPL3

- `A8-A15`: en un ciclo I/O, el Z80 pone el contenido del acumulador en `A8-A15`. Los cartuchos I/O estándar ignoran estos bits.
- Líneas `CS1/CS2/CS12`: solo para accesos de memoria paginada.
- `RFSH_n`: refresh de DRAM.

Para la parte MegaRAM/SCC+ que ya trae tnCartWonder, sí se usan `A8-A15`, `MREQ_n`, `SLTSL_n`, `CS*`. Pero nuestro módulo OPL3 no toca nada de eso.

---

## 6. Ciclo de bus Z80 para I/O

El Z80 usa `IN A,(n)` y `OUT (n),A` para I/O. Ambas instrucciones ocupan un ciclo de bus de **4 ciclos T + 1 wait state automático**. A 3.579545 MHz, cada ciclo T dura **279.365 ns**.

### Escritura (OUT (puerto), A)

```
Ciclo T:      T1       T2       TW       T3
CLOCK:     ___|‾‾|____|‾‾|____|‾‾|____|‾‾|___

A0-A7:     ====< puerto válido                      >====
D0-D7:     ========< dato Z80 hacia cartucho        >===
IORQ_n:    ‾‾‾‾‾‾‾‾‾‾‾‾\_________________/‾‾‾‾‾‾
WR_n:      ‾‾‾‾‾‾‾‾‾‾‾‾\_________________/‾‾‾‾‾‾
```

Cronología:

- **T1**: dirección del puerto en `A0-A7`.
- **Mitad T1→T2**: bajan `IORQ_n` y `WR_n`.
- **T2**: dato válido en `D0-D7`.
- **TW**: wait state automático (el Z80 lo inserta siempre en I/O).
- **Flanco de subida entre TW y T3**: `IORQ_n` y `WR_n` suben. El dato debe estar capturado antes de ese flanco.

**Ventana útil para capturar el dato**: ~2 ciclos T ≈ 560 ns.

### Lectura (IN A, (puerto))

Mismo ciclo, pero:

- `RD_n` baja en vez de `WR_n`.
- El cartucho debe activar `BUSDIR_n = 0` para que el transceiver del MSX se oriente hacia el Z80.
- El dato debe estar estable en `D0-D7` antes del flanco de subida de `RD_n`.

**Ventana útil para responder**: ~560 ns.

### Wait states

Si el FPGA necesita más tiempo, puede bajar `WAIT_n` y el Z80 inserta wait states adicionales hasta que se libere. **Para OPL3 no hace falta** — el core responde en uno o dos ciclos del reloj interno del FPGA (30 ns cada uno a 33.8688 MHz), muy por debajo de los 560 ns disponibles.

---

## 7. 🐛 Bug eléctrico conocido: `INT_n` push-pull vs open-collector

El estándar del slot MSX especifica que `INT_n` es una línea **open-collector con pull-up en la placa madre** (porque es compartida entre todos los cartuchos). El diseño de la WonderTANG cablea `INT_n` como **salida push-pull del 74LVC245**. Issue #16 del repo oficial lo documenta.

### Consecuencia

- En máquinas como el Philips VG8235, cuando otro cartucho (o el propio MSX) quiere poner `INT_n=1`, no puede, porque el push-pull de la WonderTANG fuerza `INT_n=0` continuamente. El MSX se cuelga al arrancar.
- En la mayoría de MSX, esto no da problema porque el pull-up es fuerte y el MSX no depende tanto de esa línea al arranque. Pero es una bomba de tiempo.

### Impacto en nuestro proyecto OPL3

El OPL3 tiene **dos timers** que pueden generar interrupciones. Opciones:

1. **No habilitar los timers del OPL3**. Muchos drivers de MoonSound hacen polling de status en `C4h` en vez de usar INT.
2. **Habilitar timers pero no conectar la salida al `INT_n` del slot**. El software puede leer el status polling.
3. **Si hace falta INT físico**: modificación de placa (transistor NPN en open-collector externo), documentada como opcional.

### Fixes en RTL según revisión de tnCartWonder

- v1.01c: tnCartWonder invierte la lógica de INT porque la placa lo requiere.
- v1.02d: tnCartWonder NO invierte.
- v2.0b: sigue siendo push-pull por diseño de placa; tnCart/tnCartWonder lo gestiona manteniendo `INT_33=1` siempre que ningún periférico interno lo asserta.

**Recomendación para este proyecto**: desactivar por defecto la generación de INT desde el OPL3 y dejarlo como opción configurable.

---

## 8. `BUSDIR_n` — la señal que todos olvidan

El MSX tiene un **transceiver bidireccional** en el bus de datos del slot. Por defecto, los datos fluyen MSX → cartucho. Si el cartucho quiere meter datos en una lectura, tiene que decirle al transceiver del MSX que invierta la dirección.

Esa señal es `BUSDIR_n`, sale del cartucho hacia el MSX, activa a bajo.

### Regla

```
BUSDIR_n = 0  SOLO cuando:
              IORQ_n = 0  (o MREQ_n según caso)
            & RD_n = 0
            & puerto/dirección direccionada a nosotros

En el resto de casos, BUSDIR_n = 1 (alta) o en alta impedancia con pull-up.
```

### Consecuencias de equivocarse

- **BUSDIR_n=0 cuando no toca**: conflicto en el bus de datos del MSX. Dos salidas a la vez, una ganará y la otra se calentará. Riesgo de daño al cartucho, al MSX o a ambos.
- **BUSDIR_n=1 cuando sí toca**: el MSX no lee nunca el dato del cartucho, interpretará `FFh` o basura.

---

## 9. Salida tri-state del bus de datos

El bus de datos `D[7:0]` es bidireccional. El FPGA debe poner los pines en **alta impedancia** el resto del tiempo, y solo conducir cuando responde a una lectura propia.

```verilog
// Pseudocódigo
assign BUS_33 = (mi_acceso_de_lectura) ? dato_a_devolver : 8'bz;
```

En FPGAs, el tri-state solo funciona en los buffers de I/O reales (pines físicos del chip), no en lógica interna. Gowin IDE lo gestiona bien si el puerto del módulo top-level está declarado como `inout`.

En la práctica, como el bus está detrás del 74LVC245, el tri-state real lo controla la señal `OE` del buffer. El FPGA maneja esa `OE` según el estado del acceso.

---

## 10. Sincronización al dominio del FPGA (clock domain crossing)

Este es el punto más delicado para evitar **metaestabilidad**.

### El problema

- El MSX corre a 3.579545 MHz.
- El FPGA del proyecto corre a 27 MHz (cristal onboard del Tang Nano), 33.8688 MHz (PLL para OPL4), o lo que se genere. Totalmente asíncrono respecto al MSX.
- Cuando el FPGA muestrea una señal del MSX en el flanco del propio reloj, si esa señal está cambiando justo en ese instante, el flip-flop puede entrar en **metaestabilidad**: un estado intermedio entre 0 y 1 que puede tardar tiempo indeterminado en resolverse.
- El síntoma típico es "funciona 99 de cada 100 veces, a veces el MSX se cuelga o se lee basura".

### Solución: sincronizadores de 2 flip-flops

Cada señal de entrada asíncrona (CLOCK, IORQ_n, RD_n, WR_n, MREQ_n, SLTSL_n, BUS_33 cuando viene del MSX, etc.) debe pasar por **al menos 2 flip-flops en cascada clockeados con el reloj del FPGA** antes de usarse en lógica.

```verilog
// Sincronizador típico de 2 etapas
reg [1:0] iorq_sync;
always @(posedge clk_fpga) begin
    iorq_sync <= {iorq_sync[0], iorq_33_pin};
end
wire iorq_n_synced = iorq_sync[1];
```

### Detección de flancos

Cuando se necesita reaccionar a un flanco (por ejemplo, "capturar dato cuando cae `WR_n`"):

```verilog
reg iorq_n_prev;
always @(posedge clk_fpga) iorq_n_prev <= iorq_n_synced;
wire iorq_falling = iorq_n_prev & ~iorq_n_synced;
wire iorq_rising  = ~iorq_n_prev & iorq_n_synced;
```

Para un ciclo I/O del MSX, como las señales se mantienen estables durante toda la ventana (~560 ns = muchos ciclos del FPGA), normalmente basta con ver **niveles sincronizados** sin detectar flancos estrictamente.

---

## 11. Decodificación de puertos C4-C7 (OPL3 del MoonSound)

La lógica de detección de nuestro módulo OPL3:

```
acceso_OPL3 = IORQ_n_synced == 0
           && M1_n_synced == 1           // excluye interrupt ack
           && (A[7:2] == 6'b110001)      // C4h-C7h = 1100 01xx

escribir_OPL3 = acceso_OPL3 && WR_n_synced == 0
leer_OPL3     = acceso_OPL3 && RD_n_synced == 0
```

Los dos bits bajos seleccionan cuál de los cuatro puertos:

| A[1:0] | Puerto | Función OPL3 |
|---|---|---|
| `00` | C4h | Register select banco 1 (y status read en lectura) |
| `01` | C5h | Data banco 1 |
| `10` | C6h | Register select banco 2 |
| `11` | C7h | Data banco 2 |

---

## 12. Secuencia de escritura a registro OPL3

El OPL3 usa modelo **address + data**, no se escribe un registro en un solo paso:

```
OUT (C4h), reg_num    ; selecciona registro del banco 1
OUT (C5h), valor      ; escribe valor en el registro seleccionado
```

O para el banco 2:

```
OUT (C6h), reg_num
OUT (C7h), valor
```

### Timing interno del YMF262

Tras un write, el chip real requiere cierto tiempo antes de aceptar el siguiente acceso:

- Tras register select: esperar **32 ciclos del reloj maestro** ≈ 945 ns a 33.8688 MHz.
- Tras data write: esperar **3 ciclos del reloj maestro** ≈ 88 ns (para registros normales; algunos piden hasta 84 ciclos).

Los drivers MSX (Moonblaster, Meridian, VGMPlay) respetan estos delays insertando NOPs del Z80. El core `gtaylormb/opl3_fpga` implementa internamente este timing, así que **nuestro wrapper no tiene que preocuparse de añadir delays propios** — solo hacer pasar la escritura directamente al core.

---

## 13. Lectura de status (IN A, (C4h))

Formato del byte de status del OPL3:

```
   Bit:    7    6    5    4    3    2    1    0
          IRQ  T1   T2   0    0    0    0    0
```

- **Bit 7 (IRQ)**: algún timer ha overflowed.
- **Bit 6 (T1)**: Timer 1 overflow.
- **Bit 5 (T2)**: Timer 2 overflow.
- **Bits 4-0**: 0.

### Secuencia en RTL

Cuando se detecta `IORQ_n=0 && RD_n=0 && A[7:0]=C4h`:

1. Activar `BUSDIR_n = 0` (orientar transceiver del MSX hacia el Z80).
2. Poner el byte de status en `BUS_33[7:0]`.
3. Mantener hasta que suba `RD_n`.
4. Cuando sube `RD_n`: devolver `BUS_33` a alta impedancia y `BUSDIR_n = 1`.

Con reloj FPGA a 33.8688 MHz (ciclo 30 ns), las tres acciones se hacen en <100 ns. Margen sobrado sobre los 560 ns de ventana.

---

## 14. Audio: caminos distintos según revisión de la WonderTANG

### WonderTANG v1.02d y anteriores

- El FPGA sintetiza un **DAC de 1-bit (delta-sigma modulator)** en RTL.
- Salida por filtro RC pasivo → jack 3.5 mm o `SOUNDIN` del MSX.
- tnCart rev1 trae `pwm_dac.v` (o similar) para esto.

### WonderTANG v2.0b (objetivo del proyecto)

- El Tang Nano 20K tiene un chip **MAX98357A** soldado — amplificador I2S Class-D con DAC integrado.
- La WonderTANG 2.0b expone los pines del MAX98357A vía el conector JST `J3`, pins soldados directamente al Tang Nano entre los rótulos "SiPEED" y "TANG NANO 20K".
- La salida del MAX98357A va tanto al jack estéreo de la placa como al pin `SOUNDIN` del slot MSX.

### I2S a generar desde RTL

El MAX98357A espera una señal I2S estándar:

- **BCLK** (bit clock): típicamente 32× o 64× LRCLK.
- **LRCLK** (left/right clock): sample rate (= Fs).
- **DIN** (data): PCM 16 o 24 bits, MSB first, cambio en flanco de bajada de BCLK.

El sample rate útil para el OPL3/OPL4 es **49.716 kHz** (33.8688 MHz / 680, o 44.1 kHz tras resampling). El MAX98357A acepta cualquier Fs entre 8 y 96 kHz.

tnCart ya trae un módulo `i2s_transmitter.v` para esto. **Lo reutilizamos**: el PCM estéreo 16-bit que escupe el core gtaylormb se enruta a la entrada del I2S transmitter existente.

### Mezcla con otros periféricos internos

tnCart mezcla internamente las salidas de SCC+, OPLL, PSG secundario, etc., antes de pasar al I2S transmitter. Nuestra salida OPL3 se suma a ese mixer.

**Cuidado con el clipping**: al añadir un canal más al mixer, hay que dejar margen (o bajar el nivel global de todos). El OPL3 puede dar hasta 16 bits con picos altos, sumado a los demás = riesgo de saturación.

### Reporte documentado

Usuarios de WonderTANG 2.0b reportan que el audio vía MAX98357A tiene más ruido que la versión 1-bit DAC de la v1.02d, porque el amplificador Class-D amplifica también las interferencias internas del Tang Nano. Es limitación de placa, no hay solución RTL — se asume.

---

## 15. Alimentación

```
Slot MSX 5V ──→ [LDO AMS1117-3.3] ──→ 3.3V para:
                                       - 74LVC245 (U2, U4)
                                       - Tang Nano 20K (entrada 3.3V)
                                          └─→ LDO interno 1.2V → core FPGA
```

### Gotcha documentado: MSX con fuentes envejecidas

- Dropout típico del AMS1117 ≈ 1.2V.
- MSX con fuente vieja que solo entrega 4.7V: salida del LDO ≈ 3.5V, en el límite.
- MSX con fuente aún peor (4.5V): LDO fuera de regulación, salida baja a ~3.2V o menos.
- Consecuencia: `Fmax` del FPGA baja, aparecen glitches aleatorios, timing reports mienten.

**Antes de depurar cualquier cosa rara en hardware real: medir con multímetro la tensión de 5V del slot y los 3.3V en el pin del Tang Nano.**

---

## 16. Pin mapping FPGA ↔ señal (WonderTANG 2.0b)

El archivo de constraints físicos de Gowin es `rtl/src/tnCart_board_wt200b.cst` en el repo `herraa1/tnCartWonder`. Estructura conceptual:

```
IO_LOC "CLOCK_33"      <pinNN>;   IO_PORT "CLOCK_33"      PULL_MODE=UP;
IO_LOC "IORQ_33"       <pinNN>;   IO_PORT "IORQ_33"       PULL_MODE=UP;
IO_LOC "RD_33"         <pinNN>;   IO_PORT "RD_33"         PULL_MODE=UP;
IO_LOC "WR_33"         <pinNN>;   IO_PORT "WR_33"         PULL_MODE=UP;
IO_LOC "MREQ_33"       <pinNN>;   IO_PORT "MREQ_33"       PULL_MODE=UP;
IO_LOC "SLTSL_33"      <pinNN>;   IO_PORT "SLTSL_33"      PULL_MODE=UP;
IO_LOC "M1_33"         <pinNN>;   IO_PORT "M1_33"         PULL_MODE=UP;
IO_LOC "RESET_33"      <pinNN>;   IO_PORT "RESET_33"      PULL_MODE=UP;

IO_LOC "MSEL0_33"      <pinNN>;
IO_LOC "MSEL1_33"      <pinNN>;

IO_LOC "BUS_33[0]"     <pinNN>;
IO_LOC "BUS_33[1]"     <pinNN>;
...
IO_LOC "BUS_33[7]"     <pinNN>;

IO_LOC "BUSDIR_33"     <pinNN>;
IO_LOC "INT_33"        <pinNN>;
IO_LOC "WAIT_33"       <pinNN>;

// Audio (v2.0b, I2S al MAX98357A del Tang Nano)
IO_LOC "I2S_BCLK"      <pinNN>;
IO_LOC "I2S_LRCLK"     <pinNN>;
IO_LOC "I2S_DIN"       <pinNN>;

// SD card
IO_LOC "SD_CLK"        <pinNN>;
IO_LOC "SD_CMD"        <pinNN>;
IO_LOC "SD_DAT[0..3]"  <pinNN>;
```

Los números de pin exactos hay que leerlos del `.cst` real cuando se clone el repo. No los incluyo aquí porque cualquier copia de memoria es fuente de errores — el `.cst` es la fuente de verdad.

El `.cst` real de wt200b está anexado al final de este documento como **Apéndice A** (commit `d3a54f1` de `herraa1/tnCartWonder`). El **Apéndice B** documenta las diferencias entre la descripción conceptual de este §16 y la realidad de wt200b — los nombres de señales, el número de chips multiplexados y los pines de audio han cambiado respecto al esquema original de tnCart rev1 sobre el que se redactó esta sección.

---

## 17. Gotchas documentados (resumen)

| # | Síntoma | Causa | Solución |
|---|---|---|---|
| 1 | Teclado deja de responder en MSX japoneses | 74LVC245 falsificado en U2 | Sustituir por 74LVC245 genuino o 74LS245 como workaround |
| 2 | MSX Philips VG8235 se cuelga al arrancar | `INT_n` push-pull vs open-collector | No usar INT del OPL3, o mod de placa con NPN |
| 3 | Fallos aleatorios en MSX antiguos | Fuente envejecida da <5V | Medir y sustituir fuente |
| 4 | Ruido alto en audio de v2.0b | MAX98357A amplifica interferencias internas del Tang Nano | Limitación de placa, no hay fix RTL |
| 5 | Artefactos gráficos del V9990 emulado en algunos MSX | Timing del V9990 ajustado al límite | Reajustar constraints en Gowin, probar Fmax |
| 6 | WonderTANG no se detecta al primer arranque | Problema conocido con algunas máquinas | Un segundo reset desde el MSX |

---

## 18. Checklist de primer día en hardware

Cuando se tenga el fork clonado, la WonderTANG en mano y el MSX conectado:

- [ ] Medir con multímetro la tensión de 5V del slot MSX (debe ser 4.9-5.1V).
- [ ] Medir 3.3V en el pin VCC del Tang Nano (debe ser 3.25-3.35V).
- [ ] Verificar marcaje de U2 y U4: deben decir `74LVC245` con logo de fabricante legítimo.
- [ ] Fotografiar el esquemático y anotar referencias reales (por si hay variaciones).
- [ ] Flashear bitstream stock de tnCartWonder y confirmar arranque Nextor → SD → MSX-DOS.
- [ ] Abrir `tnCart_board_wt200b.cst` y anexarlo completo al §16 de este documento.

---

## 19. Referencias

- Repo WonderTANG (esquemático KiCad, BOM, issues): <https://github.com/lfantoniosi/WonderTANG>
- Repo tnCartWonder (fork con `.cst` para wt200b, wt102d, wt101c): <https://github.com/herraa1/tnCartWonder>
- Repo tnCart upstream: <https://github.com/buppu3/tnCart>
- Issue #16 (bug nINT push-pull): <https://github.com/lfantoniosi/WonderTANG/issues/16>
- Issue #10 tnCart (compatibilidad WonderTANG 1.02d, swap MSEL): <https://github.com/buppu3/tnCart/issues/10>
- Datasheet 74LVC245 (Nexperia): buscar PDF oficial.
- Datasheet MAX98357A (Analog Devices): <https://www.analog.com/media/en/technical-documentation/data-sheets/max98357a-max98357b.pdf>
- Wiki Tang Nano 20K (pin mapping, PLLs): <https://wiki.sipeed.com/hardware/en/tang/tang-nano-20k/nano-20k.html>
- Manual técnico MSX (ciclos de bus Z80, pinout del slot): MSX Technical Handbook.
- Manual YMF262 (OPL3): timings internos, registros.
- Manual YMF278B (OPL4): mapeo completo incluyendo wavetable (Fase 2).

---

## 20. Apéndice A — `board_wt200b.cst` real

Anexado del submódulo `external/tnCartWonder` commit `d3a54f1` (heads/main), archivo `rtl/src/board/wt200b/board_wt200b.cst`. Fuente de verdad para el pin mapping físico de WonderTANG v2.0b. Si en el futuro se actualiza el submódulo, refrescar este apéndice.

```cst
// TangNano20k clock
IO_LOC "CLK_27M" 4;
IO_PORT "CLK_27M" IO_TYPE=LVCMOS33 PULL_MODE=UP BANK_VCCIO=3.3;

// TangNano20k TF (microSD)
IO_LOC "TF_CMD" 82;
IO_PORT "TF_CMD" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "TF_DAT3" 81;
IO_PORT "TF_DAT3" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "TF_DAT2" 80;
IO_PORT "TF_DAT2" IO_TYPE=LVCMOS33 PULL_MODE=UP BANK_VCCIO=3.3;
IO_LOC "TF_DAT1" 85;
IO_PORT "TF_DAT1" IO_TYPE=LVCMOS33 PULL_MODE=UP BANK_VCCIO=3.3;
IO_LOC "TF_DAT0" 84;
IO_PORT "TF_DAT0" IO_TYPE=LVCMOS33 PULL_MODE=UP BANK_VCCIO=3.3;
IO_LOC "TF_SCLK" 83;
IO_PORT "TF_SCLK" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;

// TangNano20k TMDS (HDMI, salida del V9990 emulado)
IO_LOC "tmds_clk_p" 33,34;
IO_PORT "tmds_clk_p" PULL_MODE=NONE DRIVE=3.5;
IO_LOC "tmds_data_p[0]" 35,36;
IO_PORT "tmds_data_p[0]" PULL_MODE=NONE DRIVE=3.5;
IO_LOC "tmds_data_p[1]" 37,38;
IO_PORT "tmds_data_p[1]" PULL_MODE=NONE DRIVE=3.5;
IO_LOC "tmds_data_p[2]" 39,40;
IO_PORT "tmds_data_p[2]" PULL_MODE=NONE DRIVE=3.5;

// TangNano20k FLASH (SPI interna, almacena ROM Nextor sin microSD)
IO_LOC "mspi_sclk" 59;
IO_PORT "mspi_sclk" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "mspi_cs" 60;
IO_PORT "mspi_cs" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "mspi_miso" 62;
IO_PORT "mspi_miso" IO_TYPE=LVCMOS33 PULL_MODE=UP BANK_VCCIO=3.3;
IO_LOC "mspi_mosi" 61;
IO_PORT "mspi_mosi" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "mspi_hold" 63;
IO_PORT "mspi_hold" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;

// UART
IO_LOC "UART_RX" 70;
IO_LOC "UART_TX" 69;
IO_PORT "UART_RX" IO_TYPE=LVCMOS33;
IO_PORT "UART_TX" IO_TYPE=LVCMOS33;

// access LED
IO_LOC "LED" 75;
IO_PORT "LED" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;

// cartridge signal
IO_LOC "CART_DATA_DIR" 52;
IO_PORT "CART_DATA_DIR" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "CART_MUX_CS_n[2]" 19;
IO_PORT "CART_MUX_CS_n[2]" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "CART_MUX_CS_n[1]" 17;
IO_PORT "CART_MUX_CS_n[1]" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "CART_MUX_CS_n[0]" 20;
IO_PORT "CART_MUX_CS_n[0]" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "CART_WAIT_n" 42;
IO_PORT "CART_WAIT_n" IO_TYPE=LVCMOS33 PULL_MODE=NONE DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "CART_INT_n" 73;
IO_PORT "CART_INT_n" IO_TYPE=LVCMOS33 PULL_MODE=NONE DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "CART_BUSDIR_n" 74;
IO_PORT "CART_BUSDIR_n" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "CART_DATA_SIG[7]" 49;
IO_PORT "CART_DATA_SIG[7]" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "CART_DATA_SIG[6]" 53;
IO_PORT "CART_DATA_SIG[6]" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "CART_DATA_SIG[5]" 71;
IO_PORT "CART_DATA_SIG[5]" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "CART_DATA_SIG[4]" 72;
IO_PORT "CART_DATA_SIG[4]" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "CART_DATA_SIG[3]" 79;
IO_PORT "CART_DATA_SIG[3]" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "CART_DATA_SIG[2]" 86;
IO_PORT "CART_DATA_SIG[2]" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "CART_DATA_SIG[1]" 41;
IO_PORT "CART_DATA_SIG[1]" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "CART_DATA_SIG[0]" 48;
IO_PORT "CART_DATA_SIG[0]" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "CART_MUX_SIG[7]" 31;
IO_PORT "CART_MUX_SIG[7]" IO_TYPE=LVCMOS33 PULL_MODE=UP BANK_VCCIO=3.3;
IO_LOC "CART_MUX_SIG[6]" 30;
IO_PORT "CART_MUX_SIG[6]" IO_TYPE=LVCMOS33 PULL_MODE=UP BANK_VCCIO=3.3;
IO_LOC "CART_MUX_SIG[5]" 29;
IO_PORT "CART_MUX_SIG[5]" IO_TYPE=LVCMOS33 PULL_MODE=UP BANK_VCCIO=3.3;
IO_LOC "CART_MUX_SIG[4]" 26;
IO_PORT "CART_MUX_SIG[4]" IO_TYPE=LVCMOS33 PULL_MODE=UP BANK_VCCIO=3.3;
IO_LOC "CART_MUX_SIG[3]" 25;
IO_PORT "CART_MUX_SIG[3]" IO_TYPE=LVCMOS33 PULL_MODE=UP BANK_VCCIO=3.3;
IO_LOC "CART_MUX_SIG[2]" 28;
IO_PORT "CART_MUX_SIG[2]" IO_TYPE=LVCMOS33 PULL_MODE=UP BANK_VCCIO=3.3;
IO_LOC "CART_MUX_SIG[1]" 27;
IO_PORT "CART_MUX_SIG[1]" IO_TYPE=LVCMOS33 PULL_MODE=UP BANK_VCCIO=3.3;
IO_LOC "CART_MUX_SIG[0]" 77;
IO_PORT "CART_MUX_SIG[0]" IO_TYPE=LVCMOS33 PULL_MODE=UP BANK_VCCIO=3.3;
IO_LOC "CART_CLOCK" 76;
IO_PORT "CART_CLOCK" IO_TYPE=LVCMOS33 PULL_MODE=UP BANK_VCCIO=3.3;
IO_LOC "CART_WR_n" 16;
IO_PORT "CART_WR_n" IO_TYPE=LVCMOS33 PULL_MODE=UP BANK_VCCIO=3.3;
IO_LOC "CART_RD_n" 15;
IO_PORT "CART_RD_n" IO_TYPE=LVCMOS33 PULL_MODE=UP BANK_VCCIO=3.3;
IO_LOC "CART_SLTSL_n" 18;
IO_PORT "CART_SLTSL_n" IO_TYPE=LVCMOS33 PULL_MODE=UP BANK_VCCIO=3.3;

// Audio DAC (MAX98357A I2S)
IO_LOC "DAC_DIN" 54;
IO_PORT "DAC_DIN" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "DAC_BCLK" 56;
IO_PORT "DAC_BCLK" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "DAC_LRCLK" 55;
IO_PORT "DAC_LRCLK" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
IO_LOC "DAC_SDMODE_n" 51;
IO_PORT "DAC_SDMODE_n" IO_TYPE=LVCMOS33 PULL_MODE=UP DRIVE=8 BANK_VCCIO=3.3;
```

---

## 21. Apéndice B — Diferencias entre §16 conceptual y wt200b real

El §16 fue redactado tomando como referencia el esquema **conceptual** del cartucho tnCart rev1. La realidad de WonderTANG v2.0b en el codebase `tnCartWonder` introduce variaciones importantes para tener presentes al escribir RTL nuevo:

| Concepto en §16 | Realidad wt200b (commit `d3a54f1`) | Implicación para RTL |
|---|---|---|
| Sufijo `_33` (ej. `CLOCK_33`, `IORQ_33`) | Prefijo `CART_*` (ej. `CART_CLOCK`, `CART_RD_n`, `CART_SLTSL_n`) | Usa `CART_*` en módulos nuevos. El `_33` solo aparece en código heredado de tnCart rev1. |
| 2 chips 74LVC245 controlados por `MSEL0_33`/`MSEL1_33` | **3 chips** 74LVC245 controlados por `CART_MUX_CS_n[2:0]` | El multiplex es distinto. Ver módulo `BOARD_REV1_BUS` (`rtl/src/board/rev1/board_rev1_bus.sv`), reutilizado en wt200b con state machine de 9 ciclos. |
| `BUS_33[7:0]` multiplexa A0-A7 + D0-D7 | `CART_MUX_SIG[7:0]` multiplexa A8-A15 / A0-A7 / control bus; **`CART_DATA_SIG[7:0]` separado para D0-D7 (no multiplexado)** | El bus de datos tiene pines propios. El multiplex solo cicla address y control. |
| Multiplex con dos grupos: address + datos | Tres grupos: A8-A15 (CS0), A0-A7 (CS1), control bus (CS2: MERQ/IORQ/CS1/CS2/RESET/RFSH/CS12/M1) | Las señales de control que el §16 trata como pines directos (`IORQ_n`, `MREQ_n`, `RESET_n`, `M1_n`) están multiplexadas en CS2. |
| Swap de MSEL entre revisiones de placa | No aplica en wt200b: state machine fija en `BOARD_REV1_BUS` | La elección de qué chip activar (`CART_MUX_CS_n`) está hardcoded. Lo que sí varía entre placas son las constraints físicas (`.cst`). |
| Audio I2S: `I2S_BCLK`, `I2S_LRCLK`, `I2S_DIN` | `DAC_BCLK` (pin 56), `DAC_LRCLK` (pin 55), `DAC_DIN` (pin 54), **`DAC_SDMODE_n` (pin 51) para mute/standby** del MAX98357A | Hay una cuarta señal de control del DAC no documentada en §14. |
| Sample rate I2S 49.716 kHz (objetivo OPL3/OPL4) | I2S TX corre a ≈ **48 kHz** (CLK_DAC ≈ 1.542 MHz / 32 bits estéreo) en el bitstream stock | Si Fase 1 quiere los 44.1 kHz exactos del MoonSound real, hay que reconfigurar el divisor del DAC o resamplear en RTL. |
| Reloj 33.8688 MHz necesario para OPL3 (§11/§14) | El proyecto stock usa cristal de **27 MHz** (`CLK_27M`, pin 4). Dos PLLs: `u_pll_base` → 107.4 MHz (`CLK_BASE`/`CLK_MEM`) y `u_pll_tmds` → 134.25 MHz (HDMI/V9990) | Para OPL3 hay que añadir un nuevo `rPLL` o derivar 33.8688 MHz desde uno existente. |

Otros pines wt200b que el §16 no menciona pero existen en el `.cst` (ver Apéndice A):

- **HDMI / TMDS**: `tmds_clk_p` (33,34), `tmds_data_p[0..2]` (35-40) → salida del V9990 emulado.
- **Flash SPI interna del Tang Nano**: `mspi_sclk/cs/miso/mosi/hold` (59-63) → almacena ROM de Nextor sin necesitar microSD.
- **microSD**: `TF_SCLK` (83), `TF_CMD` (82), `TF_DAT[0..3]` (84, 85, 80, 81) → SDIO 4-bit completo (no SPI).
- **LED de actividad**: pin 75.
- **UART**: `UART_RX` (70), `UART_TX` (69) → debug.

**Recomendación**: cuando se escriba RTL propio para Fase 1, usar los nombres del Apéndice A, no los del §5/§16. El §16 se conserva como introducción conceptual al multiplexado, pero **la fuente de verdad es el Apéndice A**.
