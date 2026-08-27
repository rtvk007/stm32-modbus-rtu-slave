# MISRA C:2012 / C99 orientation

This stack is written for C99 with practices that map well to common MISRA rules.
It is **not** a certified MISRA deliverable; run your project checker on `core/`.

## Applied practices

| Topic | Approach |
|-------|----------|
| Heap | No `malloc`/`free` in `core/` or STM32 port |
| Types | `stdint.h` fixed-width; protocol constants use `u` suffix |
| Control flow | Braced `if`/`else`/`for`/`while`/`switch` |
| Pointers | NULL checks at public API entry |
| Scope | `static` for file-local helpers |
| Recursion | None |
| HW coupling | MCU headers only under `platform/` |
| Casts | Explicit; HAL non-const TX buffer noted as BSP deviation |

## Likely checker deviations (BSP)

- `HAL_UART_Transmit` requires `uint8_t *` (drops const) — STM32 HAL API.
- Inclusion of vendor `main.h` / HAL in `platform/stm32` only.

## Recommended include order in application

1. `modbus_cfg.h` overrides via compiler `-D` if needed  
2. Core headers (`modbus_master.h` / `modbus_slave.h`)  
3. Platform header (`modbus_stm32_port.h`)
