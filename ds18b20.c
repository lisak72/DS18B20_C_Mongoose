/*
 * Author: Jiri Liska, Trebon, Czech Republic, liskaj72@gmail.com
 * 07/2020
*/



#include "ds18b20.h"

struct T{
  uint8_t number; //1-255
  uint8_t *device_address;//=rom =is 64bit sensor address
  uint8_t devaddr[6];
  uint8_t crc;
  char char_address[64]; //address of device in hexa chars //only 6 device-specific bytes
  bool isDs18b20; //is device ds18b20?
  struct mgos_onewire *onewire;
};

const uint8_t REG_CONF_RESOLUTION_9BIT= 0x00;
const uint8_t REG_CONF_RESOLUTION_10BIT= 0x20;
const uint8_t REG_CONF_RESOLUTION_11BIT= 0x40;
const uint8_t REG_CONF_RESOLUTION_MASK= 0x60; //only bit 5 and 6 are used for conf resolution
const uint8_t CMD_CONVERT_T= 0x44; //initiate temperature conversion
const uint8_t CMD_READ_SCRATCHPAD= 0xBE;
const uint8_t DATA_TEMP_LSB= 0; //lsb byte 0 from scratchpad
const uint8_t DATA_TEMP_MSB= 1; //msb byte 1 from scratchpad
const uint8_t DATA_REG_CONF= 4; //byte 4 of scratchpad is the configuration register
const uint8_t DATA_SCRATCHPAD_SIZE= 9; //9 data bytes scratchpad size

//private funct, "constructor" of struct T
uint8_t addToDevField(uint8_t *devrom, struct mgos_onewire *ow){ //add memory for one sensor to array of sensors, returns last access field item index
  if(debug) LOG(LL_INFO,("addToDevField started.."));
  //*tow=(struct T*)realloc(tow,(sizeof(*tow)+sizeof(struct T*)));
    if(debug) LOG(LL_INFO,("sizeof(*tow) is now=%i \n",sizeof(*tow)));
  tow[countDevs]=(struct T*)malloc(sizeof(struct T));
  if(debug) LOG(LL_INFO,("pointer 001 \n"));
  tow[countDevs]->device_address=(uint8_t*)malloc(64);
  if(debug) LOG(LL_INFO,("addToDevField, memory allocated --note001.. tow=%i, *tow=%i, tow[%i]=%i, *tow[%i]=%i sizeof(struct T)= %i memory.", sizeof(tow), sizeof(*tow), countDevs, sizeof(tow[countDevs]), countDevs, sizeof(*tow[countDevs]),sizeof(struct T)));
      for(int c=0;c<8;c++){
        tow[countDevs]->device_address[c]=devrom[c];     //devrom cpy
      }
    if(debug) LOG(LL_INFO,("add T addr --note001:%x, device address addr:%x", (uint32_t) tow[countDevs],(uint32_t) tow[countDevs]->device_address));
  tow[countDevs]->onewire=ow;
  if(tow[countDevs]->device_address[0]==DEVICE_FAMILY_DS18B20) tow[countDevs]->isDs18b20=true;
    else tow[countDevs]->isDs18b20=false;
    char buf[64];
  if(debug) LOG(LL_INFO,("addToDevField buf.."));
  sprintf(buf,"%x:%x:%x:%x:%x:%x", tow[countDevs]->device_address[1], tow[countDevs]->device_address[2], tow[countDevs]->device_address[3], tow[countDevs]->device_address[4], tow[countDevs]->device_address[5], tow[countDevs]->device_address[6]);//bit1,2,3,4,5,6
    tow[countDevs]->devaddr[0]=tow[countDevs]->device_address[1];
    tow[countDevs]->devaddr[1]=tow[countDevs]->device_address[2];
    tow[countDevs]->devaddr[2]=tow[countDevs]->device_address[3];
    tow[countDevs]->devaddr[3]=tow[countDevs]->device_address[4];
    tow[countDevs]->devaddr[4]=tow[countDevs]->device_address[5];
    tow[countDevs]->devaddr[5]=tow[countDevs]->device_address[6];
    tow[countDevs]->crc=tow[countDevs]->device_address[7];
  strcpy(tow[countDevs]->char_address,buf);
  tow[countDevs]->number=(countDevs+1);
  if(debug) LOG(LL_INFO,("add dev number %i addr:%x Address: %s",tow[countDevs]->number, (uint32_t) tow[countDevs], tow[countDevs]->char_address));
  
  if(debug) LOG(LL_INFO,("addToDevField finished ok!"));
  return countDevs++;
}

