#ifdef TachM
// -----------------  Tach
void initTach() {
  uint8_t pin = readArgsInt(); // первый аргумент pin
  pin =  pinTest(pin, true);
  String num = readArgsString(); // второй аргумент прификс реле 0 1 2
  uint16_t bDelay = readArgsInt(); // третий время нажатия
  sendStatus(stateTachS + num, 0);
  buttons[num.toInt()].attach(pin);
  buttons[num.toInt()].interval(bDelay);
  but[num.toInt()] = true;
  boolean inv = readArgsInt(); // четвертый аргумент инверсия входа
  sendOptions(invTachS + num, inv);
  modulesReg(tachS + num);
}
void handleButtons() {
  static uint8_t num = 0;
  String numS = String(num, DEC);
  if (but[num]) {
    buttons[num].update();
    if (buttons[num].fell()) {
      //Serial.print("Tach on ");
      //Serial.println(getStatusInt(stateTachS + numS));
      flag = sendStatus(stateTachS + numS, !getOptionsInt(invTachS + numS));
    }
    if (buttons[num].rose()) {
      //Serial.print("Tach off ");
      //Serial.println(getStatusInt(stateTachS + numS));
      flag = sendStatus(stateTachS + numS, getOptionsInt(invTachS + numS));
    }
  }
  num++;
  if (num == NUM_BUTTONS) num = 0;
}
#endif
#ifdef irM
// ----------------------Приемник ИK
void irReceived() {
  byte pin = readArgsInt();
  pin =  pinTest(pin);
  irReceiver = new IRrecv(pin);  // Create a new IRrecv object. Change to what ever pin you need etc.
  irReceiver->enableIRIn(); // Start the receiver
  // задача опрашивать IR код
  ts.add(tIR, 100, [&](void*) {
    handleIrReceiv();
  }, nullptr, true);
  sendStatus(irReceivedS, "ffffffff");
  modulesReg(irReceivedS);
}
void handleIrReceiv() {
  if (irReceiver->decode(&results)) {
    dump(&results);
    flag = sendStatus(irReceivedS, uint64ToString(results.value,HEX));
    irReceiver->resume();  // Continue looking for IR codes after your finished dealing with the data.
  }
}
void dump(decode_results *results) {
  uint16_t count = results->rawlen;
  //Serial.println(count);
    sendOptions(irDecodeTypeS, results->decode_type);
}
#endif
