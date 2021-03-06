/* V59 = relayOveride feedback to app
 * 
 */

bool resetFlag = false;

// This function runs every time Blynk connection is established.
BLYNK_CONNECTED() 
{
  if ( isFirstConnect ) 
  {
    Blynk.syncAll();
    isFirstConnect = false;
  }
  
  #ifdef forwardTemp
    blynkBridge1.setAuthToken(forwardAuth); // Device to forward to
  #endif
}


void sendBlynk() { //Feedback to the Blynk app
  Blynk.virtualWrite(V49,temp.cur);
  Blynk.virtualWrite(V54,temp.target);
  Blynk.virtualWrite(V56,temp.offset);
  Blynk.virtualWrite(V58,cooldownPeriod);

  #ifdef inverseMode //Send high if using inverse mode
    Blynk.virtualWrite(V61,255);
  #else
    Blynk.virtualWrite(V61,0);
  #endif


  #ifdef forwardTemp
    blynkBridge1.virtualWrite(V51,temp.cur);
  #endif
}

BLYNK_WRITE(V51) {
  temp.cur = param.asFloat(); //Get temp from other device
}

BLYNK_WRITE(V59) {
  relayOveride = param.asInt();
}

BLYNK_WRITE(V60) {
  relayControl = param.asInt();
}

BLYNK_WRITE(V53) {
  temp.target = param.asFloat();
}

BLYNK_WRITE(V55) {
  temp.offset = param.asFloat();
}

BLYNK_WRITE(V57) {
  cooldownPeriod = param.asInt();
}

BLYNK_WRITE(V21) {
  if (param.asInt() == 1) {
    configSave();
    Blynk.virtualWrite(21,0);
  }
}

BLYNK_WRITE(V70) {
  resetFlag = param.asInt();

  if (resetFlag == true) {
    ESP.reset();
  }
}
