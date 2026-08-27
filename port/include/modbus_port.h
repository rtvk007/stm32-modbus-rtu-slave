/**
 * @file    modbus_port.h
 * @brief   Platform portability interface (Layer 0 — BSP / HAL adapter).
 *
 * @details The Modbus core NEVER includes MCU headers. Each target provides
 *          an implementation of this interface (see platform/stm32 and
 *          platform/template). To port to a new MCU:
 *          1. Copy platform/template/modbus_port_template.c
 *          2. Fill UART TX/RX, RX flush, and millisecond tick
 *          3. Bind callbacks into a modbus_port_t and pass to master/slave
 *
 * MISRA: Core modules depend only on this header; no HW types leak upward.
 */
#ifndef MODBUS_PORT_H
#define MODBUS_PORT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Write bytes to the serial line (blocking or with driver-internal wait).
 * @param[in] ctx   Platform context (e.g. UART handle wrapper).
 * @param[in] data  Buffer to transmit.
 * @param[in] len   Number of bytes.
 * @return Number of bytes written (>=0), or negative on error.
 */
typedef int32_t (*modbus_port_uart_write_fn)(void *ctx,
                                             const uint8_t *data,
                                             uint16_t len);

/**
 * @brief Read up to @p len bytes with a timeout.
 * @param[in]  ctx         Platform context.
 * @param[out] data        Destination buffer.
 * @param[in]  len         Maximum bytes to read.
 * @param[in]  timeout_ms  Wait time for at least one byte / operation.
 * @return Bytes read (>0), 0 on timeout, or negative on error.
 */
typedef int32_t (*modbus_port_uart_read_fn)(void *ctx,
                                            uint8_t *data,
                                            uint16_t len,
                                            uint32_t timeout_ms);

/**
 * @brief Discard pending RX data (optional; may be NULL).
 * @param[in] ctx  Platform context.
 */
typedef void (*modbus_port_uart_flush_fn)(void *ctx);

/**
 * @brief Millisecond tick for timeouts (monotonic, wrap-safe differences).
 * @param[in] ctx  Platform context (may be unused).
 * @return Tick in milliseconds.
 */
typedef uint32_t (*modbus_port_get_tick_ms_fn)(void *ctx);

/**
 * @brief Portability object bound to one physical serial channel.
 */
typedef struct {
    modbus_port_uart_write_fn  uart_write;   /**< Required */
    modbus_port_uart_read_fn   uart_read;    /**< Required */
    modbus_port_uart_flush_fn  uart_flush;   /**< Optional (NULL allowed) */
    modbus_port_get_tick_ms_fn get_tick_ms;  /**< Required */
    void                      *ctx;          /**< Passed to all callbacks */
} modbus_port_t;

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_PORT_H */
