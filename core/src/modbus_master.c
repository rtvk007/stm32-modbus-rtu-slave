/**
 * @file    modbus_master.c
 * @brief   Modbus RTU master implementation (portable; uses modbus_port_t only).
 */
#include "modbus_master.h"

#if (MODBUS_CFG_ENABLE_MASTER == 1u)

#include "modbus_rtu.h"
#include "modbus_crc.h"

#include <string.h>

/* -------------------------------------------------------------------------- */
/* Internal helpers                                                           */
/* -------------------------------------------------------------------------- */

/**
 * @brief Busy-wait using port tick (wrap-safe).
 */
static void master_delay_ms(const modbus_port_t *port, uint32_t ms)
{
    uint32_t start;
    uint32_t now;
    uint32_t elapsed;

    if ((port == NULL) || (port->get_tick_ms == NULL) || (ms == 0u)) {
        return;
    }

    start = port->get_tick_ms(port->ctx);
    do {
        now = port->get_tick_ms(port->ctx);
        elapsed = now - start; /* unsigned wrap-safe */
    } while (elapsed < ms);
}

/**
 * @brief Compare two buffers of equal length.
 * @return true if equal.
 */
static bool master_buf_eq(const uint8_t *a, const uint8_t *b, uint16_t len)
{
    uint16_t i;
    bool eq = true;

    for (i = 0u; i < len; i++) {
        if (a[i] != b[i]) {
            eq = false;
            break;
        }
    }

    return eq;
}

/**
 * @brief Send request PDU and receive response PDU.
 */
static modbus_status_t master_transceive(modbus_master_t *master,
                                         uint8_t slave,
                                         const uint8_t *req_pdu,
                                         uint16_t req_pdu_len,
                                         uint8_t *rsp_pdu,
                                         uint16_t *rsp_pdu_len,
                                         uint16_t rsp_pdu_cap)
{
    uint8_t adu[MODBUS_ADU_MAX_SIZE];
    uint8_t rx[MODBUS_ADU_MAX_SIZE];
    uint16_t adu_len;
    uint16_t rx_len = 0u;
    int32_t n;
    uint8_t rsp_slave;
    const uint8_t *pdu;
    uint16_t pdu_len;
    modbus_status_t st;
    const modbus_port_t *port;

    if ((master == NULL) || (req_pdu == NULL) || (rsp_pdu == NULL) ||
        (rsp_pdu_len == NULL)) {
        return MODBUS_ERR_NULL_PTR;
    }

    port = master->port;
    if ((port == NULL) || (port->uart_write == NULL) ||
        (port->uart_read == NULL) || (port->get_tick_ms == NULL)) {
        return MODBUS_ERR_NULL_PTR;
    }

    if (slave > MODBUS_ADDR_MAX) {
        return MODBUS_ERR_INVALID_ARG;
    }

    adu_len = modbus_rtu_build(slave, req_pdu, req_pdu_len, adu,
                               (uint16_t)sizeof(adu));
    if (adu_len == 0u) {
        return MODBUS_ERR_FRAME;
    }

    if (port->uart_flush != NULL) {
        port->uart_flush(port->ctx);
    }

    n = port->uart_write(port->ctx, adu, adu_len);
    if ((n < 0) || ((uint16_t)n != adu_len)) {
        return MODBUS_ERR_IO;
    }

    /* Broadcast: no response expected */
    if (slave == MODBUS_ADDR_BROADCAST) {
        master_delay_ms(port, master->turnaround_ms);
        *rsp_pdu_len = 0u;
        return MODBUS_OK;
    }

    master_delay_ms(port, master->turnaround_ms);

    while (rx_len < (uint16_t)sizeof(rx)) {
        const uint32_t to = (rx_len == 0u) ? master->response_timeout_ms
                                           : master->byte_timeout_ms;
        n = port->uart_read(port->ctx, &rx[rx_len], 1u, to);
        if (n < 0) {
            return MODBUS_ERR_IO;
        }
        if (n == 0) {
            if (rx_len == 0u) {
                return MODBUS_ERR_NO_RESPONSE;
            }
            break;
        }
        rx_len = (uint16_t)(rx_len + (uint16_t)n);
    }

    if (rx_len < 4u) {
        return MODBUS_ERR_FRAME;
    }

    st = modbus_rtu_parse(rx, rx_len, &rsp_slave, &pdu, &pdu_len);
    if (st != MODBUS_OK) {
        return st;
    }

    if (rsp_slave != slave) {
        return MODBUS_ERR_FRAME;
    }

    if (pdu_len < 2u) {
        return MODBUS_ERR_FRAME;
    }

    if ((pdu[0] & MODBUS_FC_EXCEPTION_FLAG) != 0u) {
        master->last_exception = pdu[1];
        return MODBUS_ERR_EXCEPTION;
    }

    if (pdu_len > rsp_pdu_cap) {
        return MODBUS_ERR_OVERFLOW;
    }

    (void)memcpy(rsp_pdu, pdu, (size_t)pdu_len);
    *rsp_pdu_len = pdu_len;

    return MODBUS_OK;
}