//public funct
uint8_t onewireGetCount(){ //return number of discovered devices
 return countDevs;
}

//public funct
uint8_t DS18B20GetCount(){
  if(countDevs==0) return 0; //no init or no device
  uint8_t counter=0, counterDS=0;
  while (counter<countDevs)
  {
    if(tow[counter]->isDs18b20) counterDS++;
    counter++;
  }
  if(debug) LOG(LL_INFO,("DS18B20GetCount before return ok!\n"));
  return counter;
}

//public f
float DS18B20_GetTempTNumber(uint8_t num){
  if (countDevs==0 || num>countDevs) return -999;
  float temp=-998;
  for(uint8_t i=0; i<countDevs; i++){
    if((tow[i]->isDs18b20) && (tow[i]->number==num)){
      temp=DS_get_temp(tow[i]);
      break;
    }
  }
  return temp;
}

// public f
uint8_t DS18B20_GetNumbyRom(uint8_t *romaddr){ //only 6bytes is device-specific address
  if (countDevs==0) return 0; //no devices or no init
  uint8_t idevaddr[6];
  for (uint8_t i=0; i<6; i++){
    idevaddr[i]=romaddr[i];
  }
  if(debug) LOG(LL_INFO,("given rom addr> %x:%x:%x:%x:%x:%x",idevaddr[0], idevaddr[1], idevaddr[2], idevaddr[3], idevaddr[4], idevaddr[5]));
    uint8_t ret=0;
  for(int i=0;i<countDevs;i++){
      if(tow[i]->devaddr[0]!=idevaddr[0]) continue;
      if(tow[i]->devaddr[1]!=idevaddr[1]) continue;
      if(tow[i]->devaddr[2]!=idevaddr[2]) continue;
      if(tow[i]->devaddr[3]!=idevaddr[3]) continue;
      if(tow[i]->devaddr[4]!=idevaddr[4]) continue;
      if(tow[i]->devaddr[5]!=idevaddr[5]) continue;
      else ret=tow[i]->number;
      if(debug) LOG(LL_INFO,("GetNumByRom iteration %i",i));
  }
  return ret;
}

//public f
float DS18B20_GetTempTByRom(uint8_t *romaddress){
  return(DS18B20_GetTempTNumber(DS18B20_GetNumbyRom(romaddress)));
}

