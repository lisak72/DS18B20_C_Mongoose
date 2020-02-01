/**
 *
 **/

#ifndef DS18B20_H
#define DS18B20_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "mgos_features.h"
#include "common/cs_dbg.h"
#include "mgos_init.h"
#include "mgos_sys_config.h"
#include "mgos_system.h"
#include "mgos_config.h"
#include "mgos_gpio.h"
#include "mgos_onewire.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define DEVICE_FAMILY_DS18B20 0x28
struct T** tow; //define array of sensors
static uint8_t countDevs=0;

//init - return count of DS18B20 sensors
uint8_t DS18B20_init(struct mgos_onewire *ow);

//return number of all devices discovered on onewire
uint8_t onewireGetCount(); //return number of discovered devices

//return number of DS18B20 devices discovered on onewire
uint8_t DS18B20GetCount();

//get temperature from device number num
float DS18B20_GetTempTNumber(uint8_t num);


//return count of sensors, fill rom[] codes
// uint8_t DS18B20CountSensors(struct mgos_onewire *ow);

//return address of device number i



// uint8_t* search_rom(struct mgos_onewire *ow);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DS18B20_H */