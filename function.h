

//---------------------------------------------------------
//Orignal code functions
// Toggle the relay on
void RelayOn() 
{
  if (!resetRequired) {
  digitalWrite( RELAY, HIGH );
  relayState = true;
  Blynk.virtualWrite( V0, HIGH ); // Sync the Blynk button widget state
  Blynk.virtualWrite( V1, relayState*255 );
  }
}

// Toggle the relay off
void RelayOff() 
{
  digitalWrite( RELAY, LOW );
  relayState = false;
  Blynk.virtualWrite( V0, LOW ); // Sync the Blynk button widget state
  Blynk.virtualWrite( V1, relayState*255 );
}

// Handle switch changes originating on the Blynk app
BLYNK_WRITE( V0 ) 
{
  int SwitchStatus = param.asInt();
  
  if ( SwitchStatus )
    RelayOn();
  else 
    RelayOff();
}


// This function runs every time Blynk connection is established.
BLYNK_CONNECTED() 
{
  if ( isFirstConnect ) 
  {
    Blynk.syncAll();
    isFirstConnect = false;
  }
}

// Handle hardware switch activation.
void ButtonCheck() 
{
  // look for new button press
  boolean SwitchState = ( digitalRead( SWITCH ) );
  
  // toggle the switch if there's a new button press
  if ( !SwitchState && SwitchReset == true ) 
  {
    if ( relayState )
      RelayOff();
    else
      RelayOn();
  
    // Flag that indicates the physical button hasn't been released
    SwitchReset = false;
    delay( 50 );            // De-bounce interlude.

    relayOveride = true; //If button is used, overide the temperature control
    Blynk.virtualWrite(V59,relayOveride); //Feedback to the app
  } 
  else if ( SwitchState ) 
  {
    // reset flag the physical button release
    SwitchReset = true;
  }
}

// Relay toggle helper function.
void ToggleRelay() 
{
  relayState = !relayState;
  
  if ( relayState ) 
    RelayOn();
  else 
    RelayOff();
}

// CSE7766 checksum.
bool CheckSum() 
{
  unsigned char checksum = 0;
  
  for (unsigned char i = 2; i < 23; i++) 
    checksum += serialBuffer[i];

  return checksum == serialBuffer[23];
}

// Process a cse7766 data packet.
void ProcessCse7766Packet() 
{
  // Confirm packet checksum.
  if ( !CheckSum() ) 
  {
    error = SENSOR_ERROR_CRC;
    return;
  }

  // Check for calibration error.
  if ( serialBuffer[0] == 0xAA ) 
  {
    error = SENSOR_ERROR_CALIBRATION;
    return;
  }
  if ( (serialBuffer[0] & 0xFC) == 0xFC ) 
  {
    error = SENSOR_ERROR_OTHER;
    return;
  }

  // Retrieve calibration coefficients.
  unsigned long coefV = (serialBuffer[2] << 16 | serialBuffer[3] << 8 | serialBuffer[4] );
  unsigned long coefC = (serialBuffer[8] << 16 | serialBuffer[9] << 8 | serialBuffer[10]);
  unsigned long coefP = (serialBuffer[14] << 16 | serialBuffer[15] << 8 | serialBuffer[16]);
  uint8_t adj = serialBuffer[20];

  // Calculate voltage.
  voltage = 0;
  if ( (adj & 0x40) == 0x40 ) 
  {
    unsigned long voltageCycle = serialBuffer[5] << 16 | serialBuffer[6] << 8 | serialBuffer[7];
    voltage = ratioV*coefV/voltageCycle/CSE7766_V2R;
  }

  // Calculate power.
  power = 0;
  if ( (adj & 0x10) == 0x10 ) 
  {
    if ( (serialBuffer[0] & 0xF2) != 0xF2 ) 
    {
      unsigned long powerCycle = serialBuffer[17] << 16 | serialBuffer[18] << 8 | serialBuffer[19];
      power = ratioP*coefP/powerCycle/CSE7766_V1R/CSE7766_V2R;
    }
  }

  // Calculate current.
  current = 0;
  if ( (adj & 0x20) == 0x20 ) 
  {
    if ( power > 0 ) 
    {
      unsigned long currentCycle = serialBuffer[11] << 16 | serialBuffer[12] << 8 | serialBuffer[13];
      current = ratioC*coefC/currentCycle/CSE7766_V1R;
    }
  }

  // Calculate energy.
  unsigned int difference;
  static unsigned int cfPulsesLast = 0;
  unsigned int cfPulses = serialBuffer[21] << 8 | serialBuffer[22];
  
  if (0 == cfPulsesLast) 
    cfPulsesLast = cfPulses;
  
  if (cfPulses < cfPulsesLast) 
    difference = cfPulses + (0xFFFF - cfPulsesLast) + 1;
  else
    difference = cfPulses - cfPulsesLast;
  
  energy += difference*(float)coefP/1000000.0;
  cfPulsesLast = cfPulses;

  // Energy reset.
  if ( power == 0 )
    energyResetCounter++;
  else
    energyResetCounter = 0;
  if ( energyResetCounter >= MAX_ENREGY_RESET_COUNT )
  {
    energy = 0.0;
    energyResetCounter = 0;
  }
   
  // Push data to Blynk app.
  Blynk.virtualWrite( V3, voltage ); // Voltage (Volts).
  Blynk.virtualWrite( V4, current ); // Current (Amps).
  Blynk.virtualWrite( V5, power );   // Power (Watts).
  Blynk.virtualWrite( V6, energy );  // Energy (kWh).
}