//private funct
//20200701 change raw from uint16 to int16
float DS_get_temp(struct T* ts){
if (!(mgos_onewire_reset(ts->onewire))) return -997; 
  mgos_onewire_select(ts->onewire,ts->device_address);
  mgos_onewire_write(ts->onewire,CMD_CONVERT_T);
  uint32_t cnvtime=0;
  while (! (mgos_onewire_read_bit(ts->onewire)))
  {
    mgos_msleep(10); //waiting for conversion
    if(debug) cnvtime+=10;
  } 
  if(debug) LOG(LL_INFO,("conversion time %u ms", cnvtime));
mgos_onewire_reset(ts->onewire);
mgos_msleep(100); 
if(debug) LOG(LL_INFO,("DS_get_temp rom addr> %x:%x:%x:%x:%x:%x:%x:%x",ts->device_address[0],ts->device_address[1],ts->device_address[2],ts->device_address[3],ts->device_address[4],ts->device_address[5],ts->device_address[6],ts->device_address[7]));
mgos_onewire_select(ts->onewire,ts->device_address);
uint8_t data[DATA_SCRATCHPAD_SIZE];
mgos_usleep(100);
mgos_onewire_write(ts->onewire,CMD_READ_SCRATCHPAD);
mgos_msleep(100);
for(uint8_t s=0; s<DATA_SCRATCHPAD_SIZE; s++){  //read complete scratchpad
    data[s]=mgos_onewire_read(ts->onewire);
    if(debug) LOG(LL_INFO,("reading scratchpad data %i data=%x",s,data[s]));
}
int16_t raw= data[DATA_TEMP_MSB];
raw=(raw <<8);
raw=(raw | data[DATA_TEMP_LSB]);
if(debug) LOG(LL_INFO,("DS_get_temp lsb=%x, msb=%x",data[DATA_TEMP_LSB],data[DATA_TEMP_MSB] ));
if(debug) LOG(LL_INFO,("DS_get_temp crc=%x",data[8]));
uint8_t cfg= (data[DATA_REG_CONF] & REG_CONF_RESOLUTION_MASK);//only bit 5 and 6 used for conf res
if(debug) LOG(LL_INFO,("data[DATA_REG_CONF]=%x", data[DATA_REG_CONF]));
if(debug) LOG(LL_INFO,("DS_get_temp resolution cfg=%x", cfg));
if (cfg == REG_CONF_RESOLUTION_9BIT) {
    raw = raw & ~7;
  } else if (cfg == REG_CONF_RESOLUTION_10BIT) {
    raw = raw & ~3;
  } else if (cfg == REG_CONF_RESOLUTION_11BIT) {
    raw = raw & ~1;
  }
//default resolution 12bit 0x60
if(debug) LOG(LL_INFO,("DS_get_temp raw=%x", raw));
return (float)(raw/16.0); //minimal resolution 1/16 C
}

//public
uint8_t DS18B20_init(struct mgos_onewire *ow){
  if (!(mgos_onewire_reset(ow))) return 0;
  mgos_onewire_search_clean(ow);
  mgos_onewire_target_setup(ow,DEVICE_FAMILY_DS18B20);
  if(debug) LOG(LL_INFO,("DS18B20_init entering.."));
  uint8_t* rom_st=(uint8_t*)malloc((sizeof(uint8_t))*8); //primary alocation of rom of sensor //tohle malloc asi taky zbytecne --mallocknote
    for(uint8_t j=0; j<8;j++){
        rom_st[j]=0x00;
      }
  while(mgos_onewire_next(ow,rom_st,0)){
    addToDevField(rom_st, ow);
    if(debug) LOG(LL_INFO,("init read rom addr> %x:%x:%x:%x:%x:%x:%x:%x",rom_st[0],rom_st[1],rom_st[2],rom_st[3],rom_st[4],rom_st[5],rom_st[6],rom_st[7]));
  }
    if(debug) LOG(LL_INFO,("DS18B20_init finished ok!"));
  return countDevs;
}


//private function uint8_t to hex 
char *byteToHexF(uint8_t byteOfAddress){
  char *p_ret=(char*)malloc(sizeof(char)*3);
  char ret[2];     
  char hex_char[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
  ret[0]=hex_char[(byteOfAddress >> 4) & 0x0F]; 
  ret[1]=hex_char[byteOfAddress & 0x0F];
  strcpy(p_ret, ret);
  return p_ret;
}

//private f
//return device-specific 6byte-part of rom address of device as char*
char* DS18B20GetCharDeviceAddress(struct T* devT){
  char* addrstring=(char*)malloc(64*sizeof(char));
    strcpy(addrstring, devT->char_address);
  return addrstring;
}

//private
//delay, use uint64_t
void DelayMicroseconds(uint64_t micros){
  uint64_t actTime;
  actTime=mgos_uptime_micros();
  while ((actTime+micros)>mgos_uptime_micros()) {};
}

//private
//delay miliseconds, use uint32_t
void DelayMiliseconds(uint32_t milis){
  DelayMicroseconds(milis*1000);
}



//public
char** DS18B20ListAddresses(){
  char** addrarr=(char**) malloc ((countDevs+2)*(sizeof(char*)));
  for(uint8_t i=0; i<countDevs;i++){
       addrarr[i]= DS18B20GetCharDeviceAddress(tow[i]);
       addrarr[i+1]=NULL;
  }
  return addrarr;
}

