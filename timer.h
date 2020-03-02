/*Timer code
 * 0 - Reads from sensor & gets temp from bridged device
 * 1 - Reads button
 * 2 - Update Blynk app data
 * 3 - Heater trigger cooldown
 */


#define countDataAmount 4 //Number of timers
unsigned long countData[countDataAmount] = {0}; //Holds count information. (Adjust for numeber of timers needed.)


//Used by timer function
unsigned long millisCount(int _mode, int _id) { //_mode: 0-Start 1-Stop | _id Identity number (allow more by editing the length of countData
  unsigned long _count;
  if (_mode == 0) {
    countData[_id]= millis();
    return 0;
  }

  if (_mode == 1) {
    _count = millis() - countData[_id];
    return _count;
  }

}

//Will set all variables in timer table to 0.
void timerSetup() {
  for (int i=0;i<countDataAmount;i++) {
    millisCount(0,i);
  }
}

//Use this to check if set time has passed.
//Will return true or false
bool timer(unsigned long _interval,int _id) { //_interval in millis, _id in countData
  if (millisCount(1,_id) >= _interval) {
    millisCount(0,_id);
    return true;
    Serial.print("Timer: "); Serial.print(_id); Serial.print(" ");
    Serial.println("True");
  } else {
    return false;
  }
   
}

unsigned long timerCheck(int _id) { //Simply return the elapsed count
  return millisCount(1,_id);
}

void timerReset(byte _id) { //Reset timer start time
  countData[_id]= millis();
}
