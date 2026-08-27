/**
 * @file    modbus_slave.h
 * @brief   Modbus RTU slave (server) API — portable core.
 */
#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#include "modbus_types.h"
#include "modbus_map.h"
#include "modbus_port.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (MODBUS_CFG_ENABLE_SLAVE == 1u)

typedef struct modbus_slave modbus_slave_t;

/**
 * @brief Optional callback after a successful write to the map.
 * @param[in] slave     Slave instance.
 * @param[in] fc        Function code.
 * @param[in] addr      Starting PDU address.
 * @param[in] quantity  Number of coils/registers written.
 * @param[in] user      User pointer from slave->user.
 */
typedef void (*modbus_slave_write_cb_t)(modbus_slave_t *slave,
                                        uint8_t fc,
                                        uint16_t addr,
                                        uint16_t quantity,
                                        void *user);

struct modbus_slave {
    uint8_t                   address;
    modbus_map_t             *map;
    const modbus_port_t      *port;
    uint32_t                  frame_timeout_ms;
    uint32_t                  byte_timeout_ms;
    modbus_slave_write_cb_t   on_write;
    void                     *user;
    uint8_t                   rx[MODBUS_ADU_MAX_SIZE];
    uint16_t                  rx_len;
};

/**
 * @brief Initialise slave instance.
 */
modbus_status_t modbus_slave_init(modbus_slave_t *slave,
                                  uint8_t address,
                                  modbus_map_t *map,
                                  const modbus_port_t *port,
                                  uint32_t baud);

/**
 * @brief Poll UART for one request and send a response if addressed.
 * @note  Call from the main loop or a dedicated task.
 */
modbus_status_t modbus_slave_poll(modbus_slave_t *slave);

/**
 * @brief Process a complete ADU already in memory (unit tests / custom RX).
 * @param[out] resp      Response ADU buffer.
 * @param[out] resp_len  Response length (0 for broadcast / not addressed).
 * @param[in]  resp_cap  Capacity of @p resp.
 */
modbus_status_t modbus_slave_handle(modbus_slave_t *slave,
                                    const uint8_t *adu,
                                    uint16_t adu_len,
                                    uint8_t *resp,
                                    uint16_t *resp_len,
                                    uint16_t resp_cap);

#endif /* MODBUS_CFG_ENABLE_SLAVE */

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_SLAVE_H */