static modbus_status_t master_read_bits(modbus_master_t *master,
                                        uint8_t slave,
                                        uint8_t fc,
                                        uint16_t addr,
                                        uint16_t quantity,
                                        uint8_t *dest)
{
    uint8_t req[5];
    uint8_t rsp[MODBUS_PDU_MAX_SIZE];
    uint16_t rsp_len = 0u;
    modbus_status_t st;
    uint8_t byte_count;

    if (dest == NULL) {
        return MODBUS_ERR_NULL_PTR;
    }
    if ((quantity == 0u) || (quantity > MODBUS_MAX_READ_BITS)) {
        return MODBUS_ERR_INVALID_ARG;
    }

    req[0] = fc;
    modbus_put_u16_be(&req[1], addr);
    modbus_put_u16_be(&req[3], quantity);

    st = master_transceive(master, slave, req, 5u, rsp, &rsp_len,
                           (uint16_t)sizeof(rsp));
    if (st != MODBUS_OK) {
        return st;
    }

    if ((rsp_len < 2u) || (rsp[0] != fc)) {
        return MODBUS_ERR_FRAME;
    }

    byte_count = rsp[1];
    if ((byte_count != (uint8_t)((quantity + 7u) / 8u)) ||
        (rsp_len != (uint16_t)(2u + (uint16_t)byte_count))) {
        return MODBUS_ERR_FRAME;
    }

    modbus_unpack_bits(&rsp[2], quantity, dest);
    return MODBUS_OK;
}

static modbus_status_t master_read_regs(modbus_master_t *master,
                                        uint8_t slave,
                                        uint8_t fc,
                                        uint16_t addr,
                                        uint16_t quantity,
                                        uint16_t *dest)
{
    uint8_t req[5];
    uint8_t rsp[MODBUS_PDU_MAX_SIZE];
    uint16_t rsp_len = 0u;
    modbus_status_t st;
    uint8_t byte_count;
    uint16_t i;

    if (dest == NULL) {
        return MODBUS_ERR_NULL_PTR;
    }
    if ((quantity == 0u) || (quantity > MODBUS_MAX_READ_REGS)) {
        return MODBUS_ERR_INVALID_ARG;
    }

    req[0] = fc;
    modbus_put_u16_be(&req[1], addr);
    modbus_put_u16_be(&req[3], quantity);

    st = master_transceive(master, slave, req, 5u, rsp, &rsp_len,
                           (uint16_t)sizeof(rsp));
    if (st != MODBUS_OK) {
        return st;
    }

    if ((rsp_len < 2u) || (rsp[0] != fc)) {
        return MODBUS_ERR_FRAME;
    }

    byte_count = rsp[1];
    if ((byte_count != (uint8_t)(quantity * 2u)) ||
        (rsp_len != (uint16_t)(2u + (uint16_t)byte_count))) {
        return MODBUS_ERR_FRAME;
    }

    for (i = 0u; i < quantity; i++) {
        dest[i] = modbus_get_u16_be(&rsp[(uint16_t)(2u + (i * 2u))]);
    }

    return MODBUS_OK;
}

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

