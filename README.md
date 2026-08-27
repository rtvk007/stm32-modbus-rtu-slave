# Modbus RTU — layered C99 driver (STM32 + portable core)

Hardware-independent **core** + thin **MCU port**. Port to another MCU by
copying `platform/template` and implementing four UART/tick callbacks.

Specs: Modbus Application Protocol V1.1b3 · Serial Line V1.02 (RTU).

## Tree

```
config/                 Compile-time sizes / features (modbus_cfg.h)
port/include/           modbus_port.h — BSP contract (no HW types)
core/                   Portable protocol (CRC, RTU, map, master, slave)
platform/stm32/         STM32Cube HAL adapter
platform/template/      Copy-paste port for a new MCU
examples/               STM32 app sketches + host selftest
docs/ARCHITECTURE.md    Layer diagram and MISRA notes
```

## Function codes

01, 02, 03, 04, 05, 06, 15, 16 — master and slave.

## Host selftest (no MCU)

```bat
build.bat
```

## STM32 (CubeMX / CubeIDE)

1. USART **9600 8E1** (typical).
2. Include paths: `config`, `core/include`, `port/include`, `platform/stm32`.
3. Compile all `core/src/*.c` + `platform/stm32/modbus_stm32_port.c`.
4. Use `examples/stm32_slave_app.c` or `stm32_master_app.c` as a guide.

```c
modbus_stm32_port_init(&g_mb_port, &huart1, 100u);
modbus_slave_init(&g_slave, 1u, &g_map, &g_mb_port.port, 9600u);
/* loop: */ modbus_slave_poll(&g_slave);
```

## Porting checklist

| Keep as-is | Replace per MCU |
|------------|-----------------|
| `core/**` | `platform/<mcu>/**` |
| `config/modbus_cfg.h` (tune sizes) | UART + `HAL_GetTick` / timer |
| `port/include/modbus_port.h` | — |

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).
