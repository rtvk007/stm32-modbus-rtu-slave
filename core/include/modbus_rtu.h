/**
 * @file    modbus_rtu.h
 * @brief   Modbus RTU framing and bit packing (portable core).
 */
#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include "modbus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Compute T1.5 / T3.5 character timing in milliseconds from baud rate.
 * @param[in]  baud    Baud rate (e.g. 9600).
 * @param[out] t15_ms  Inter-character timeout (~1.5 char times), may be NULL.
 * @param[out] t35_ms  Inter-frame silent interval (~3.5 char times), may be NULL.
 */
void modbus_rtu_timing_ms(uint32_t baud, uint32_t *t15_ms, uint32_t *t35_ms);

/**
 * @brief Build RTU ADU: [addr][PDU...][CRC_lo][CRC_hi].
 * @return ADU length, or 0u on error.
 */
uint16_t modbus_rtu_build(uint8_t slave,
                          const uint8_t *pdu,
                          uint16_t pdu_len,
                          uint8_t *adu,
                          uint16_t adu_cap);

/**
 * @brief Parse and validate RTU ADU; on success points @p pdu into @p adu.
 */
modbus_status_t modbus_rtu_parse(const uint8_t *adu,
                                 uint16_t adu_len,
                                 uint8_t *slave,
                                 const uint8_t **pdu,
                                 uint16_t *pdu_len);

/**
 * @brief Pack 0/1 bits into Modbus coil byte array (LSB first in each byte).
 * @return Number of bytes written.
 */
uint16_t modbus_pack_bits(const uint8_t *bits, uint16_t count, uint8_t *out);

/**
 * @brief Unpack Modbus coil bytes into 0/1 bit array.
 */
void modbus_unpack_bits(const uint8_t *in, uint16_t count, uint8_t *bits);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_RTU_H */
