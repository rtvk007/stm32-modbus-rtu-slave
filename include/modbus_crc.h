/**
 * Modbus RTU CRC-16 (polynomial 0xA001, init 0xFFFF)
 * Modbus over Serial Line Specification V1.02
 */
#ifndef MODBUS_CRC_H
#define MODBUS_CRC_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Compute CRC-16 over data[0..len-1]. */
uint16_t modbus_crc16(const uint8_t *data, size_t len);

/**
 * Append CRC to frame in little-endian (CRC_lo, CRC_hi).
 * frame must have room for 2 extra bytes; len is current payload length.
 * Returns new length (len + 2).
 */
size_t modbus_crc16_append(uint8_t *frame, size_t len);

/**
 * Verify CRC of an ADU. Total length includes 2 CRC bytes.
 * Returns true if CRC matches.
 */
bool modbus_crc16_check(const uint8_t *adu, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_CRC_H */
