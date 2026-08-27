/**
 * @file    selftest.c
 * @brief   Host-side unit checks for portable core (no MCU / UART required).
 */
#include "modbus_crc.h"
#include "modbus_slave.h"
#include "modbus_rtu.h"

#include <stdio.h>

static int g_failures;

static void expect(int cond, const char *msg)
{
    if (cond == 0) {
        (void)printf("FAIL: %s\n", msg);
        g_failures++;
    } else {
        (void)printf("OK:   %s\n", msg);
    }
}

int main(void)
{
    /* Wire CRC_lo=0xC4 CRC_hi=0x0B => value 0x0BC4 */
    {
        const uint8_t frame[] = {0x01u, 0x03u, 0x00u, 0x00u, 0x00u, 0x02u};
        const uint16_t crc = modbus_crc16(frame, 6u);
        expect(crc == 0x0BC4u, "CRC-16 known vector FC03");
    }

    {
        modbus_map_t map;
        modbus_slave_t slave;
        const uint8_t req_pdu[] = {0x03u, 0x00u, 0x00u, 0x00u, 0x02u};
        uint8_t adu[32];
        uint8_t resp[64];
        uint16_t resp_len = 0u;
        uint16_t adu_len;
        modbus_status_t st;

        (void)modbus_map_init(&map);
        map.holding_registers[0] = 0x1234u;
        map.holding_registers[1] = 0x5678u;
        (void)modbus_slave_init(&slave, 1u, &map, NULL, 9600u);

        adu_len = modbus_rtu_build(1u, req_pdu, 5u, adu, (uint16_t)sizeof(adu));
        expect(adu_len == 8u, "build ADU length");

        st = modbus_slave_handle(&slave, adu, adu_len, resp, &resp_len,
                                 (uint16_t)sizeof(resp));
        expect(st == MODBUS_OK, "slave handle OK");
        expect(resp_len == 9u, "response length");
        expect((resp[0] == 1u) && (resp[1] == 0x03u) && (resp[2] == 4u),
               "response header");
        expect((resp[3] == 0x12u) && (resp[4] == 0x34u), "reg0 BE");
        expect((resp[5] == 0x56u) && (resp[6] == 0x78u), "reg1 BE");
        expect(modbus_crc16_check(resp, resp_len) == true, "response CRC");
    }

    {
        modbus_map_t map;
        modbus_slave_t slave;
        const uint8_t req_pdu[] = {0x06u, 0x00u, 0x00u, 0xABu, 0xCDu};
        uint8_t adu[32];
        uint8_t resp[64];
        uint16_t resp_len = 0u;
        uint16_t adu_len;

        (void)modbus_map_init(&map);
        (void)modbus_slave_init(&slave, 1u, &map, NULL, 9600u);
        adu_len = modbus_rtu_build(1u, req_pdu, 5u, adu, (uint16_t)sizeof(adu));
        expect(modbus_slave_handle(&slave, adu, adu_len, resp, &resp_len,
                                   (uint16_t)sizeof(resp)) == MODBUS_OK,
               "write single register");
        expect(map.holding_registers[0] == 0xABCDu, "holding[0] updated");
    }

    (void)printf("\n%s (%d failures)\n",
                 (g_failures != 0) ? "FAILED" : "ALL PASSED",
                 g_failures);
    return (g_failures != 0) ? 1 : 0;
}