modbus_status_t modbus_master_init(modbus_master_t *master,
                                   const modbus_port_t *port,
                                   uint32_t baud)
{
    uint32_t t15 = 1u;
    uint32_t t35 = 2u;

    if ((master == NULL) || (port == NULL)) {
        return MODBUS_ERR_NULL_PTR;
    }

    (void)memset(master, 0, sizeof(*master));
    master->port = port;
    modbus_rtu_timing_ms(baud, &t15, &t35);
    master->response_timeout_ms = MODBUS_CFG_DEFAULT_RESPONSE_TIMEOUT_MS;
    master->byte_timeout_ms = t15;
    master->turnaround_ms = t35;
    master->last_exception = 0u;

    return MODBUS_OK;
}

modbus_status_t modbus_master_read_coils(modbus_master_t *master,
                                         uint8_t slave,
                                         uint16_t addr,
                                         uint16_t quantity,
                                         uint8_t *dest)
{
    return master_read_bits(master, slave, MODBUS_FC_READ_COILS,
                            addr, quantity, dest);
}

modbus_status_t modbus_master_read_discrete_inputs(modbus_master_t *master,
                                                   uint8_t slave,
                                                   uint16_t addr,
                                                   uint16_t quantity,
                                                   uint8_t *dest)
{
    return master_read_bits(master, slave, MODBUS_FC_READ_DISCRETE_INPUTS,
                            addr, quantity, dest);
}

modbus_status_t modbus_master_read_holding_registers(modbus_master_t *master,
                                                     uint8_t slave,
                                                     uint16_t addr,
                                                     uint16_t quantity,
                                                     uint16_t *dest)
{
    return master_read_regs(master, slave, MODBUS_FC_READ_HOLDING_REGISTERS,
                            addr, quantity, dest);
}

modbus_status_t modbus_master_read_input_registers(modbus_master_t *master,
                                                   uint8_t slave,
                                                   uint16_t addr,
                                                   uint16_t quantity,
                                                   uint16_t *dest)
{
    return master_read_regs(master, slave, MODBUS_FC_READ_INPUT_REGISTERS,
                            addr, quantity, dest);
}

modbus_status_t modbus_master_write_single_coil(modbus_master_t *master,
                                                uint8_t slave,
                                                uint16_t addr,
                                                bool on)
{
    uint8_t req[5];
    uint8_t rsp[MODBUS_PDU_MAX_SIZE];
    uint16_t rsp_len = 0u;
    modbus_status_t st;

    req[0] = MODBUS_FC_WRITE_SINGLE_COIL;
    modbus_put_u16_be(&req[1], addr);
    modbus_put_u16_be(&req[3], on ? (uint16_t)MODBUS_COIL_ON
                                   : (uint16_t)MODBUS_COIL_OFF);

    st = master_transceive(master, slave, req, 5u, rsp, &rsp_len,
                           (uint16_t)sizeof(rsp));
    if (st != MODBUS_OK) {
        return st;
    }

    if ((rsp_len != 5u) || (master_buf_eq(req, rsp, 5u) == false)) {
        return MODBUS_ERR_FRAME;
    }

    return MODBUS_OK;
}

modbus_status_t modbus_master_write_single_register(modbus_master_t *master,
                                                    uint8_t slave,
                                                    uint16_t addr,
                                                    uint16_t value)
{
    uint8_t req[5];
    uint8_t rsp[MODBUS_PDU_MAX_SIZE];
    uint16_t rsp_len = 0u;
    modbus_status_t st;

    req[0] = MODBUS_FC_WRITE_SINGLE_REGISTER;
    modbus_put_u16_be(&req[1], addr);
    modbus_put_u16_be(&req[3], value);

    st = master_transceive(master, slave, req, 5u, rsp, &rsp_len,
                           (uint16_t)sizeof(rsp));
    if (st != MODBUS_OK) {
        return st;
    }

    if ((rsp_len != 5u) || (master_buf_eq(req, rsp, 5u) == false)) {
        return MODBUS_ERR_FRAME;
    }

    return MODBUS_OK;
}

