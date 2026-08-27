/**
 * @file    README.md
 * @brief   STM32 port notes
 *
 * Files in this folder are the ONLY Modbus sources that include STM32 HAL.
 * Link against your Cube-generated `stm32****_hal_uart.c` and ensure
 * `main.h` (or the family HAL header) is on the include path.
 *
 * RS-485: drive DE/RE around `uart_write` in a custom wrapper if needed;
 * you can wrap `modbus_stm32_port_t` without touching `core/`.
 */
