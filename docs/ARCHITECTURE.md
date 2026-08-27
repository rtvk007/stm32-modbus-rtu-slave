# Architecture — layered Modbus RTU (C99 / MISRA-oriented)

## Layers (bottom → top)

| Layer | Path | Hardware? | Responsibility |
|-------|------|-----------|----------------|
| **0 – Port / BSP** | `port/include/modbus_port.h` + `platform/<mcu>/` | **Yes** | UART TX/RX, RX flush, ms tick |
| **1 – Core protocol** | `core/` | **No** | CRC, RTU framing, master, slave, map |
| **2 – Config** | `config/modbus_cfg.h` | No | Sizes, feature flags |
| **3 – Application** | `examples/` / your firmware | Board-specific | Init map, call poll / master APIs |

```
┌─────────────────────────────────────────┐
│  Application (stm32_*_app.c / your app) │
├─────────────────────────────────────────┤
│  Core: master / slave / map / RTU / CRC │  ← copy unchanged to any MCU
├─────────────────────────────────────────┤
│  modbus_port_t callbacks                │  ← stable contract
├─────────────────────────────────────────┤
│  platform/stm32  OR  platform/<new>     │  ← only files you replace
└─────────────────────────────────────────┘
```

## Porting to a non-STM32 MCU

1. Copy `platform/template/modbus_port_template.c` → `platform/<mcu>/`.
2. Implement write / read / flush / get_tick_ms.
3. Keep including `core/` + `config/` + `port/include` only.
4. **Do not modify** `core/` for UART differences.

## STM32 integration (CubeMX)

1. USART: 9600 8E1 (or 8N2 if no parity).
2. Add include paths: `config`, `core/include`, `port/include`, `platform/stm32`.
3. Add all `core/src/*.c` + `platform/stm32/modbus_stm32_port.c`.
4. Call `Modbus_SlaveApp_Init()` / `Poll()` from `main.c` (see examples).

## MISRA / C99 practices used

- C99 fixed-width types (`stdint.h`); no heap in core or STM32 port.
- Explicit braces on control structures; checked return values at API boundaries.
- Static file-scope helpers; no recursion.
- MCU headers confined to `platform/*` (core stays portable).
- Unsigned literals with `u` suffix for protocol constants.

Run a MISRA checker (e.g. PC-Lint / Coverity) on `core/` for project certification; BSP files may need documented deviations for HAL casts.
