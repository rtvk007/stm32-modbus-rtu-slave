/**
 * @file    modbus_map.h
 * @brief   Application data model (coils / discrete / holding / input).
 * @note    Hardware-independent storage used by the slave (and optionally app).
 */
#ifndef MODBUS_MAP_H
#define MODBUS_MAP_H

#include "modbus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Slave process-image / register map.
 * @note  Coil and discrete arrays store 0u or 1u per element (not packed).
 */
typedef struct {
    uint8_t  coils[MODBUS_CFG_NUM_COILS];
    uint8_t  discrete_inputs[MODBUS_CFG_NUM_DISCRETE_INPUTS];
    uint16_t holding_registers[MODBUS_CFG_NUM_HOLDING_REGS];
    uint16_t input_registers[MODBUS_CFG_NUM_INPUT_REGS];
    uint16_t num_coils;
    uint16_t num_discrete_inputs;
    uint16_t num_holding_registers;
    uint16_t num_input_registers;
} modbus_map_t;

/**
 * @brief Zero the map and set counts to configured maxima.
 * @param[in,out] map  Map instance (must not be NULL).
 * @return MODBUS_OK or MODBUS_ERR_NULL_PTR.
 */
modbus_status_t modbus_map_init(modbus_map_t *map);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_MAP_H */
