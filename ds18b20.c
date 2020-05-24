/*
*/



#include "ds18b20.h"

struct T{
  uint8_t number; //1-255
  uint8_t *device_address;//=rom =is 64bit sensor address
  uint8_t devaddr[6];
  uint8_t crc;
  char char_address[64]; //address of device in hexa chars
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


const bool debug=true;
//private funct
float DS_get_temp(struct T* ts);
void createDevField();
uint8_t addToDevField(uint8_t devrom[], struct mgos_onewire *ow);
char *byteToHexF(uint8_t byteOfAddress);


//private funct
void createDevField(){ //primary creation of array of sensors, run only once, next run will redefine array
  if(debug) LOG(LL_INFO,("createDevField entering.."));
  // tow=(struct T**)malloc(sizeof(struct T*));
 /*
  if(countDevs!=0) {  //if array already exist (redefinition)
  if(debug) LOG(LL_INFO,("createDevField freeing allocation.."));
    for(uint8_t i=0;i<countDevs;i++){
      free(tow[i]);
      tow[i]=NULL;
    }
  }
  */   
  if(debug) LOG(LL_INFO,("createDevField after if..\n"));
  countDevs=0;
  if(debug) LOG(LL_INFO,("createDevField finished ok., tow=%i, *tow=%i, tow[0]=%i, *tow[0]=%i memory.", sizeof(tow), sizeof(*tow), sizeof(tow[0]), sizeof(*tow[0])));
}

//private funct, "constructor" of struct T
uint8_t addToDevField(uint8_t devrom[], struct mgos_onewire *ow){ //add memory for one sensor to array of sensors, returns last access field item index
  if(debug) LOG(LL_INFO,("addToDevField started.."));
  //*tow=(struct T*)realloc(tow,(sizeof(*tow)+sizeof(struct T*)));
    if(debug) LOG(LL_INFO,("sizeof(*tow) is now=%i \n",sizeof(*tow)));
  tow[countDevs]=(struct T*)malloc(sizeof(struct T));
  if(debug) LOG(LL_INFO,("pointer 001 \n"));
  tow[countDevs]->device_address=(uint8_t*)malloc(64);
  if(debug) LOG(LL_INFO,("addToDevField, memory allocated.. tow=%i, *tow=%i, tow[%i]=%i, *tow[%i]=%i sizeof(struct T)= %i memory.", sizeof(tow), sizeof(*tow), countDevs, sizeof(tow[countDevs]), countDevs, sizeof(*tow[countDevs]),sizeof(struct T)));
  tow[countDevs]->device_address=devrom;
  tow[countDevs]->onewire=ow;
  if(tow[countDevs]->device_address[0]==DEVICE_FAMILY_DS18B20) tow[countDevs]->isDs18b20=true;
    else tow[countDevs]->isDs18b20=false;
    char buf[32];
  if(debug) LOG(LL_INFO,("addToDevField buf.."));
  sprintf(buf,"%x:%x:%x:%x:%x:%x:%x:%x", tow[countDevs]->device_address[0], tow[countDevs]->device_address[1], tow[countDevs]->device_address[2], tow[countDevs]->device_address[3], tow[countDevs]->device_address[4], tow[countDevs]->device_address[5], tow[countDevs]->device_address[6], tow[countDevs]->device_address[7]);//bit1,2,3,4,5,6
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


//private funct
float DS_get_temp(struct T* ts){
if (!(mgos_onewire_reset(ts->onewire))) return -999; 
  mgos_onewire_write(ts->onewire,CMD_CONVERT_T);
  mgos_msleep(100);
mgos_onewire_reset(ts->onewire);
mgos_msleep(1000);
if(debug) LOG(LL_INFO,("DS_get_temp rom addr> %x:%x:%x:%x:%x:%x:%x:%x",ts->device_address[0],ts->device_address[1],ts->device_address[2],ts->device_address[3],ts->device_address[4],ts->device_address[5],ts->device_address[6],ts->device_address[7]));
mgos_onewire_select(ts->onewire,ts->device_address);
uint8_t data[DATA_SCRATCHPAD_SIZE];
mgos_msleep(100);
mgos_onewire_write(ts->onewire,CMD_READ_SCRATCHPAD);
mgos_msleep(100);
for(uint8_t s=0; s<DATA_SCRATCHPAD_SIZE; s++){  //read all scratchpad
    data[s]=mgos_onewire_read(ts->onewire);
    if(debug) LOG(LL_INFO,("reading scratchpad data %i data=%x",s,data[s]));
}
//uint16_t raw= (data[DATA_TEMP_MSB] <<8 | data[DATA_TEMP_LSB]);
uint16_t raw= data[DATA_TEMP_MSB];
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
  createDevField();
  uint8_t rom[8];
  uint8_t* rom_st=(uint8_t*)malloc((sizeof(uint8_t))*8); //primary alocation of rom of sensor
    for(uint8_t j=0; j<8;j++){
        rom_st[j]=0x00;
      }
  while(mgos_onewire_next(ow,rom,0)){
    for(uint8_t j=0; j<8;j++){
        rom_st[j]=rom[j];
      }
    addToDevField(rom_st, ow);
  }
  if(debug) LOG(LL_INFO,("init read rom addr> %x:%x:%x:%x:%x:%x:%x:%x",rom[0],rom[1],rom[2],rom[3],rom[4],rom[5],rom[6],rom[7]));
  if(debug) LOG(LL_INFO,("DS18B20_init finished ok!"));
  return countDevs;
}

/*
//function counting number of sensors on one onewire link *ow
uint8_t DS18B20CountSensors(struct mgos_onewire *ow){
const uint8_t CMD_READ_SCRATCHPAD= 0xBE;
// const uint16_t CMD_READ_ROM= 0x33;  //cmd read_rom (rom size 64b), only if one device on a ow bus
const uint8_t CMD_SEARCH_ROM= 0xf0; //search all onewire bus
const uint8_t DATA_SCRATCHPAD_SIZE= 9; //9 data bytes scratchpad size
mgos_onewire_reset(ow);
mgos_onewire_target_setup(ow, DEVICE_FAMILY_DS18B20);
uint8_t i=0;
//device_addresses[0]=malloc(64);
while (mgos_onewire_next(ow,(p_device_address[][i]),0)){
i++;
//device_addresses[i]=malloc(64);
}
return i;
}
*/

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

/*
char* DS18B20GetDeviceAddress(uint8_t i){
  char* addrstring=(char*)malloc(64);
  char *added=(char*)malloc(sizeof(char)*3);
  for(uint8_t j=0;j<8;j++){
    added=byteToHexF(device_addresses[i][j]);
    strcat(addrstring, added);
    //strcat(addrstring, '\0');
    j++;
  }
  return addrstring;
}
*/

/*
//private function read_scratchpad read rom of device and return 64bit (8bit array[9])
uint8_t* read_scratchpad(struct mgos_onewire *ow, uint8_t sizeOfScratchpad){
  uint8_t scratchpad_buffer[sizeOfScratchpad];
}
*/

/*
uint8_t* search_rom(struct mgos_onewire *ow){   //search rom f0h 
    mgos_onewire_write(ow,0xf0);
    uint8_t *code_part[8];
    for(uint8_t i=0; i<8; i++){
    *code_part[i] =  mgos_onewire_read(ow);
    }
   return code_part;
}
*/
