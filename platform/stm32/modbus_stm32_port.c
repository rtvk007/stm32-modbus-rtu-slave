/**
 * @file    modbus_stm32_port.c
 * @brief   STM32Cube HAL implementation of modbus_port_t.
 *
 * MISRA notes for this BSP file:
 * - Dynamic allocation is not used.
 * - HAL return codes are checked and mapped to negative I/O errors.
 * - Only this translation unit may include STM32 HAL headers for Modbus.
 */
#include "modbus_stm32_port.h"

#include <stddef.h>

/* -------------------------------------------------------------------------- */
/* Port callbacks (static — file scope)                                       */
/* -------------------------------------------------------------------------- */

/**
 * @brief HAL UART transmit wrapper.
 */
static int32_t stm32_uart_write(void *ctx, const uint8_t *data, uint16_t len)
{
    modbus_stm32_port_t *p = (modbus_stm32_port_t *)ctx;
    HAL_StatusTypeDef hs;
    int32_t result;

    if ((p == NULL) || (p->huart == NULL) || (data == NULL) || (len == 0u)) {
        result = -1;
    } else {
        hs = HAL_UART_Transmit(p->huart,
                               (uint8_t *)data, /* HAL API is non-const */
                               len,
                               p->tx_timeout_ms);
        if (hs == HAL_OK) {
            result = (int32_t)len;
        } else {
            result = -1;
        }
    }

    return result;
}

/**
 * @brief Read with timeout: first byte waits up to timeout_ms, then returns.
 * @note  Polling mode for simplicity and MISRA clarity. For high baud / DMA,
 *        replace with ring-buffer + IDLE-line IRQ without changing core/.
 */
static int32_t stm32_uart_read(void *ctx,
                               uint8_t *data,
                               uint16_t len,
                               uint32_t timeout_ms)
{
    modbus_stm32_port_t *p = (modbus_stm32_port_t *)ctx;
    HAL_StatusTypeDef hs;
    int32_t result;

    if ((p == NULL) || (p->huart == NULL) || (data == NULL) || (len == 0u)) {
        result = -1;
    } else {
        hs = HAL_UART_Receive(p->huart, data, len, timeout_ms);
        if (hs == HAL_OK) {
            result = (int32_t)len;
        } else if (hs == HAL_TIMEOUT) {
            result = 0;
        } else {
            result = -1;
        }
    }

    return result;
}

/**
 * @brief Flush UART RX (abort ongoing RX and clear errors / data).
 */
static void stm32_uart_flush(void *ctx)
{
    modbus_stm32_port_t *p = (modbus_stm32_port_t *)ctx;

    if ((p != NULL) && (p->huart != NULL)) {
        (void)HAL_UART_AbortReceive(p->huart);
        __HAL_UART_CLEAR_OREFLAG(p->huart);
        /* Read RDR if data pending — family macros differ; ORE clear is enough
         * for most Cube packs. Add __HAL_UART_FLUSH_DRREGISTER if available. */
#if defined(__HAL_UART_FLUSH_DRREGISTER)
        __HAL_UART_FLUSH_DRREGISTER(p->huart);
#endif
    }
}

/**
 * @brief SysTick-based millisecond clock from HAL.
 */
static uint32_t stm32_get_tick_ms(void *ctx)
{
    (void)ctx;
    return HAL_GetTick();
}

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

int32_t modbus_stm32_port_init(modbus_stm32_port_t *ctx,
                               UART_HandleTypeDef *huart,
                               uint32_t tx_timeout_ms)
{
    int32_t rc = -1;

    if ((ctx != NULL) && (huart != NULL)) {
        ctx->huart = huart;
        ctx->tx_timeout_ms = (tx_timeout_ms == 0u) ? 100u : tx_timeout_ms;

        ctx->port.uart_write = stm32_uart_write;
        ctx->port.uart_read = stm32_uart_read;
        ctx->port.uart_flush = stm32_uart_flush;
        ctx->port.get_tick_ms = stm32_get_tick_ms;
        ctx->port.ctx = ctx;

        rc = 0;
    }

    return rc;
}
