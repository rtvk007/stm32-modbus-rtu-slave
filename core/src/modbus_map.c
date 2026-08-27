/**
 * @file    modbus_map.c
 * @brief   Data map initialisation (portable core).
 */
#include "modbus_map.h"

modbus_status_t modbus_map_init(modbus_map_t *map)
{
    uint16_t i;

    if (map == NULL) {
        return MODBUS_ERR_NULL_PTR;
    }

    for (i = 0u; i < (uint16_t)MODBUS_CFG_NUM_COILS; i++) {
        map->coils[i] = 0u;
    }
    for (i = 0u; i < (uint16_t)MODBUS_CFG_NUM_DISCRETE_INPUTS; i++) {
        map->discrete_inputs[i] = 0u;
    }
    for (i = 0u; i < (uint16_t)MODBUS_CFG_NUM_HOLDING_REGS; i++) {
        map->holding_registers[i] = 0u;
    }
    for (i = 0u; i < (uint16_t)MODBUS_CFG_NUM_INPUT_REGS; i++) {
        map->input_registers[i] = 0u;
    }

    map->num_coils = (uint16_t)MODBUS_CFG_NUM_COILS;
    map->num_discrete_inputs = (uint16_t)MODBUS_CFG_NUM_DISCRETE_INPUTS;
    map->num_holding_registers = (uint16_t)MODBUS_CFG_NUM_HOLDING_REGS;
    map->num_input_registers = (uint16_t)MODBUS_CFG_NUM_INPUT_REGS;

    return MODBUS_OK;
}
