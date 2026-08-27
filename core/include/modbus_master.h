/**
 * @file    modbus_master.h
 * @brief   Modbus RTU master (client) API — portable core.
 */
#ifndef MODBUS_MASTER_H
#define MODBUS_MASTER_H

#include "modbus_types.h"
#include "modbus_port.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (MODBUS_CFG_ENABLE_MASTER == 1u)

/**
 * @brief Master instance (no heap; place in BSS / static / stack).
 */
typedef struct {
    const modbus_port_t *port;              /**< Bound platform port */
    uint32_t             response_timeout_ms;
    uint32_t             byte_timeout_ms;
    uint32_t             turnaround_ms;
    uint8_t              last_exception;    /**< Valid if status is EXCEPTION */
} modbus_master_t;

/**
 * @brief Initialise master and derive RTU timings from @p baud.
 */
modbus_status_t modbus_master_init(modbus_master_t *master,
                                   const modbus_port_t *port,
                                   uint32_t baud);

modbus_status_t modbus_master_read_coils(modbus_master_t *master,
                                         uint8_t slave,
                                         uint16_t addr,
                                         uint16_t quantity,
                                         uint8_t *dest);

modbus_status_t modbus_master_read_discrete_inputs(modbus_master_t *master,
                                                   uint8_t slave,
                                                   uint16_t addr,
                                                   uint16_t quantity,
                                                   uint8_t *dest);

modbus_status_t modbus_master_read_holding_registers(modbus_master_t *master,
                                                     uint8_t slave,
                                                     uint16_t addr,
                                                     uint16_t quantity,
                                                     uint16_t *dest);

modbus_status_t modbus_master_read_input_registers(modbus_master_t *master,
                                                   uint8_t slave,
                                                   uint16_t addr,
                                                   uint16_t quantity,
                                                   uint16_t *dest);

modbus_status_t modbus_master_write_single_coil(modbus_master_t *master,
                                                uint8_t slave,
                                                uint16_t addr,
                                                bool on);

modbus_status_t modbus_master_write_single_register(modbus_master_t *master,
                                                    uint8_t slave,
                                                    uint16_t addr,
                                                    uint16_t value);

modbus_status_t modbus_master_write_multiple_coils(modbus_master_t *master,
                                                   uint8_t slave,
                                                   uint16_t addr,
                                                   uint16_t quantity,
                                                   const uint8_t *src);

modbus_status_t modbus_master_write_multiple_registers(modbus_master_t *master,
                                                       uint8_t slave,
                                                       uint16_t addr,
                                                       uint16_t quantity,
                                                       const uint16_t *src);

#endif /* MODBUS_CFG_ENABLE_MASTER */

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_MASTER_H */
