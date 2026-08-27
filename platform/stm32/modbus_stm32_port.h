/**
 * @file    modbus_stm32_port.h
 * @brief   STM32 HAL adapter for modbus_port_t (Layer 0 — BSP).
 *
 * @details Requires STM32Cube HAL (UART + HAL_GetTick).
 *          Include paths / defines come from your CubeMX project:
 *          - stm32XXxx_hal.h (via main.h typically)
 *
 * Integration steps:
 * 1. Configure USART in CubeMX (8E1 recommended for Modbus).
 * 2. Add this file + modbus_stm32_port.c to the project.
 * 3. Call modbus_stm32_port_init() after MX_USARTx_UART_Init().
 * 4. Pass &handle.port to modbus_master_init / modbus_slave_init.
 *
 * To move to another MCU: replace this folder with platform/<mcu>/ —
 * do not change core/ sources.
 */
#ifndef MODBUS_STM32_PORT_H
#define MODBUS_STM32_PORT_H

#include "modbus_port.h"

/* Bring in the device HAL. Prefer project main.h which includes the right chip. */
#if defined(USE_HAL_DRIVER)
#include "main.h"
#else
/*
 * Fallback: user must provide UART_HandleTypeDef and HAL prototypes.
 * Uncomment / adapt the include for your family if not using main.h:
 *   #include "stm32f4xx_hal.h"
 */
#include "main.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief STM32-specific context (static allocation — no malloc).
 */
typedef struct {
    UART_HandleTypeDef *huart;   /**< Cube-generated UART handle */
    uint32_t            tx_timeout_ms;
    modbus_port_t       port;    /**< Filled by init; pass &port to core */
} modbus_stm32_port_t;

/**
 * @brief Bind an STM32 UART handle to a Modbus port object.
 * @param[in,out] ctx             Static context (caller-owned).
 * @param[in]     huart           Initialised HAL UART handle (e.g. &huart1).
 * @param[in]     tx_timeout_ms   HAL_UART_Transmit timeout.
 * @return 0 on success, -1 on null arguments.
 */
int32_t modbus_stm32_port_init(modbus_stm32_port_t *ctx,
                               UART_HandleTypeDef *huart,
                               uint32_t tx_timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_STM32_PORT_H */
