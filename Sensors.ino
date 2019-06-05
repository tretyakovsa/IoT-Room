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
    flag = sendStatus(irReceivedS, uint64ToString(results.value, HEX));
    irReceiver->resume();  // Continue looking for IR codes after your finished dealing with the data.
  }
}
void dump(decode_results *results) {
  uint16_t count = results->rawlen;
  //Serial.println(count);
  sendOptions(irDecodeTypeS, results->decode_type);
}
#endif
#ifdef rfM // #endif
// ----------------------Приемник на 433мГ
void rfReceived() {
  byte pin = readArgsInt();
  pin =  pinTest(pin);
//  Serial.println("rfReceived?");
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
#ifdef A0M // #endif
// -----------------  Аналоговый вход A0
void initA0() {
  static uint16_t t = readArgsInt(); //время опроса датчика
  static uint8_t averageFactor = readArgsInt();
  if (t < 500) t = 1000;
  if (averageFactor == 0) averageFactor = 1023;
  analogRead(A0);
  sendStatus(stateA0S, analogRead(A0));
  sendOptions(alarmA0S, 0);
  String alarmSet = "ALARM " + stateA0S + " " + highalarmA0S + " " + lowalarmA0S;
  sCmd.readStr(alarmSet);
  alarmLoad(stateA0S, highalarmA0S, lowalarmA0S);
  ts.add(tA0, t, [&](void*) {
    uint32_t a = 0;
    for (uint8_t i = 1; i <= 10; i++) {
      a += analogRead(A0);
    }
    a = a / 10;
    sendStatus(stateA0S, a);
    alarmTest(stateA0S, highalarmA0S, lowalarmA0S, alarmA0S);
  }, nullptr, true);
  modulesReg("A0");
}
#endif
#ifdef DS18B20M // #endif
// ----------------- OneWire -------------------------------------------------------------------------------
void initOneWire() {
  uint8_t pin = readArgsInt();
  pin =  pinTest(pin, HIGH);
  static uint16_t t = readArgsInt();
  static uint8_t averageFactor = readArgsInt();
  if (t < 750) t = 1000;
  oneWire =  new OneWire(pin);
  sensors.setOneWire(oneWire);
  sensors.begin();
  byte num = sensors.getDS18Count();
  if (num > 0) {
    sensors.setResolution(9);
    for (byte i = 0; i < num; i++) {
      String numS = (String)(i + 1);
      sendStatus(temperatureS + numS, (String)sensors.getTempCByIndex(i));
      sendOptions(alarmtempS + numS, 0);
      sendOptions(highalarmtempS + numS, 0);
      sendOptions(lowalarmtempS + numS, 0);
      alarmLoad(temperatureS + numS, highalarmtempS + numS, lowalarmtempS + numS);
      modulesReg(temperatureS + numS);
    }
    sendOptions(temperatureS + "num", num);
    ts.add(tDS, t, [&](void*) {
      float temp = 0;
      sensors.requestTemperatures();
      for (byte i = 0; i < sensors.getDS18Count(); i++) {
        temp = sensors.getTempCByIndex(i);
        String num;
        num = (String)(i + 1);
        sendStatus(temperatureS + num, (String)temp);
        alarmTest(temperatureS + num, highalarmtempS + num, lowalarmtempS + num, alarmtempS + num);
      }
    }, nullptr, true);
  }
}
#endif
#ifdef DHTM //
// -----------------  DHT
void initDHT() {
  uint8_t pin = readArgsInt();
  pin =  pinTest(pin, HIGH);
  dht.setup(pin);
  delay(1000);
  static uint16_t t = readArgsInt();
  static uint16_t test = dht.getMinimumSamplingPeriod();
  if (t < test) t = test;
  //Serial.println(t);
  String temp;
  temp += dht.getTemperature();
  //Serial.println(temp);
  if (temp != "nan") {
    sendStatus(temperatureS, dht.getTemperature());
    sendOptions(alarmtempS, 0);
    alarmLoad(temperatureS, highalarmtempS, lowalarmtempS);
    sendStatus(humidityS, dht.getHumidity());
    sendOptions(alarmhumS, 0);
    alarmLoad(humidityS, highalarmhumS, lowalarmhumS);
    ts.add(tDHT, test, [&](void*) {
      sendStatus(temperatureS, dht.getTemperature());
      //Serial.println(temp);
      sendStatus(humidityS, dht.getHumidity());
      alarmTest(temperatureS, highalarmtempS, lowalarmtempS, alarmtempS);
      alarmTest(humidityS, highalarmhumS, lowalarmhumS, alarmhumS);
    }, nullptr, true);
    modulesReg(temperatureS);
    modulesReg(humidityS);
  }
}
#endif
// ------------------------------- Проверка уровней ------------------------------------------------
// Текущее значение сенсора уровни признак датчика
void alarmTest(String value, String high, String low, String sAlarm ) {
  if (getOptionsFloat(high) != 0 || getOptionsFloat(low) != 0) { // нужно добавить флаг остановки теста
    //Serial.println("test");
    if (getStatusFloat(value) > getOptionsFloat(high) && getOptionsInt(sAlarm) == LOW) {
      sendOptions(sAlarm, HIGH);
      //Serial.println(getStatusFloat(value));
      //      Serial.print("up<");
      //      Serial.println(getStatusFloat(value));
      flag = sendStatus(value, getStatusFloat(value));
    }
    if (getStatusFloat(value) < getOptionsFloat(low) && getOptionsInt(sAlarm) == HIGH) {
      sendOptions(sAlarm, LOW);
      //        Serial.print("down>");
      //        Serial.println(getStatusFloat(value));
      flag = sendStatus(value, getStatusFloat(value));
    }
  }
}

// ----------------------- Загрузка данных уровней сработки ------------------------------------------
// Имя параметра теста и имена границ
void alarmLoad(String sName, String high, String low) {
  sendOptionsF(high, 0);
  sendOptionsF(low, 0);
  String configSensor = readFile(ScenaryS, 4096);
  configSensor.replace("\r\n", "\n");
  configSensor.replace("\n\n", "\n");
  configSensor += "\n";
  do {
    String test = selectToMarker(configSensor, "\n");
    // if temperature1 > 26.50
    String del = "if " + sName + " >";
    if (test.indexOf(del) != -1) {
      //      Serial.println(test);
      test.replace(del, emptyS);
      sendOptionsF(high, test.toFloat());
    }
    del = "if " + sName + " <";
    if (test.indexOf(del) != -1) {
      //      Serial.println(test);
      test.replace(del, emptyS);
      sendOptionsF(low, test.toFloat());
    }
    configSensor = deleteBeforeDelimiter(configSensor, "\n"); //Откидываем обработанную строку
    configSensor = deleteBeforeDelimiterTo(configSensor, "if"); // откидываем все до следующего if
  } while (configSensor.length() != 0);
}
