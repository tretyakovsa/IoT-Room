// ---------------Инициализация модулей
void initCMD() {
  sCmd.addCommand("UART",       uart);
  sCmd.addCommand("print",       printTest);
  sCmd.addCommand("ADMIN",       initAdmin);
  sCmd.addCommand("GET",       initGet);
  sCmd.addCommand("//",       alarmComm);
  sCmd.addCommand("#",       alarmOff);
  commandsReg("GET");
  sCmd.setDefaultHandler(unrecognized);
  sCmd.addCommand("NTP",        initNTP);
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
#endif
#ifdef rgbM // #endif
  sCmd.addCommand("RGB",       initRGB);
  #endif
}

void unrecognized(const char *command) {
  Serial.println("What?");
}
void alarmComm() {
  //Serial.println("Comment?");
}
void alarmOff() {
  //Serial.println("CommandOff?");
}
// Настраивает Serial по команде sCmd.addCommand("Serial",       uart);
void uart() {
  Serial.end();
  Serial.begin(readArgsInt());
  delay(100);
  Serial.println();
}
// По комманде print печатает аргумент для тастов
void printTest() {
  Serial.println("Test " + readArgsString());
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
