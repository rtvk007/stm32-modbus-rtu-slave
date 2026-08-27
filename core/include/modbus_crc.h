/**
 * @file    modbus_crc.h
 * @brief   Modbus RTU CRC-16 (portable core).
 */
#ifndef MODBUS_CRC_H
#define MODBUS_CRC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Compute Modbus CRC-16 (poly 0xA001, init 0xFFFF).
 * @param[in] data  Input bytes.
 * @param[in] len   Length in bytes.
 * @return CRC value (low byte is transmitted first on the wire).
 */
uint16_t modbus_crc16(const uint8_t *data, uint16_t len);

/**
 * @brief Append CRC as little-endian (CRC_lo, CRC_hi) after @p len bytes.
 * @param[in,out] frame  Buffer with payload; must have room for 2 extra bytes.
 * @param[in]     len    Current payload length.
 * @return New length (len + 2), or 0u on error.
 */
uint16_t modbus_crc16_append(uint8_t *frame, uint16_t len);

/**
 * @brief Verify CRC of a complete RTU ADU.
 * @param[in] adu  ADU including CRC.
 * @param[in] len  Total length including 2 CRC bytes.
 * @return true if CRC matches.
 */
bool modbus_crc16_check(const uint8_t *adu, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_CRC_H */
