#include "structs.h"

//Device name should be unique as it is used
//to identify the device for OTA updates.
#define deviceName "Heater"


/*Uncomment the operating mode you wish to use. 
 *  Normal  - Used for heating. If temp.cur < desired, turn on
 *  Inverse - Operates in the inverse. If temp.cur > desired turn on | This mode could be used for an exhaust fan to limit
 *  temperature or humidity. (Just feed in a humidity value instead of temperature.
*/
#define normalMode
//#define inverseMode 

//Temerature settings
Temperature temp = {
  95,  //Current Temp (Don't change)
  60 ,  //Target Temp
  6 ,  //Temp Range
  0 ,  //Mode (0:F or 1:C) (Not coded)
};

/*This amount of time must pass before the heater is allowd to turn back on (in seconds)
* Though this shouldn't be needed with the temp.offset, I wished to add it anyway.
* This is an anti-short cycle feature.
 */
int cooldownPeriod = 30;

// Max current allowed before disconnecting relay. Sonoff says the s31 is rated for 15 amps.
#define maxSafeLoad 15

// Cooldown before attempting to reconnect load
#define loadCooldown 30

// Attempts before giving up and leaving relay off until reset
#define loadConnectAttempts 3

//If you wish to forward the temperature to another device uncomment this line.
//And, add the token in "blynk.h"
#define forwardTemp
