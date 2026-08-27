/**
 * @file    modbus_slave.c
 * @brief   Modbus RTU slave implementation (portable; uses modbus_port_t only).
 */
#include "modbus_slave.h"

#if (MODBUS_CFG_ENABLE_SLAVE == 1u)

#include "modbus_rtu.h"
#include "modbus_crc.h"

#include <string.h>

/* -------------------------------------------------------------------------- */
/* PDU handlers                                                               */
/* -------------------------------------------------------------------------- */

static uint16_t slave_exception(uint8_t fc, uint8_t ex, uint8_t *pdu)
{
    pdu[0] = (uint8_t)(fc | MODBUS_FC_EXCEPTION_FLAG);
    pdu[1] = ex;
    return 2u;
}

static bool slave_range_ok(uint16_t addr, uint16_t qty, uint16_t map_size)
{
    bool ok = false;

    if (qty != 0u) {
        if (((uint32_t)addr + (uint32_t)qty) <= (uint32_t)map_size) {
            ok = true;
        }
    }

    return ok;
}

static uint16_t handle_read_bits(modbus_slave_t *slave,
                                 uint8_t fc,
                                 const uint8_t *req,
                                 uint16_t req_len,
                                 uint8_t *rsp,
                                 bool discrete)
{
    uint16_t addr;
    uint16_t qty;
    uint16_t map_n;
    const uint8_t *src;
    uint8_t nbytes;

    if (req_len != 5u) {
        return slave_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, rsp);
    }

    addr = modbus_get_u16_be(&req[1]);
    qty = modbus_get_u16_be(&req[3]);

    if ((qty < 1u) || (qty > MODBUS_MAX_READ_BITS)) {
        return slave_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, rsp);
    }

    if (discrete) {
        map_n = slave->map->num_discrete_inputs;
        src = slave->map->discrete_inputs;
    } else {
        map_n = slave->map->num_coils;
        src = slave->map->coils;
    }

    if (slave_range_ok(addr, qty, map_n) == false) {
        return slave_exception(fc, MODBUS_EX_ILLEGAL_DATA_ADDRESS, rsp);
    }

    nbytes = (uint8_t)((qty + 7u) / 8u);
    rsp[0] = fc;
    rsp[1] = nbytes;
    (void)modbus_pack_bits(&src[addr], qty, &rsp[2]);

    return (uint16_t)(2u + (uint16_t)nbytes);
}

static uint16_t handle_read_regs(modbus_slave_t *slave,
                                 uint8_t fc,
                                 const uint8_t *req,
                                 uint16_t req_len,
                                 uint8_t *rsp,
                                 bool input)
{
    uint16_t addr;
    uint16_t qty;
    uint16_t map_n;
    const uint16_t *src;
    uint16_t i;

    if (req_len != 5u) {
        return slave_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, rsp);
    }

    addr = modbus_get_u16_be(&req[1]);
    qty = modbus_get_u16_be(&req[3]);

    if ((qty < 1u) || (qty > MODBUS_MAX_READ_REGS)) {
        return slave_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, rsp);
    }

    if (input) {
        map_n = slave->map->num_input_registers;
        src = slave->map->input_registers;
    } else {
        map_n = slave->map->num_holding_registers;
        src = slave->map->holding_registers;
    }

    if (slave_range_ok(addr, qty, map_n) == false) {
        return slave_exception(fc, MODBUS_EX_ILLEGAL_DATA_ADDRESS, rsp);
    }

    rsp[0] = fc;
    rsp[1] = (uint8_t)(qty * 2u);
    for (i = 0u; i < qty; i++) {
        modbus_put_u16_be(&rsp[(uint16_t)(2u + (i * 2u))], src[addr + i]);
    }

    return (uint16_t)(2u + (qty * 2u));
}

