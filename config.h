#include "structs.h"

//Device name should be unique as it is used
//to identify the device for OTA updates.
#define deviceName "Heater"

//Blynk token for the device that will be providing
//the temperature.
#define tempDeviceToken ""

//Temerature settings
Temperature temp = {
  90,  //Current Temp (Don't change)
  0 ,  //Target Temp
  2 ,  //Temp Range
  0 ,  //Mode (0:F or 1:C) (Not coded)
};

//This amount of time must pass before the heater is allowd to turn back on (in seconds)
//Though this shouldn't be needed with the temp.offset, I wished to add it anyway.
int cooldownPeriod = 30;
