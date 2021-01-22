// ---------------Инициализация модулей
void initCMD() {
  sCmd.addCommand("UART",       uart);
  sCmd.addCommand("ADMIN",       initAdmin);
  sCmd.addCommand("//",       alarmCommOff);
  sCmd.addCommand("#",       alarmCommOff);
  sCmd.addCommand("param",       initParam);
  commandsReg("param");
#ifdef pinShimM
  sCmd.addCommand("SHIMOUT",       initShimOut);
  sCmd.addCommand("SHIM",       initShim);
#endif
#ifdef pinOutM
  sCmd.addCommand("PINOUT",       initPinOut);
  sCmd.addCommand("RELAY",       initRelay);
#endif
#ifdef buzzerM
  sCmd.addCommand("BUZZER",       initBuzzer);
#endif
#ifdef ntpM
  sCmd.addCommand("NTP",        initNTP);
#endif
#ifdef TimersM
  sCmd.addCommand("TIMERS",       initTimers);
#endif
#ifdef TachM
  sCmd.addCommand("TACH",       initTach);
#endif
#ifdef A0M
  sCmd.addCommand("A0",       initA0);
#endif
#ifdef dsM // 
  sCmd.addCommand("DS18B20",       initOneWire);
#endif
#ifdef dhtM // 
  sCmd.addCommand("DHT",       initDHT);
#endif
#ifdef Si7021M // 
  sCmd.addCommand("SI7021",       initSi7021);
#endif
#ifdef rgbM // 
  sCmd.addCommand("NRGB",       initRGB);
#endif
#ifdef pultM // 
  sCmd.addCommand("PULT",       pultInit);
#endif
#ifdef pult // 
  sCmd.addCommand("PULT",       pultInit);
#endif
#ifdef PWMServoM // #endif
  sCmd.addCommand("PCA9685",       initPCA9685);
#endif
#ifdef I2CM // #endif
  sCmd.addCommand("I2C",       initI2C);
#endif
#ifdef rfM // #endif
  sCmd.addCommand("RF-RECEIVED",       rfReceived);
  sCmd.addCommand("RF-TRANSMITTER",     rfTransmitter);
#endif
#ifdef RC522 // 
sCmd.addCommand("RC522",       initRC522);
#endif
#ifdef irM
  sCmd.addCommand("IR-RECEIVED",       irReceived);
#ifdef  irTransmitterM
  sCmd.addCommand("IR-TRANSMITTER",     irTransmitter);
#endif
#endif
}



void unrecognized(const char *command) {
  //Serial.println("What?");
}
void alarmCommOff() {
  //Serial.println("Comment?");
}

// Настраивает Serial по команде sCmd.addCommand("UART",       uart);
void uart() {
  Serial.end();
  Serial.begin(readArgsInt());
  delay(100);
  Serial.println();
  sCmd.addCommand("print",       printTest);
  modulesReg("uart");
}
// По комманде print печатает аргумент для тестов
void printTest() {
  Serial.println(readArgsString());
}



// Читает аргументы из команд каждый слежующий вызов читает следующий аргумент возвращает String
String readArgsString() {
  String arg;
  arg = sCmd.next();
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
