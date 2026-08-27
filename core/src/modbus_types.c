/**
 * @file    modbus_types.c
 * @brief   Shared helpers for the Modbus core.
 */
#include "modbus_types.h"

const char *modbus_strerror(modbus_status_t status)
{
    const char *msg;

    switch (status) {
    case MODBUS_OK:
        msg = "ok";
        break;
    case MODBUS_ERR_NULL_PTR:
        msg = "null pointer";
        break;
    case MODBUS_ERR_INVALID_ARG:
        msg = "invalid argument";
        break;
    case MODBUS_ERR_TIMEOUT:
        msg = "timeout";
        break;
    case MODBUS_ERR_CRC:
        msg = "CRC error";
        break;
    case MODBUS_ERR_FRAME:
        msg = "frame error";
        break;
    case MODBUS_ERR_EXCEPTION:
        msg = "Modbus exception";
        break;
    case MODBUS_ERR_IO:
        msg = "I/O error";
        break;
    case MODBUS_ERR_NO_RESPONSE:
        msg = "no response";
        break;
    case MODBUS_ERR_OVERFLOW:
        msg = "buffer overflow";
        break;
    case MODBUS_ERR_BUSY:
        msg = "busy";
        break;
    case MODBUS_ERR_NOT_SUPPORTED:
        msg = "not supported";
        break;
    default:
        msg = "unknown error";
        break;
    }

    return msg;
}

void modbus_put_u16_be(uint8_t *p, uint16_t v)
{
    if (p != NULL) {
        p[0] = (uint8_t)((v >> 8) & 0xFFu);
        p[1] = (uint8_t)(v & 0xFFu);
    }
}

uint16_t modbus_get_u16_be(const uint8_t *p)
{
    uint16_t v = 0u;

    if (p != NULL) {
        v = (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
    }

    return v;
}
