#ifdef rfM // #endif
// ----------------------Передатчик на 433мГ
void rfTransmitter() {
  byte pin = readArgsInt();
  pin =  pinTest(pin, HIGH);
  mySwitch.enableTransmit(pin);
  sCmd.addCommand(rfsendS.c_str(), handleRfTransmit);
  commandsReg(rfsendS);
  actionsReg(rfsendS);
  modulesReg("rfTransmitter");
}

void handleRfTransmit() {
  int cod = readArgsInt();
  int len = readArgsInt();
  if (len == 0) len = 24;
  mySwitch.send(cod, len);
}

// ----------------------Приемник на 433мГ
void rfReceived() {
  byte pin = readArgsInt();
  pin =  pinTest(pin);
  mySwitch.enableReceive(pin);
  pinMode(pin, INPUT);
  // задача опрашивать RC код
  //ts.add(tRC, 5, [&](void*) {
    // handleRfReceiv();
  //}, nullptr, true);
  sendStatus(rfReceivedS, 0);
  sendOptions(rfBitS, 0);
  sendOptions(rfProtocolS, 0);
  modulesReg(rfReceivedS);
}

void handleRfReceiv() {
  if (mySwitch.available()) {
    int value = mySwitch.getReceivedValue();
    if (value == 0) {
      sendStatus(rfReceivedS, 0);
      sendOptions(rfBitS, 0);
      sendOptions(rfProtocolS, 0);
    } else {
      uint32_t temp = mySwitch.getReceivedValue() ;
      //Serial.println(temp);
      flag = sendStatus(rfReceivedS, temp);
      temp = mySwitch.getReceivedBitlength();
      //Serial.println(temp);
      sendOptions(rfBitS, temp);
      temp = mySwitch.getReceivedProtocol();
      //Serial.println(temp);
      sendOptions(rfProtocolS, temp);
    }
    mySwitch.resetAvailable();
  }
}
#endif