static uint16_t handle_write_single_coil(modbus_slave_t *slave,
                                         const uint8_t *req,
                                         uint16_t req_len,
                                         uint8_t *rsp)
{
    const uint8_t fc = MODBUS_FC_WRITE_SINGLE_COIL;
    uint16_t addr;
    uint16_t value;

    if (req_len != 5u) {
        return slave_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, rsp);
    }

    addr = modbus_get_u16_be(&req[1]);
    value = modbus_get_u16_be(&req[3]);

    if ((value != MODBUS_COIL_ON) && (value != MODBUS_COIL_OFF)) {
        return slave_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, rsp);
    }

    if (slave_range_ok(addr, 1u, slave->map->num_coils) == false) {
        return slave_exception(fc, MODBUS_EX_ILLEGAL_DATA_ADDRESS, rsp);
    }

    slave->map->coils[addr] = (value == MODBUS_COIL_ON) ? 1u : 0u;
    if (slave->on_write != NULL) {
        slave->on_write(slave, fc, addr, 1u, slave->user);
    }

    (void)memcpy(rsp, req, 5u);
    return 5u;
}

static uint16_t handle_write_single_register(modbus_slave_t *slave,
                                             const uint8_t *req,
                                             uint16_t req_len,
                                             uint8_t *rsp)
{
    const uint8_t fc = MODBUS_FC_WRITE_SINGLE_REGISTER;
    uint16_t addr;
    uint16_t value;

    if (req_len != 5u) {
        return slave_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, rsp);
    }

    addr = modbus_get_u16_be(&req[1]);
    value = modbus_get_u16_be(&req[3]);

    if (slave_range_ok(addr, 1u, slave->map->num_holding_registers) == false) {
        return slave_exception(fc, MODBUS_EX_ILLEGAL_DATA_ADDRESS, rsp);
    }

    slave->map->holding_registers[addr] = value;
    if (slave->on_write != NULL) {
        slave->on_write(slave, fc, addr, 1u, slave->user);
    }

    (void)memcpy(rsp, req, 5u);
    return 5u;
}

static uint16_t handle_write_multiple_coils(modbus_slave_t *slave,
                                            const uint8_t *req,
                                            uint16_t req_len,
                                            uint8_t *rsp)
{
    const uint8_t fc = MODBUS_FC_WRITE_MULTIPLE_COILS;
    uint16_t addr;
    uint16_t qty;
    uint8_t byte_count;
    uint16_t i;

    if (req_len < 6u) {
        return slave_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, rsp);
    }

    addr = modbus_get_u16_be(&req[1]);
    qty = modbus_get_u16_be(&req[3]);
    byte_count = req[5];

    if ((qty < 1u) || (qty > MODBUS_MAX_WRITE_BITS) ||
        (byte_count != (uint8_t)((qty + 7u) / 8u)) ||
        (req_len != (uint16_t)(6u + (uint16_t)byte_count))) {
        return slave_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, rsp);
    }

    if (slave_range_ok(addr, qty, slave->map->num_coils) == false) {
        return slave_exception(fc, MODBUS_EX_ILLEGAL_DATA_ADDRESS, rsp);
    }

    for (i = 0u; i < qty; i++) {
        slave->map->coils[addr + i] =
            (uint8_t)((req[(uint16_t)(6u + (i / 8u))] >> (i % 8u)) & 0x01u);
    }

    if (slave->on_write != NULL) {
        slave->on_write(slave, fc, addr, qty, slave->user);
    }

    rsp[0] = fc;
    modbus_put_u16_be(&rsp[1], addr);
    modbus_put_u16_be(&rsp[3], qty);
    return 5u;
}

