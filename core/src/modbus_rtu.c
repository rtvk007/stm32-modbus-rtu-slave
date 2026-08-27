/**
 * @file    modbus_rtu.c
 * @brief   RTU framing helpers (portable core).
 */
#include "modbus_rtu.h"
#include "modbus_crc.h"

void modbus_rtu_timing_ms(uint32_t baud, uint32_t *t15_ms, uint32_t *t35_ms)
{
    uint32_t char_us;
    uint32_t t15_us;
    uint32_t t35_us;
    uint32_t local_baud = baud;

    if (local_baud == 0u) {
        local_baud = MODBUS_CFG_DEFAULT_BAUD;
    }

    /* Spec: above 19200 baud use fixed 750 us / 1750 us */
    if (local_baud > 19200u) {
        if (t15_ms != NULL) {
            *t15_ms = 1u;
        }
        if (t35_ms != NULL) {
            *t35_ms = 2u;
        }
    } else {
        /* 11 bit times per character (start + 8 data + parity + stop) */
        char_us = (11u * 1000000u) / local_baud;
        t15_us = (char_us * 3u) / 2u;
        t35_us = (char_us * 7u) / 2u;

        if (t15_ms != NULL) {
            *t15_ms = (t15_us + 999u) / 1000u;
            if (*t15_ms < 1u) {
                *t15_ms = 1u;
            }
        }
        if (t35_ms != NULL) {
            *t35_ms = (t35_us + 999u) / 1000u;
            if (*t35_ms < 1u) {
                *t35_ms = 1u;
            }
        }
    }
}

uint16_t modbus_rtu_build(uint8_t slave,
                          const uint8_t *pdu,
                          uint16_t pdu_len,
                          uint8_t *adu,
                          uint16_t adu_cap)
{
    uint16_t i;
    uint16_t out_len = 0u;

    if ((pdu == NULL) || (adu == NULL) || (pdu_len == 0u) ||
        (pdu_len > MODBUS_PDU_MAX_SIZE)) {
        return 0u;
    }

    if (adu_cap < (uint16_t)(pdu_len + 3u)) {
        return 0u;
    }

    adu[0] = slave;
    for (i = 0u; i < pdu_len; i++) {
        adu[(uint16_t)(1u + i)] = pdu[i];
    }

    out_len = modbus_crc16_append(adu, (uint16_t)(1u + pdu_len));
    return out_len;
}

modbus_status_t modbus_rtu_parse(const uint8_t *adu,
                                 uint16_t adu_len,
                                 uint8_t *slave,
                                 const uint8_t **pdu,
                                 uint16_t *pdu_len)
{
    if ((adu == NULL) || (slave == NULL) || (pdu == NULL) || (pdu_len == NULL)) {
        return MODBUS_ERR_NULL_PTR;
    }

    if (adu_len < 4u) {
        return MODBUS_ERR_FRAME;
    }

    if (adu_len > MODBUS_ADU_MAX_SIZE) {
        return MODBUS_ERR_OVERFLOW;
    }

    if (modbus_crc16_check(adu, adu_len) == false) {
        return MODBUS_ERR_CRC;
    }

    *slave = adu[0];
    *pdu = &adu[1];
    *pdu_len = (uint16_t)(adu_len - 3u);

    if ((*pdu_len < 1u) || (*pdu_len > MODBUS_PDU_MAX_SIZE)) {
        return MODBUS_ERR_FRAME;
    }

    return MODBUS_OK;
}

uint16_t modbus_pack_bits(const uint8_t *bits, uint16_t count, uint8_t *out)
{
    uint16_t nbytes;
    uint16_t i;

    if ((bits == NULL) || (out == NULL)) {
        return 0u;
    }

    nbytes = (uint16_t)((count + 7u) / 8u);
    for (i = 0u; i < nbytes; i++) {
        out[i] = 0u;
    }

    for (i = 0u; i < count; i++) {
        if (bits[i] != 0u) {
            out[i / 8u] = (uint8_t)(out[i / 8u] | (uint8_t)(1u << (i % 8u)));
        }
    }

    return nbytes;
}

void modbus_unpack_bits(const uint8_t *in, uint16_t count, uint8_t *bits)
{
    uint16_t i;

    if ((in == NULL) || (bits == NULL)) {
        return;
    }

    for (i = 0u; i < count; i++) {
        bits[i] = (uint8_t)((in[i / 8u] >> (i % 8u)) & 0x01u);
    }
}
