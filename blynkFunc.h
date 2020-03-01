/* V59 = relayOveride feedback to app
 * 
 */

BLYNK_WRITE(V50) {
  temp.cur = param.asInt(); //Get temp from other device
}

BLYNK_WRITE(V59) {
  if (param.asInt() != relayOveride) {
    relayOveride = param.asInt();
  }
}

void sendBlynk() { //Feedback to the Blynk app
  Blynk.virtualWrite(V51,temp.cur);

  if (isFirstRun) {
    Blynk.virtualWrite(V53,temp.target);
    Blynk.virtualWrite(V55,temp.offset);
    Blynk.virtualWrite(V57,cooldownPeriod);
  }
}