static uint16_t handle_write_multiple_registers(modbus_slave_t *slave,
                                                const uint8_t *req,
                                                uint16_t req_len,
                                                uint8_t *rsp)
{
    const uint8_t fc = MODBUS_FC_WRITE_MULTIPLE_REGISTERS;
    uint16_t addr;
    uint16_t qty;
    uint8_t byte_count;
    uint16_t i;

    if (req_len < 6u) {
        return slave_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, rsp);
    }

    addr = modbus_get_u16_be(&req[1]);
    qty = modbus_get_u16_be(&req[3]);
    byte_count = req[5];

    if ((qty < 1u) || (qty > MODBUS_MAX_WRITE_REGS) ||
        (byte_count != (uint8_t)(qty * 2u)) ||
        (req_len != (uint16_t)(6u + (uint16_t)byte_count))) {
        return slave_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, rsp);
    }

    if (slave_range_ok(addr, qty, slave->map->num_holding_registers) == false) {
        return slave_exception(fc, MODBUS_EX_ILLEGAL_DATA_ADDRESS, rsp);
    }

    for (i = 0u; i < qty; i++) {
        slave->map->holding_registers[addr + i] =
            modbus_get_u16_be(&req[(uint16_t)(6u + (i * 2u))]);
    }

    if (slave->on_write != NULL) {
        slave->on_write(slave, fc, addr, qty, slave->user);
    }

    rsp[0] = fc;
    modbus_put_u16_be(&rsp[1], addr);
    modbus_put_u16_be(&rsp[3], qty);
    return 5u;
}

static uint16_t process_pdu(modbus_slave_t *slave,
                            const uint8_t *pdu,
                            uint16_t pdu_len,
                            uint8_t *rsp)
{
    uint8_t fc;
    uint16_t out_len = 0u;

    if ((pdu == NULL) || (pdu_len < 1u) || (rsp == NULL)) {
        return 0u;
    }

    fc = pdu[0];

    switch (fc) {
    case MODBUS_FC_READ_COILS:
        out_len = handle_read_bits(slave, fc, pdu, pdu_len, rsp, false);
        break;
    case MODBUS_FC_READ_DISCRETE_INPUTS:
        out_len = handle_read_bits(slave, fc, pdu, pdu_len, rsp, true);
        break;
    case MODBUS_FC_READ_HOLDING_REGISTERS:
        out_len = handle_read_regs(slave, fc, pdu, pdu_len, rsp, false);
        break;
    case MODBUS_FC_READ_INPUT_REGISTERS:
        out_len = handle_read_regs(slave, fc, pdu, pdu_len, rsp, true);
        break;
    case MODBUS_FC_WRITE_SINGLE_COIL:
        out_len = handle_write_single_coil(slave, pdu, pdu_len, rsp);
        break;
    case MODBUS_FC_WRITE_SINGLE_REGISTER:
        out_len = handle_write_single_register(slave, pdu, pdu_len, rsp);
        break;
    case MODBUS_FC_WRITE_MULTIPLE_COILS:
        out_len = handle_write_multiple_coils(slave, pdu, pdu_len, rsp);
        break;
    case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
        out_len = handle_write_multiple_registers(slave, pdu, pdu_len, rsp);
        break;
    default:
        out_len = slave_exception(fc, MODBUS_EX_ILLEGAL_FUNCTION, rsp);
        break;
    }

    return out_len;
}

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

modbus_status_t modbus_slave_init(modbus_slave_t *slave,
                                  uint8_t address,
                                  modbus_map_t *map,
                                  const modbus_port_t *port,
                                  uint32_t baud)
{
    uint32_t t15 = 1u;
    uint32_t t35 = 2u;

    if ((slave == NULL) || (map == NULL)) {
        return MODBUS_ERR_NULL_PTR;
    }

    if ((address != MODBUS_ADDR_BROADCAST) &&
        ((address < MODBUS_ADDR_MIN) || (address > MODBUS_ADDR_MAX))) {
        return MODBUS_ERR_INVALID_ARG;
    }

    (void)memset(slave, 0, sizeof(*slave));
    slave->address = address;
    slave->map = map;
    slave->port = port;
    modbus_rtu_timing_ms(baud, &t15, &t35);
    slave->byte_timeout_ms = t15;
    slave->frame_timeout_ms = t35;

    return MODBUS_OK;
}

