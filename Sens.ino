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
#ifdef A0M
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

// -----------------------Данных уровней активных модулей --------------------------------------------
void alarmLoadModules() {
  String  modulesN = selectToMarker(modules, "]");
  modulesN = deleteBeforeDelimiter(modulesN, "[");
  modulesN += ",";
  modulesN.replace("\"", emptyS);
  //Serial.println(modulesN);
  //"upgrade","relay1","ntp","ddns","mqtt","analog"
  do {
    String m = selectToMarker(modulesN, ",");
    //Serial.println(m);
    String alarmSet = "ALARM ";
    if (m == "analog")  alarmSet += stateA0S + " " + highalarmA0S + " " + lowalarmA0S;
    if (m == temperatureS)  alarmSet += temperatureS + " " + highalarmtempS + " " + lowalarmtempS;
    if (m == humidityS)  alarmSet += humidityS + " " + highalarmhumS + " " + lowalarmhumS;
#ifdef POW
    if (m == "pow")  alarmSet += ActivePowerWS + " " + highalarmpowS + " " + lowalarmpowS;
#endif
    modulesN = deleteBeforeDelimiter(modulesN, ",");
  } while (modulesN != emptyS);
}

// ----------------------- Загрузка данных уровней сработки ------------------------------------------
// Имя параметра теста и имена границ
void alarmLoad(String sName, String high, String low) {
  sendOptionsF(high, 0);
  sendOptionsF(low, 0);
  String scen = ScenaryS + getSetup(configsS) + ".txt";
  String configSensor = readFile(scen, 4096);
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
#endif
#ifdef dsM // 
// ----------------- OneWire -------------------------------------------------------------------------------
void initOneWire() {
  uint8_t pin = readArgsInt();
  pin =  pinTest(pin, HIGH);
  static uint16_t t = readArgsInt();
  static uint8_t averageFactor = readArgsInt();
  if (t < 750) t = 1000;
  //Serial.println(t);
  //Serial.println(pin);
  oneWire =  new OneWire(pin);
  sensors.setOneWire(oneWire);
  sensors.begin();
  byte num = sensors.getDS18Count();
  //Serial.println(num);
  if (num != 0) {
    for (byte i = 0; i < num; i++) {
      sensors.setResolution(9);
      String numS = (String)(i + 1);
      sendStatus(temperatureS + numS, (String)sensors.getTempCByIndex(i));
      //Serial.println();
      sendOptions(alarmtempS + numS, 0);
      sendOptions(highalarmtempS + numS, 0);
      sendOptions(lowalarmtempS + numS, 0);
      alarmLoad(temperatureS + numS, highalarmtempS + numS, lowalarmtempS + numS);
      modulesReg(temperatureS + numS);
    }
    sendOptions(temperatureS + "num", num);
    ts.add(tDS, t, [&](void*) {
      //    static float oldTemp = 0;
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
#ifdef dhtM // 
// -----------------  DHT
void initDHT() {
  uint8_t pin = readArgsInt();
  pin =  pinTest(pin, HIGH);
  dht.setup(pin);
  delay(1000);
  static uint16_t t = readArgsInt();
  static uint16_t test = dht.getMinimumSamplingPeriod();
  if (t > test) test = t;
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
      sendStatus(humidityS, dht.getHumidity());
      alarmTest(temperatureS, highalarmtempS, lowalarmtempS, alarmtempS);
      alarmTest(humidityS, highalarmhumS, lowalarmhumS, alarmhumS);
    }, nullptr, true);
    modulesReg(temperatureS);
    modulesReg(humidityS);
  }
}
#endif

#ifdef Si7021M // 
// -----------------  Si7021 --------------------------------------------------------------------
void initSi7021() {
  // Добавить проверку наличия I2C устройств
  if (sensor_Si7021.begin()) {
    sendStatus(temperatureS, sensor_Si7021.readTemperature());
    sendOptions(alarmtempS, 0);
    alarmLoad(temperatureS, highalarmtempS, lowalarmtempS);
    sendStatus(humidityS, sensor_Si7021.readHumidity());
    sendOptions(alarmhumS, 0);
    alarmLoad(humidityS, highalarmhumS, lowalarmhumS);
    ts.add(tSI, 1000, [&](void*) {
      sendStatus(temperatureS, sensor_Si7021.readTemperature());
      sendStatus(humidityS, sensor_Si7021.readHumidity());
      alarmTest(temperatureS, highalarmtempS, lowalarmtempS, alarmtempS);
      alarmTest(humidityS, highalarmhumS, lowalarmhumS, alarmhumS);
    }, nullptr, true);
    modulesReg(temperatureS);
    modulesReg(humidityS);
  }
}
#endif

#ifdef RC522 // 
void initRC522() {
  SPI.begin();
  mfrc522.PCD_Init();    // Init MFRC522
  //mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  // задача опрашивать IR код
  ts.add(tRC522, 100, [&](void*) {
    handleRC522();
  }, nullptr, true);
  sendStatus(rfidkeyS, "");
  modulesReg(rfidS);
}

void handleRC522() {
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    //sendStatus(rfidkeyS, "");
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    sendStatus(rfidkeyS, "Bad read");
    return;
  }
  String tag;
  if (mfrc522.uid.size == 0) {
  } else {
    int j = mfrc522.uid.size;
    for (int i = 0; i < j; i++) {
      //tag += mfrc522.uid.uidByte[i];
      String one = String(mfrc522.uid.uidByte[i], HEX);
      if (one.length() == 1) tag += "0";
      tag += one;
      if (i < j-1) tag += "_";

    };

    tag.toUpperCase();
    Serial.println(tag);
    flag = sendStatus(rfidkeyS, tag);
  };

  mfrc522.PICC_HaltA();
}
#endif
