/**
 * @file    modbus_port_template.c
 * @brief   Copy-paste template to port Modbus RTU to a new MCU.
 *
 * HOW TO PORT
 * -----------
 * 1. Copy this file to platform/<your_mcu>/modbus_<mcu>_port.c (+ matching .h).
 * 2. Replace YOUR_UART_* stubs with driver calls (register or vendor HAL).
 * 3. Keep function signatures identical so core/ sources stay untouched.
 * 4. In application code:
 *        static modbus_mcu_port_t g_port;
 *        modbus_mcu_port_init(&g_port, ...);
 *        modbus_slave_init(&slave, 1u, &map, &g_port.port, 9600u);
 *
 * DO NOT put MCU includes into core/ or config/.
 */

#include "modbus_port.h"

#include <stdint.h>
#include <stddef.h>

/* === BEGIN: replace with your MCU headers ================================= */
/* #include "your_mcu_uart.h" */
/* === END ================================================================== */

typedef struct {
    /* Opaque UART handle / base address for your MCU */
    void    *uart;
    uint32_t tx_timeout_ms;
    modbus_port_t port;
} modbus_mcu_port_t;

/* --- Stubs: implement these four operations ------------------------------- */

static int32_t mcu_uart_write(void *ctx, const uint8_t *data, uint16_t len)
{
    modbus_mcu_port_t *p = (modbus_mcu_port_t *)ctx;

    if ((p == NULL) || (data == NULL) || (len == 0u)) {
        return -1;
    }

    /* TODO: blocking or buffered TX of len bytes */
    (void)p;
    return -1; /* replace with (int32_t)len on success */
}

static int32_t mcu_uart_read(void *ctx,
                             uint8_t *data,
                             uint16_t len,
                             uint32_t timeout_ms)
{
    modbus_mcu_port_t *p = (modbus_mcu_port_t *)ctx;

    if ((p == NULL) || (data == NULL) || (len == 0u)) {
        return -1;
    }

    /* TODO: wait up to timeout_ms for data; return bytes read or 0 on timeout */
    (void)p;
    (void)timeout_ms;
    return 0;
}

static void mcu_uart_flush(void *ctx)
{
    modbus_mcu_port_t *p = (modbus_mcu_port_t *)ctx;

    if (p != NULL) {
        /* TODO: clear RX FIFO / abort RX */
        (void)p;
    }
}

static uint32_t mcu_get_tick_ms(void *ctx)
{
    (void)ctx;
    /* TODO: return free-running millisecond counter */
    return 0u;
}

/**
 * @brief Example init — rename and expose via your platform header.
 */
int32_t modbus_mcu_port_init(modbus_mcu_port_t *ctx, void *uart, uint32_t tx_timeout_ms)
{
    int32_t rc = -1;

    if ((ctx != NULL) && (uart != NULL)) {
        ctx->uart = uart;
        ctx->tx_timeout_ms = tx_timeout_ms;
        ctx->port.uart_write = mcu_uart_write;
        ctx->port.uart_read = mcu_uart_read;
        ctx->port.uart_flush = mcu_uart_flush;
        ctx->port.get_tick_ms = mcu_get_tick_ms;
        ctx->port.ctx = ctx;
        rc = 0;
    }

    return rc;
}
