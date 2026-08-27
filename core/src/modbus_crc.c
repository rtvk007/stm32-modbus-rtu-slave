/**
 * @file    modbus_crc.c
 * @brief   Modbus RTU CRC-16 implementation (portable, no heap).
 */
#include "modbus_crc.h"

uint16_t modbus_crc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFFu;
    uint16_t i;
    uint8_t bit;

    if (data == NULL) {
        return 0u;
    }

    for (i = 0u; i < len; i++) {
        crc ^= (uint16_t)data[i];
        for (bit = 0u; bit < 8u; bit++) {
            if ((crc & 0x0001u) != 0u) {
                crc = (uint16_t)((crc >> 1) ^ 0xA001u);
            } else {
                crc = (uint16_t)(crc >> 1);
            }
        }
    }

    return crc;
}

uint16_t modbus_crc16_append(uint8_t *frame, uint16_t len)
{
    uint16_t crc;

    if (frame == NULL) {
        return 0u;
    }

    crc = modbus_crc16(frame, len);
    frame[len] = (uint8_t)(crc & 0xFFu);
    frame[(uint16_t)(len + 1u)] = (uint8_t)((crc >> 8) & 0xFFu);

    return (uint16_t)(len + 2u);
}

bool modbus_crc16_check(const uint8_t *adu, uint16_t len)
{
    uint16_t calc;
    uint16_t recv;
    bool ok = false;

    if ((adu != NULL) && (len >= 4u)) {
        calc = modbus_crc16(adu, (uint16_t)(len - 2u));
        recv = (uint16_t)((uint16_t)adu[len - 2u] |
                          ((uint16_t)adu[len - 1u] << 8));
        if (calc == recv) {
            ok = true;
        }
    }

    return ok;
}
