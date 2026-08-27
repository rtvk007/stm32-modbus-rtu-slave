/**
 * @file    stm32_master_app.c
 * @brief   Example Modbus RTU master application for STM32 (CubeHAL).
 */
#include "modbus_master.h"
#include "modbus_stm32_port.h"

extern UART_HandleTypeDef huart2;

static modbus_stm32_port_t g_mb_port;
static modbus_master_t     g_master;

void Modbus_MasterApp_Init(void)
{
    (void)modbus_stm32_port_init(&g_mb_port, &huart2, 100u);
    (void)modbus_master_init(&g_master, &g_mb_port.port, 9600u);
}

/**
 * @brief Example transaction: write then read holding register 0 on slave 1.
 * @return Modbus status code.
 */
modbus_status_t Modbus_MasterApp_Demo(void)
{
    uint16_t regs[2];
    modbus_status_t st;

    st = modbus_master_write_single_register(&g_master, 1u, 0u, 0x1234u);
    if (st != MODBUS_OK) {
        return st;
    }

    st = modbus_master_read_holding_registers(&g_master, 1u, 0u, 2u, regs);
    return st;
}
