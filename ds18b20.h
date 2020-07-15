/**
 * Author: Jiri Liska, Trebon, Czech Republic, liskaj72@gmail.com
 * 07/2020
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
#include "mgos_time.h"
#include "mgos_onewire.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#define DEVICE_FAMILY_DS18B20 0x28
#define max_num_of_sensors 32 //maximal number of sensors on one ow line 32, for more increase number, max is 255
struct T* tow[max_num_of_sensors]; //define array of sensors, 
static uint8_t countDevs=0;  //global variable number of founded devices
const static bool debug=0;  //debug mode on/off

//private funct
float DS_get_temp(struct T* ts);
uint8_t addToDevField(uint8_t devrom[], struct mgos_onewire *ow);
char *byteToHexF(uint8_t byteOfAddress);


//init - return count of DS18B20 sensors
uint8_t DS18B20_init(struct mgos_onewire *ow);

//return number of all devices discovered on onewire
uint8_t onewireGetCount(); //return number of discovered devices

//return number of DS18B20 devices discovered on onewire
uint8_t DS18B20GetCount();

//get temperature from device number num
float DS18B20_GetTempTNumber(uint8_t num);

//return number of device by addr romaddr
//parameter romaddr example uint8_t address[]={0xdd,0xdd,0xdd,0xdd,0xdd,0xdd};
uint8_t DS18B20_GetNumbyRom(uint8_t *romaddr); 

//return temperature by addr romaddr
//parameter romaddr example uint8_t address[]={0xdd,0xdd,0xdd,0xdd,0xdd,0xdd};
float DS18B20_GetTempTByRom(uint8_t *romaddress);

/*
list all 6bit addresses of discovered devices
return pointer to mallocated array, must be freed, example of use:
  char** listDev;
  listDev=DS18B20ListAddresses();
  for(uint8_t i=0; listDev[i]!= NULL; i++){
      LOG(LL_INFO, ("Device %d addr: %s", i+1, listDev[i]));
      free(listDev[i]);
  }
  free(listDev);
*/
char** DS18B20ListAddresses();


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DS18B20_H */