// Read serial cse7766 power monitor data packet.
void ReadCse7766() 
{
  // Assume a non-specific error.
  error = SENSOR_ERROR_OTHER; 
  static unsigned char index = 0;

  while ( Serial.available() > 0 )
  {
    uint8_t input = Serial.read();

    // first byte must be 0x55 or 0xF?.
    if ( index == 0 ) 
    { 
      if ( (input != 0x55) && (input < 0xF0) ) 
        continue;
    }
    // second byte must be 0x5A.
    else if ( index == 1 ) 
    {
      if ( input != 0x5A ) 
      {
        index = 0;
        continue;
      }
    }
    
    serialBuffer[index++] = input;
    
    if ( index > 23 ) 
    {
      Serial.flush();
      break;
    }
  }

  // Process packet.
  if ( index == 24 ) 
  {
    error = SENSOR_ERROR_OK;
    ProcessCse7766Packet();
    index = 0;
  }

  // Report error state (LED) of cse7766.
  if ( error == SENSOR_ERROR_OK )
    Blynk.virtualWrite( V18, 0 );
  else
    Blynk.virtualWrite( V18, 255 );
}

//---------------------------------

//Basic functions that must run
void process() {
  if (timer(2000,0)) {
    ReadCse7766();
  }

  if (timer(100,1)) {
    ButtonCheck();
  }
}

bool cooldownCheck() {
  if (timerCheck(3) >= cooldownPeriod*1000) {
    return true;
  } else {
    return false;
  }
}

#ifdef normalMode //Create normalMode process function
  void tempProcess() {
    if (temp.cur < temp.target && relayState == false) { //If colder than set do next check
      if (cooldownCheck() == true && temp.cur < temp.target - temp.offset) { //If cooldown has passed and temp is below turn on temp, turn on
        RelayOn();
      }
    } else {
      if (temp.cur >= temp.target && relayState == true) {
        RelayOff();
        timerReset(3); //Reset cooldown timer when turning off relay
      }
    }
  }
#endif

#ifdef inverseMode
  void tempProcess() {
    if (temp.cur > temp.target && relayState == false) { //If hotter than set do next check
      if (cooldownCheck() == true && temp.cur > temp.target + temp.offset) { //If cooldown has passed and temp is above turn on temp, turn on
        RelayOn();
      }
    } else {
      if (temp.cur <= temp.target && relayState == true) {
        RelayOff();
        timerReset(3); //Reset cooldown timer when turning off relay
      }
    }
  }
#endif

void configSave() {
  EEPROM.write(0,1); //Flag for autoload
  EEPROM.put(3,configVersion);
  int i=10;
  EEPROM.put(i,temp.target);
  i+=sizeof(temp.target);
  EEPROM.put(i,temp.offset);
  i+=sizeof(temp.offset);
  EEPROM.put(i,temp.unit);
  i+=sizeof(temp.unit);
  EEPROM.put(i,cooldownPeriod);
  i+=sizeof(cooldownPeriod);
  EEPROM.commit();
}



void configLoad() {
  int i=10;
  EEPROM.get(i,temp.target);
  i+=sizeof(temp.target);
  EEPROM.get(i,temp.offset);
  i+=sizeof(temp.offset);
  EEPROM.get(i,temp.unit);
  i+=sizeof(temp.unit);
  EEPROM.get(i,cooldownPeriod);
  i+=sizeof(cooldownPeriod);
}

byte returnConfigVersion() {
  byte grabbedVersion;
  EEPROM.get(3,grabbedVersion);
  return grabbedVersion;
}

bool overloadCheck() {
    if (current > 15) {
        safeLoad = false;
        return true;
        timerReset(4);
    } else {
        return false;
    }
}

void safetyNet() {
  overloadCheck();
  
  if (!safeLoad && timerCheck(4) >= loadCooldown*1000 && loadAttempt < loadConnectAttempts)  {
        safeLoad = true;
        loadAttempt++;
        timerReset(4);
        timerReset(5);
    }

  if (loadAttempt > 0) {
    if (timerCheck(5) > 30*1000) { //If relay hasn't overloaded for 30 seconds, reset the loadAttempt count
      loadAttempt = 0;
    }

    if (loadAttempt > loadConnectAttempts) {
      resetRequired = true;
    }
  }
}
