/* Sonoff Smart Plug powered heater controller.
 *  
 *  This programed is to be flashed onto a Sonoff S31 smart plug
 *  to allow controlling a "dumb" heater that uses a mechanical thermostat.
 *  
 *  This allows remote control and monitoring of a heater that would
 *  normally not have these features. Being a discrete addition, this
 *  feature set can be moved from heater to heater.
 *  
 *  I am building upon code from the below link.
 *  The Wemo functions have been removed.
 *  
 *  https://ucexperiment.wordpress.com/2019/01/20/using-a-sonoff-s31-with-blynk/
 *  Original Author: James Eli
 *  
 *  Modifications and additions by: Cory McGahee
 *  
 *  Non-Commercial use ONLY!
 *  
 *  !!WARNING!!
 *  Though a basic overload protection system is in this code, it is not guaranteed to
 *  actually provide any protection. YOU take all risk and resposibility for any damage,
 *  inury or death caused by the function or malfunction of this code. For maximum safety,
 *  be certain that the attached load to this switch, be certain that your load is in proper
 *  working order and will not overload your outlet and smart switch.
 */ 

int configVersion = 0;

#include <ESP8266WiFi.h>
#include <Ethernet.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h> 
#include <ESP8266WebServer.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>

bool isFirstRun = true;
bool isFirstConnect = true;       // Flag for re-sync on connection.
int relayState = LOW;             // Blynk app pushbutton status.

boolean SwitchReset = true;

// esp8266 pins.
#define ESP8266_GPIO13  13        // Sonof green LED (LOW == ON).
#define ESP8266_GPIO0   0         // Sonoff pushbutton (LOW == pressed).
#define ESP8266_GPIO12  12        // Sonoff relay (HIGH == ON).
const int RELAY = ESP8266_GPIO12; // Relay switching pin. Relay is pin 12 on the SonOff
const int LED = ESP8266_GPIO13;   // On/off indicator LED. Onboard LED is 13 on Sonoff
const int SWITCH = ESP8266_GPIO0; // Pushbutton.

// CSE7766 data.
double power = 0;
double voltage = 0;
double current = 0;
double energy = 0;
double ratioV = 1.0;
double ratioC = 1.0;
double ratioP = 1.0;
// Serial data input buffer.
unsigned char serialBuffer[24];
// Serial error flags.
int error;
// Energy reset counter.
int energyResetCounter;
#define MAX_ENREGY_RESET_COUNT 12

// CSE7766 error codes.
#define SENSOR_ERROR_OK             0       // No error.
#define SENSOR_ERROR_OUT_OF_RANGE   1       // Result out of sensor range.
#define SENSOR_ERROR_WARM_UP        2       // Sensor is warming-up.
#define SENSOR_ERROR_TIMEOUT        3       // Response from sensor timed out.
#define SENSOR_ERROR_UNKNOWN_ID     4       // Sensor did not report a known ID.
#define SENSOR_ERROR_CRC            5       // Sensor data corrupted.
#define SENSOR_ERROR_I2C            6       // Wrong or locked I2C address.
#define SENSOR_ERROR_GPIO_USED      7       // The GPIO is already in use.
#define SENSOR_ERROR_CALIBRATION    8       // Calibration error or not calibrated.
#define SENSOR_ERROR_OTHER          99      // Any other error.
#define CSE7766_V1R                 1.0     // 1mR current resistor.
#define CSE7766_V2R                 1.0     // 1M voltage resistor.


//-------------------------------
bool relayOveride = false;
bool relayControl = false;
bool safeLoad = true;
bool previousSafeLoad = true;

int loadAttempt = 0;
bool resetRequired = false;


#include "config.h"
#include "timer.h"
#include "blynk.h"
#include "function.h"
#include "blynkFunc.h"




 void setup() {
  
  // WiFiManager intialization.
  WiFiManager wifi; 

  // Initialize pins.
  pinMode( RELAY, OUTPUT );
  pinMode( LED, OUTPUT );
  pinMode( SWITCH, INPUT_PULLUP );
  delay( 10 );
  // Switch relay off, LED on.
  digitalWrite( RELAY, LOW );
  digitalWrite( LED, LOW ); 

  // Create AP, if necessary
  wifi.autoConnect(deviceName);

  // Set WIFI module to STA mode
  WiFi.mode( WIFI_STA );

  // Initialize Blynk.
  #ifdef useLocalServer
    Blynk.config(auth,server,port);
  #else
    Blynk.config(auth);
  #endif

  // Set ota details.
  ArduinoOTA.setHostname(deviceName);
  ArduinoOTA.begin();

  //Call timer setup from timer.h
  timerSetup();

  Serial.flush();
  Serial.begin( 4800 );

  // Switch LED off to signal initialization complete.
  digitalWrite( LED, HIGH );

  EEPROM.begin(256); //Initialize the EEPROM

  if (EEPROM.read(0) == 1 && returnConfigVersion() == configVersion) { //If flagged in EEPROM; we load our current position
    configLoad();
    //Serial.println("Pass Config Version Check");
  } else {
    //Serial.println("Fail Config Version Check, Reseting To Default");
    configSave();
    
  }
  
 }

void loop() {

  
  if (timer(1000,2) == true) {
    sendBlynk();
    //Serial.println(temp.cur);
  }

  if (isFirstRun == true) {
   Blynk.virtualWrite(V53,temp.target);
   Blynk.virtualWrite(V55,temp.offset);
   Blynk.virtualWrite(V57,cooldownPeriod);
  }

  isFirstRun = false;
  
  ArduinoOTA.handle();
  Blynk.run();
  process();

  if (!relayOveride) {
    tempProcess();
  } else {
    if (relayControl) {
      RelayOn();
    } else {
      RelayOff();
    }
  }
  previousSafeLoad = safeLoad; // Flag
}
