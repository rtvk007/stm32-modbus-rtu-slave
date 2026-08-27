/**
 * @file    modbus_types.h
 * @brief   Shared types and Modbus protocol constants (hardware-independent).
 * @details Aligned with:
 *          - Modbus Application Protocol V1.1b3
 *          - Modbus over Serial Line Specification V1.02 (RTU)
 *
 * MISRA notes:
 * - Uses ISO C99 fixed-width types (stdint.h).
 * - No dynamic memory allocation in this module.
 */
#ifndef MODBUS_TYPES_H
#define MODBUS_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "modbus_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/* Protocol limits                                                            */
/* -------------------------------------------------------------------------- */
#define MODBUS_ADU_MAX_SIZE                 (256u)
#define MODBUS_PDU_MAX_SIZE                 (253u)

#define MODBUS_ADDR_BROADCAST               (0u)
#define MODBUS_ADDR_MIN                     (1u)
#define MODBUS_ADDR_MAX                     (247u)

#define MODBUS_MAX_READ_BITS                (2000u)
#define MODBUS_MAX_WRITE_BITS               (1968u)
#define MODBUS_MAX_READ_REGS                (125u)
#define MODBUS_MAX_WRITE_REGS               (123u)

/* -------------------------------------------------------------------------- */
/* Function codes                                                             */
/* -------------------------------------------------------------------------- */
#define MODBUS_FC_READ_COILS                (0x01u)
#define MODBUS_FC_READ_DISCRETE_INPUTS      (0x02u)
#define MODBUS_FC_READ_HOLDING_REGISTERS    (0x03u)
#define MODBUS_FC_READ_INPUT_REGISTERS      (0x04u)
#define MODBUS_FC_WRITE_SINGLE_COIL         (0x05u)
#define MODBUS_FC_WRITE_SINGLE_REGISTER     (0x06u)
#define MODBUS_FC_WRITE_MULTIPLE_COILS      (0x0Fu)
#define MODBUS_FC_WRITE_MULTIPLE_REGISTERS  (0x10u)
#define MODBUS_FC_EXCEPTION_FLAG            (0x80u)

/* -------------------------------------------------------------------------- */
/* Exception codes                                                            */
/* -------------------------------------------------------------------------- */
#define MODBUS_EX_ILLEGAL_FUNCTION          (0x01u)
#define MODBUS_EX_ILLEGAL_DATA_ADDRESS      (0x02u)
#define MODBUS_EX_ILLEGAL_DATA_VALUE        (0x03u)
#define MODBUS_EX_SLAVE_DEVICE_FAILURE      (0x04u)
#define MODBUS_EX_ACKNOWLEDGE               (0x05u)
#define MODBUS_EX_SLAVE_DEVICE_BUSY         (0x06u)
#define MODBUS_EX_MEMORY_PARITY_ERROR       (0x08u)
#define MODBUS_EX_GATEWAY_PATH_UNAVAILABLE  (0x0Au)
#define MODBUS_EX_GATEWAY_TARGET_FAILED     (0x0Bu)

#define MODBUS_COIL_OFF                     (0x0000u)
#define MODBUS_COIL_ON                      (0xFF00u)

/* -------------------------------------------------------------------------- */
/* Status codes                                                               */
/* -------------------------------------------------------------------------- */
typedef enum {
    MODBUS_OK = 0,
    MODBUS_ERR_NULL_PTR,
    MODBUS_ERR_INVALID_ARG,
    MODBUS_ERR_TIMEOUT,
    MODBUS_ERR_CRC,
    MODBUS_ERR_FRAME,
    MODBUS_ERR_EXCEPTION,
    MODBUS_ERR_IO,
    MODBUS_ERR_NO_RESPONSE,
    MODBUS_ERR_OVERFLOW,
    MODBUS_ERR_BUSY,
    MODBUS_ERR_NOT_SUPPORTED
} modbus_status_t;

/**
 * @brief Convert status code to a short C-string (for logging / debug).
 * @param[in] status  Status value.
 * @return Pointer to a static string literal (never NULL).
 */
const char *modbus_strerror(modbus_status_t status);

/**
 * @brief Store uint16 as Modbus big-endian at @p p.
 * @param[out] p  Destination (must provide 2 bytes).
 * @param[in]  v  Value.
 */
void modbus_put_u16_be(uint8_t *p, uint16_t v);

/**
 * @brief Load uint16 from Modbus big-endian at @p p.
 * @param[in] p  Source (must provide 2 bytes).
 * @return Value.
 */
uint16_t modbus_get_u16_be(const uint8_t *p);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_TYPES_H */