modbus_status_t modbus_slave_handle(modbus_slave_t *slave,
                                    const uint8_t *adu,
                                    uint16_t adu_len,
                                    uint8_t *resp,
                                    uint16_t *resp_len,
                                    uint16_t resp_cap)
{
    uint8_t req_slave;
    const uint8_t *pdu;
    uint16_t pdu_len;
    uint8_t rsp_pdu[MODBUS_PDU_MAX_SIZE];
    uint16_t rsp_pdu_len;
    modbus_status_t st;
    bool broadcast;
    bool is_write;

    if ((slave == NULL) || (slave->map == NULL) || (adu == NULL) ||
        (resp == NULL) || (resp_len == NULL)) {
        return MODBUS_ERR_NULL_PTR;
    }

    st = modbus_rtu_parse(adu, adu_len, &req_slave, &pdu, &pdu_len);
    if (st != MODBUS_OK) {
        return st;
    }

    broadcast = (req_slave == MODBUS_ADDR_BROADCAST);
    if ((broadcast == false) && (req_slave != slave->address)) {
        *resp_len = 0u;
        return MODBUS_OK;
    }

    is_write = ((pdu[0] == MODBUS_FC_WRITE_SINGLE_COIL) ||
                (pdu[0] == MODBUS_FC_WRITE_SINGLE_REGISTER) ||
                (pdu[0] == MODBUS_FC_WRITE_MULTIPLE_COILS) ||
                (pdu[0] == MODBUS_FC_WRITE_MULTIPLE_REGISTERS));

    if ((broadcast == true) && (is_write == false)) {
        *resp_len = 0u;
        return MODBUS_OK;
    }

    rsp_pdu_len = process_pdu(slave, pdu, pdu_len, rsp_pdu);
    if (rsp_pdu_len == 0u) {
        return MODBUS_ERR_FRAME;
    }

    if (broadcast == true) {
        *resp_len = 0u;
        return MODBUS_OK;
    }

    {
        const uint16_t n = modbus_rtu_build(slave->address, rsp_pdu, rsp_pdu_len,
                                            resp, resp_cap);
        if (n == 0u) {
            return MODBUS_ERR_OVERFLOW;
        }
        *resp_len = n;
    }

    return MODBUS_OK;
}

modbus_status_t modbus_slave_poll(modbus_slave_t *slave)
{
    int32_t n;
    uint8_t resp[MODBUS_ADU_MAX_SIZE];
    uint16_t resp_len = 0u;
    modbus_status_t st;
    const modbus_port_t *port;

    if (slave == NULL) {
        return MODBUS_ERR_NULL_PTR;
    }

    port = slave->port;
    if ((port == NULL) || (port->uart_read == NULL) || (port->uart_write == NULL)) {
        return MODBUS_ERR_NULL_PTR;
    }

    n = port->uart_read(port->ctx, &slave->rx[0], 1u,
                        (uint32_t)(slave->frame_timeout_ms * 10u));
    if (n < 0) {
        return MODBUS_ERR_IO;
    }
    if (n == 0) {
        return MODBUS_ERR_TIMEOUT;
    }

    slave->rx_len = 1u;

    while (slave->rx_len < MODBUS_ADU_MAX_SIZE) {
        n = port->uart_read(port->ctx, &slave->rx[slave->rx_len], 1u,
                            slave->frame_timeout_ms);
        if (n < 0) {
            slave->rx_len = 0u;
            return MODBUS_ERR_IO;
        }
        if (n == 0) {
            break;
        }
        slave->rx_len = (uint16_t)(slave->rx_len + (uint16_t)n);
    }

    st = modbus_slave_handle(slave, slave->rx, slave->rx_len,
                             resp, &resp_len, (uint16_t)sizeof(resp));
    slave->rx_len = 0u;

    if (st != MODBUS_OK) {
        return st;
    }

    if (resp_len > 0u) {
        n = port->uart_write(port->ctx, resp, resp_len);
        if ((n < 0) || ((uint16_t)n != resp_len)) {
            return MODBUS_ERR_IO;
        }
    }

    return MODBUS_OK;
}

#endif /* MODBUS_CFG_ENABLE_SLAVE */
