/**
 * @file    stm32_slave_app.c
 * @brief   Example Modbus RTU slave application for STM32 (CubeHAL).
 *
 * Drop into a CubeMX project together with:
 *   - core/* sources
 *   - port/include on the include path
 *   - config/ on the include path
 *   - platform/stm32/*
 *
 * Wire RS-485 DE/RE in your board file if using a transceiver.
 */
#include "modbus_slave.h"
#include "modbus_stm32_port.h"

/* CubeMX exports these */
extern UART_HandleTypeDef huart1;

static modbus_stm32_port_t g_mb_port;
static modbus_map_t        g_map;
static modbus_slave_t      g_slave;

static void app_on_write(modbus_slave_t *slave,
                         uint8_t fc,
                         uint16_t addr,
                         uint16_t quantity,
                         void *user)
{
    (void)slave;
    (void)user;
    (void)fc;
    (void)addr;
    (void)quantity;
    /* Hook: update actuators / EEPROM / etc. */
}

/**
 * @brief Call once after peripheral init (e.g. from main after MX_*_Init).
 */
void Modbus_SlaveApp_Init(void)
{
    (void)modbus_map_init(&g_map);
    g_map.holding_registers[0] = 0x0001u;
    g_map.input_registers[0] = 0x00AAu;
    g_map.coils[0] = 1u;

    (void)modbus_stm32_port_init(&g_mb_port, &huart1, 100u);
    (void)modbus_slave_init(&g_slave, 1u, &g_map, &g_mb_port.port, 9600u);
    g_slave.on_write = app_on_write;
}

/**
 * @brief Call periodically from the main super-loop or an RTOS task.
 */
void Modbus_SlaveApp_Poll(void)
{
    (void)modbus_slave_poll(&g_slave);
}
