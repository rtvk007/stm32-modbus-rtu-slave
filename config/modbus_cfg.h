/**
 * @file    modbus_cfg.h
 * @brief   Compile-time configuration for the Modbus RTU stack (C99 / MISRA-friendly).
 * @note    Edit this file (or override via -D) when integrating on a target MCU.
 *          This header must remain free of any MCU/HAL includes.
 */
#ifndef MODBUS_CFG_H
#define MODBUS_CFG_H

/* -------------------------------------------------------------------------- */
/* Data model sizes (PDU 0-based addressing)                                  */
/* -------------------------------------------------------------------------- */
#ifndef MODBUS_CFG_NUM_COILS
#define MODBUS_CFG_NUM_COILS            (2000u)
#endif

#ifndef MODBUS_CFG_NUM_DISCRETE_INPUTS
#define MODBUS_CFG_NUM_DISCRETE_INPUTS  (2000u)
#endif

#ifndef MODBUS_CFG_NUM_HOLDING_REGS
#define MODBUS_CFG_NUM_HOLDING_REGS     (125u)
#endif

#ifndef MODBUS_CFG_NUM_INPUT_REGS
#define MODBUS_CFG_NUM_INPUT_REGS       (125u)
#endif

/* -------------------------------------------------------------------------- */
/* Timing defaults (ms) — may be overridden at runtime on master/slave objs   */
/* -------------------------------------------------------------------------- */
#ifndef MODBUS_CFG_DEFAULT_RESPONSE_TIMEOUT_MS
#define MODBUS_CFG_DEFAULT_RESPONSE_TIMEOUT_MS  (1000u)
#endif

#ifndef MODBUS_CFG_DEFAULT_BAUD
#define MODBUS_CFG_DEFAULT_BAUD                 (9600u)
#endif

/* -------------------------------------------------------------------------- */
/* Feature switches (1u = enabled)                                            */
/* -------------------------------------------------------------------------- */
#ifndef MODBUS_CFG_ENABLE_MASTER
#define MODBUS_CFG_ENABLE_MASTER                (1u)
#endif

#ifndef MODBUS_CFG_ENABLE_SLAVE
#define MODBUS_CFG_ENABLE_SLAVE                 (1u)
#endif

#endif /* MODBUS_CFG_H */