modbus_status_t modbus_master_write_multiple_coils(modbus_master_t *master,
                                                   uint8_t slave,
                                                   uint16_t addr,
                                                   uint16_t quantity,
                                                   const uint8_t *src)
{
    uint8_t req[MODBUS_PDU_MAX_SIZE];
    uint8_t rsp[MODBUS_PDU_MAX_SIZE];
    uint16_t rsp_len = 0u;
    uint16_t nbytes;
    uint16_t req_len;
    modbus_status_t st;

    if (src == NULL) {
        return MODBUS_ERR_NULL_PTR;
    }
    if ((quantity == 0u) || (quantity > MODBUS_MAX_WRITE_BITS)) {
        return MODBUS_ERR_INVALID_ARG;
    }

    req[0] = MODBUS_FC_WRITE_MULTIPLE_COILS;
    modbus_put_u16_be(&req[1], addr);
    modbus_put_u16_be(&req[3], quantity);
    nbytes = modbus_pack_bits(src, quantity, &req[6]);
    req[5] = (uint8_t)nbytes;
    req_len = (uint16_t)(6u + nbytes);

    st = master_transceive(master, slave, req, req_len, rsp, &rsp_len,
                           (uint16_t)sizeof(rsp));
    if (st != MODBUS_OK) {
        return st;
    }

    if ((rsp_len != 5u) ||
        (rsp[0] != MODBUS_FC_WRITE_MULTIPLE_COILS) ||
        (modbus_get_u16_be(&rsp[1]) != addr) ||
        (modbus_get_u16_be(&rsp[3]) != quantity)) {
        return MODBUS_ERR_FRAME;
    }

    return MODBUS_OK;
}

modbus_status_t modbus_master_write_multiple_registers(modbus_master_t *master,
                                                       uint8_t slave,
                                                       uint16_t addr,
                                                       uint16_t quantity,
                                                       const uint16_t *src)
{
    uint8_t req[MODBUS_PDU_MAX_SIZE];
    uint8_t rsp[MODBUS_PDU_MAX_SIZE];
    uint16_t rsp_len = 0u;
    uint16_t i;
    uint16_t req_len;
    modbus_status_t st;

    if (src == NULL) {
        return MODBUS_ERR_NULL_PTR;
    }
    if ((quantity == 0u) || (quantity > MODBUS_MAX_WRITE_REGS)) {
        return MODBUS_ERR_INVALID_ARG;
    }

    req[0] = MODBUS_FC_WRITE_MULTIPLE_REGISTERS;
    modbus_put_u16_be(&req[1], addr);
    modbus_put_u16_be(&req[3], quantity);
    req[5] = (uint8_t)(quantity * 2u);
    for (i = 0u; i < quantity; i++) {
        modbus_put_u16_be(&req[(uint16_t)(6u + (i * 2u))], src[i]);
    }
    req_len = (uint16_t)(6u + (quantity * 2u));

    st = master_transceive(master, slave, req, req_len, rsp, &rsp_len,
                           (uint16_t)sizeof(rsp));
    if (st != MODBUS_OK) {
        return st;
    }

    if ((rsp_len != 5u) ||
        (rsp[0] != MODBUS_FC_WRITE_MULTIPLE_REGISTERS) ||
        (modbus_get_u16_be(&rsp[1]) != addr) ||
        (modbus_get_u16_be(&rsp[3]) != quantity)) {
        return MODBUS_ERR_FRAME;
    }

    return MODBUS_OK;
}

#endif /* MODBUS_CFG_ENABLE_MASTER */
