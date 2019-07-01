// ---------------Инициализация модулей
void initCMD() {
  sCmd.addCommand("UART",       uart);
  sCmd.addCommand("ADMIN",       initAdmin);
  sCmd.addCommand("GET",       initGet);
  commandsReg("GET");
  sCmd.addCommand("//",       alarmComm);
  sCmd.addCommand("#",       alarmOff);
  sCmd.addCommand("param",       initParam);
  commandsReg("param");
  sCmd.addCommand("NTP",        initNTP);
  sCmd.setDefaultHandler(unrecognized);
  sCmd.addCommand("TIMERS",        initTimers);
#ifdef pinOutM
  sCmd.addCommand("PINOUT",       initPinOut);
  sCmd.addCommand("RELAY",       initRelay);
#endif
#ifdef buzzerM
  sCmd.addCommand("BUZZER",       initBuzzer);
#endif
#ifdef TachM
  sCmd.addCommand("TACH",       initTach);
#endif
#ifdef irM
  sCmd.addCommand("IR-RECEIVED",       irReceived);
#ifdef  irTransmitterM
  sCmd.addCommand("IR-TRANSMITTER",     irTransmitter);
#endif
#endif
#ifdef rgbM // #endif
  sCmd.addCommand("RGB",       initRGB);
#endif
#ifdef A0M
  sCmd.addCommand("A0",       initA0);
#endif
#ifdef DS18B20M //
  sCmd.addCommand("DS18B20",       initOneWire);
#endif
#ifdef DHTM //
  sCmd.addCommand("DHT",       initDHT);
#endif
#ifdef PWMServoM // #endif
  sCmd.addCommand("PCA9685",       initPCA9685);
#endif
#ifdef rfM // #endif
  sCmd.addCommand("RF-RECEIVED",       rfReceived);
  sCmd.addCommand("RF-TRANSMITTER",     rfTransmitter);
  sCmd.addCommand("RF-LIVOLO",     rfLivolo);
#endif
}

#ifdef evonicfires //
void initLB() {
  modulesReg("light_box");
}
#endif

void unrecognized(const char *command) {
  // Serial.println("What?");
}
void alarmComm() {
  //Serial.println("Comment?");
}
void alarmOff() {
  //Serial.println("CommandOff?");
}
// Настраивает Serial по команде sCmd.addCommand("UART",       uart);
void uart() {
  Serial.end();
  Serial.begin(readArgsInt());
  delay(100);
  Serial.println();
  sCmd.addCommand("print",       printTest);
  //addAction(toneS);
  commandsReg("print");
  sendStatus("uart", "");
  //actionsReg("uart");
  modulesReg("uart");
}
// По комманде print печатает аргумент для тастов
void printTest() {
  String test = readArgsString();
  Serial.println(test);
  sendStatus("PRINT", test);
}

void handleUart() {
  static String uart;
  static String key;
  if (Serial.available()) {
    byte inByte = Serial.read();
    // char inChar = Serial.read();
    char inChar = (char)inByte;
    // int inByte = Serial.read();
    sendStatus("u", inChar);
    //if (Serial.find())
    if (key != "k" && inByte == 3) {
      key += "k";
      sendStatus("u1", key);
    }
    if (key != "key" && inByte == 170) {
      key += "ey";
      sendStatus("u1", key);
    }
    if (key == "key" && inByte < 8) {
      key += " ";
      key += inByte;
      key += " ";
      sendStatus("u1", key);
    }
    if (key.length() == 6) {
      key += inByte;
      sendStatus("uart", key);
      key="";
      uart = "";
    } //else
    uart += inChar;
    if (uart.lastIndexOf("\r\n") != -1) {
      uart.replace("\r\n", "");
      flag = sendStatus("uart", uart);
      uart = "";
    }
  }
}

// Включить модуль ADMIN
void initAdmin() {
  modulesReg("admin");
}
// По комманде GET происходит запрос на сервер
void initGet() {
  String urls = readArgsString();
  if (urls.indexOf("{{") != -1) {
    String param = urls;
    do {
      param = deleteBeforeDelimiter(param, "{{");
      String test = selectToMarker(param, "}}");
      param = deleteBeforeDelimiter(param, "}}");
      urls.replace("{{" + test + "}}", getStatus(test));
    } while (param.length() != 0);
  }
  //Serial.println(urls);
  String answer = emptyS;
  HTTPClient http;
  http.begin(urls); //HTTP
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    answer = http.getString();
  }
  http.end();
}

// Читает аргументы из команд каждый слежующий вызов читает следующий аргумент возвращает String
String readArgsString() {
  String arg;
  arg = sCmd.next();
  //Serial.println(arg);
  if (arg == emptyS) arg = emptyS;
  return arg;
}
// Читает аргументы из команд каждый слежующий вызов читает следующий аргумент возвращает Int
int readArgsInt() {
  char *arg;
  arg = sCmd.next();
  if (arg != NULL) {
    return atoi(arg);
  }
  else {
    return 0;
  }
}
