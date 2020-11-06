#ifdef pinShimM
// ------------------- Инициализация Шим вывода
void initShimOut() {
  uint8_t pin = readArgsInt(); // первый аргумент pin
  pin =  pinTest(pin);
  String num = readArgsString(); // второй аргумент прификс реле 0 1 2
  uint16_t state = readArgsInt(); // третий  аргумент состояние на старте
  boolean inv = readArgsInt(); // четвертый аргумент инверсия выхода
  String title = readArgsString(); // Пятый аргумент подпись
  String nameR = sOutS + num;
  if (title == "") title = nameR;
  sendStatus(nameR, state,1);
  sendOptions(sOutS + PinS + num, pin);
  sendOptions(sOutS + "Old" + num, state);
  sendOptions(sOutS + NotS + num, inv);
  if (pin < 17) { //  это шим через GPIO
    if (inv) {
      analogWrite(pin, 1023 - state);
    }
    else analogWrite(pin, state);
  }
  sCmd.addCommand(sOutS.c_str(), pinShimOut); //
  commandsReg(sOutS);
  actionsReg(sOutS + num);
  modulesReg(sOutS + num);
}
// http://192.168.0.91/cmd?command=relay off 1
void pinShimOut() {
  String com = readArgsString(); // действие
  String num = readArgsString(); // номер реле
  pinShimSet(num, com, sOutS);
}

void pinShimSet(String num, String com, String name) {
  String kayPin = name + PinS + num; // Получим имя ячейки пин по номеру
  String kay = name + num; // Имя реле
  uint8_t pin = getOptionsInt(kayPin); // Получим пин по Имени реле
  uint8_t inv = getOptionsInt(name + NotS + num); // Получим признак инверсии по Имени реле
  uint16_t state = getStatusInt(kay); // Получим статус реле по Имени
  // Проверим команду приготовим новый state
  if (com.toInt() != 0) state = com.toInt();
  if (com == onS ) state = getOptionsInt(sOutS + "Old" + num);
  if (com == offS || com == "0") state = 0;
  if (com == notS) {
    state = 1023 - state;
  }
  if (pin < 17) { //  это шим через GPIO
    if (inv) {
      analogWrite(pin, 1023 - state);
    }
    else analogWrite(pin, state);
  }
  if (state != 0)sendOptions(sOutS + "Old" + num, state);
  flag = sendStatus(kay, state);
  statusS = htmlStatus(configJson, kay, langOnS, langOffS);
}
#endif

#ifdef pinOutM
// ------------------- Инициализация Реле
void initRelay() {
  uint8_t pin = readArgsInt(); // первый аргумент pin
  pin =  pinTest(pin);
  String num = readArgsString(); // второй аргумент прификс реле 0 1 2
  boolean state = readArgsInt(); // третий  аргумент состояние на старте
  boolean inv = readArgsInt(); // четвертый аргумент инверсия выхода
  String title = readArgsString(); // Пятый аргумент подпись
  initPin(pin, num, state, inv, relayS, title);
  sCmd.addCommand(relayS.c_str(), relay); //
  commandsReg(relayS);
  actionsReg(relayS + num);
  modulesReg(relayS + num);
}

// ------------------- Инициализация PinOut
void initPinOut() {
  uint8_t pin = readArgsInt(); // первый аргумент pin
  pin =  pinTest(pin);
  String num = readArgsString(); // второй аргумент прификс pina 0 1 2
  boolean state = readArgsInt(); // третий  аргумент состояние на старте
  boolean inv = readArgsInt(); // четвертый аргумент инверсия выхода
  String title = readArgsString(); // Пятый аргумент подпись
  //String name = pinOutS;
  //initPin(pin, num, state, inv, name, title);
  initPin(pin, num, state, inv, pinOutS, title);
  sCmd.addCommand(pinOutS.c_str(), pinOut); //
  commandsReg(pinOutS);
  actionsReg(pinOutS + num);
  modulesReg(pinOutS + num);
}

void initPin(uint8_t pin, String num, boolean state, boolean inv, String name, String title) {
  String nameR = name + num;
  if (title == "") title = nameR;
  sendStatus(nameR, state,1);
  sendOptions(name + PinS + num, pin);
  sendOptions(name + NotS + num, inv);
  if (pin < 17) { //  это реле через GPIO
    pinMode(pin, OUTPUT);
    digitalWrite(pin, state ^ inv);
  }
  if (pin > 17 && pin <= 21 ) { //это реле через UART
    if (modules.indexOf(name + "uart") == -1) {
      modulesReg(name + "uart");
    }
    relayWrite(num.toInt(), state ^ inv);
  }
#ifdef PWMServoM // #endif
  if (pin > 21 && pin <= 36) { // это реле через PCA9685
    if (modules.indexOf(i2cS) == -1) {
      PCA9685Write(pin, state ^ inv);
    }
  }
#endif
#ifdef pinExt
  if (modules.indexOf(i2cS) == -1) {
    PCA9685Write(pin, state ^ inv);
    pcf8574.write(pin, pcf8574.read(3));
  }
#endif
}

#ifdef PWMServoM // #endif
void initPCA9685() {
  if (modules.indexOf(i2cS) == -1) {
    pwm.begin();
  }
}

void PCA9685Write(uint8_t vpin, boolean state) {
  uint8_t pin = vpin - 22;
  if (state) {
    pwm.setPWM(pin, 4096, 0); //on
  } else
    pwm.setPWM(pin, 0, 4096); // off
}
#endif

#ifdef pinExt
void initPCF8574() {
  if (modules.indexOf(i2cS) == -1) {
    pcf8574.begin();
  }
}
void PCF8574Write(uint8_t vpin, boolean state) {
  if (modules.indexOf(i2cS) == -1) {
    uint8_t pin = vpin - 38;
    pcf8574.write(pin, state);
  }
}
#endif

// http://192.168.0.91/cmd?command=relay off 1
void relay() {
  String com = readArgsString(); // действие
  String num = readArgsString(); // номер реле
  pinSet(num, com, relayS);
}

// http://192.168.0.91/cmd?command=relay off 1
void pinOut() {
  String com = readArgsString(); // действие
  String num = readArgsString(); // номер реле
  pinSet(num, com, pinOutS);
}

void pinSet(String num, String com, String name) {
  String kayPin = name + PinS + num; // Получим имя ячейки пин по номеру
  String kay = name + num; // Имя реле
  uint8_t pin = getOptionsInt(kayPin); // Получим пин по Имени реле
  uint8_t inv = getOptionsInt(name + NotS + num); // Получим признак инверсии по Имени реле
  uint8_t state = getStatusInt(kay); // Получим статус реле по Имени
  // Проверим команду приготовим новый state
  if (com == onS || com == "1" ) state = 1;
  if (com == offS || com == "0") state = 0;
  if (com == notS) state = !(state);
  if (pin < 17) { //  это реле через GPIO
    digitalWrite(pin, state ^ inv);
  }
  if (pin > 17 && pin <= 21 ) { //это реле через UART
    relayWrite(num.toInt(), state ^ inv);
  }
#ifdef PWMServoM // #endif
  if (pin > 21) { // это реле через PCA9685
    PCA9685Write(pin, state ^ inv);
  }
#endif
  flag = sendStatus(kay, state);
  statusS = htmlStatus(configJson, kay, langOnS, langOffS);
}

// -------------- Для управления реле по UART
// https://www.banggood.com/ru/ESP8266-5V-WiFi-Relay-Module-Internet-Of-Things-Smart-Home-Phone-APP-Remote-Control-Switch-p-1126605.html?rmmds=category

void relayWrite(uint8_t num, boolean state) {
  uint8_t crc = 0;
  byte miBufferON[] = {0xA0, num, state, 0xA0};
  uint8_t sizeBuf = sizeof(miBufferON);
  for (uint8_t i = 0; i < sizeBuf - 1; i++) {
    crc = crc + miBufferON[i];
  }
  miBufferON[sizeBuf - 1] = crc;
  Serial.write(miBufferON, sizeof(miBufferON));
}
#endif
String htmlStatus(String json, String state, String sOn, String sOff) {
  String out = "{}";
  if (jsonReadToInt(json, state)) {
    jsonWrite(out, titleS, sOff);
    jsonWrite(out, classS, btnS + infoS);
  }
  else {
    jsonWrite(out, titleS, sOn);
    jsonWrite(out, classS, btnS + primaryS);
  }
  return out;
}

/*
  XdrvSetPower(rpower);

  if ((SONOFF_DUAL == Settings.module) || (CH4 == Settings.module)) {
    Serial.write(0xA0);
    Serial.write(0x04);
    Serial.write(rpower &0xFF);
    Serial.write(0xA1);
    Serial.write('\n');
    Serial.flush();
  }
   где вместо "0xxx" - 0 (выключить оба реле), 1 (включить одно реле), 2 (включить второе реле), 3 (включить оба реле), то можно таким образом управлять релюшками.
*/
#ifdef buzzerM
// ------------------- Инициализация Buzera
void initBuzzer() {
  uint8_t pin = readArgsInt(); // первый аргумент pin
  pin =  pinTest(pin);
  sendOptions(buzzerPinS, pin);
  sCmd.addCommand(toneS.c_str(), buzzerTone);
  commandsReg(toneS);
  modulesReg(toneS);
}

void buzzerTone() {
  int freq = readArgsInt();
  int duration = readArgsInt();
  uint8_t pin = getOptionsInt(buzzerPinS);
  tone(pin, freq, duration);
}
#endif
#ifdef PulsM
// ------------------- Инициализация Импульс
void initPuls() {
  sCmd.addCommand(pulseS.c_str(), startPuls);
  commandsReg(pulseS);
  modulesReg(pulseS);
}

void startPuls() {
  String com = readArgsString(); // on off
  if (com != "") { // если комманда есть
    String pulseCom = readArgsString(); // Команда relay3 или rgb
    String tacks = jsonRead(pulsList, pulseCom);  //Получим номер задачи для устройства
    sendOptions(pulseS + "State" + tacks, false);
    pulseCom = topicToCom(pulseCom);   // Пробел между командой и номером
    pulseCom.replace(" ", " not ");    // Модефицируем командув not
    sendOptions(pulseComS + tacks, pulseCom); // Сохраним команду
    if (com == onS || com == "1") {         // Если комманда есть
      int freq = stringToMilis (readArgsString(), 1); // Как долго включен
      sendOptions(pulseS + tacks + "0", freq);
      if (freq != 0) {
        String temp = readArgsString(); // Как долго выключен
        int freq1 = temp.toInt();
        if (temp == "-")freq1 = freq;
        if (temp == "")freq1 = 0;
        sendOptions(pulseS + tacks + "1", freq1);
        int period = freq + freq1;
        String pulseTime = readArgsString(); // Время работы
        int pulseTimeInt = stringToMilis(pulseTime, period);
        int remainder = pulseTimeInt % (period);
        if (remainder > period / 2) {
          pulseTimeInt += period - remainder;
        } else  pulseTimeInt -= remainder;
        if (getStatusInt(pulseCom)) {
          pulseCom.replace(notS, offS);          // Модефицируем командув off
          sCmd.readStr(pulseCom);
        }
        sendOptions(pulseTimeS + tacks, pulseTimeInt);
        imPuls(tacks.toInt());
      }
    }
    if (com == "off" || com == "0") {
      pulseCom.replace(notS, offS);
      sCmd.readStr(pulseCom);
      flipper[tacks.toInt()].detach();
    }
  }
}
int stringToMilis(String times, int period) {
  int p = times.length();
  String unit = times.substring(p - 1, p);
  int timei = times.toInt();
  if (unit == "s") timei *= 1000;
  if (unit == "m") timei *= 60000;
  if (unit == "h") timei *= 3600000;
  if (unit == "i") timei *= period;
  return timei;
}
void imPuls(int tacks) {
  String pulseStateN = "pulseState" + (String)tacks;
  boolean stopF = true;
  String pulseCom = getOptions(pulseComS + tacks);           // Получить каким устройством управляем
  String pulseTime = getOptions(pulseTimeS + tacks);         // Получим текстовое значние времени работы
  int pulseTimeInt = pulseTime.toInt();                 // Получим int значние времени работы
  uint8_t low = getOptionsInt(pulseStateN);
  int timeOn = getOptionsInt(pulseS + tacks + low); // Время включено
  int timeOff = getOptionsInt(pulseS + tacks + !low); // Время выключено
  if (timeOn > 0) {                                     // Если время включено >0 сразу закончить

    if (!low) {
      pulseCom.replace(notS, onS);
      //Serial.println(pulseCom);
    }
    else {
      pulseCom.replace(notS, offS);
      //Serial.println(pulseCom);
    }

    sCmd.readStr(pulseCom);                             // Выполнить команду
    if (pulseTime != "null" && pulseTimeInt != 0 ) {
      sendOptions(pulseTimeS + tacks, (String)(pulseTimeInt - timeOn));
      if (getOptionsInt(pulseTimeS + tacks) <= 0) {
        flipper[tacks].detach();
        stopF = false;
      }
    }
    low = !low;
    sendOptions(pulseStateN, low);
    if (stopF) {
      flipper[tacks].attach_ms(timeOn, imPuls, tacks);               // Задать время через которое процедура будет вывана повторно
    }
  } else {
    sCmd.readStr(pulseCom);                            // Выключить
    flipper[tacks].detach();                               // Остановим таймер
    //low = false;                                      // Сбросить флаг ???
    sendOptions(pulseStateN, false);
  }
}

String topicToCom (String topicS) {
  uint8_t   p = 0;
  boolean f = true;
  uint8_t   u = topicS.length();
  while (p != u) {
    if  (isDigit(topicS.charAt(p))) {
      String kay = topicS.substring(0, p);
      //Serial.println(topicS.charAt(p));
      //Serial.println(kay);
      topicS.replace(kay, kay + " ");
      yield();
      f = false;
    }
    p++;
  }
  if (f) topicS += " ";
  return topicS;
}
#endif

#ifdef pultM // 
// Настраивает SoftSerial по команде sCmd.addCommand("PULT",       pultInit);
void pultInit() {
  SerialMy.end();
  int speed = readArgsInt();
  if (speed != 0) {
    SerialMy.begin(speed);
  }
  else  SerialMy.begin(9600);
  delay(100);
  sCmd.addCommand(staS.c_str(), statusF);
  modulesReg("pult");
}
void statusF() {
  //Serial.println("statusF");
  String str;
  jsonWrite(str, "temperature", getStatus("temperature1"));
  SerialMy.println(str);
  str = "{}";
  jsonWrite(str, "templevel", getStatus("templevel"));
  SerialMy.println(str);
  str = "{}";
  jsonWrite(str, timeS, getStatus(timeS));
  SerialMy.println(str);
  SerialMy.println(getEffect);

}
void handlePult() {
  if (modules.indexOf("pult") != -1) {
    if (SerialMy.available() > 0) {
      uint8_t temp = SerialMy.read();
      if (temp < 10) {
        uartRXstr += temp;
        if (uartRXstr.indexOf("3") == -1) uartRXstr = "";
        if (uartRXstr.length() == 3) {
          //Serial.println(uartRXstr);
          sendOptions(voiceS, uartRXstr);
          flag = sendStatus(voiceS, uartRXstr);
          uartRXstr = "";
        }
      } else uartRX += char(temp);
    }
    //if (uartRX.indexOf("{") == -1 && uartRX.length() < 0) uartRX = "";
    if (uartRX.indexOf("{") == -1) uartRX = "";
    if (uartRX.indexOf("\r\n") != -1) {
      //Serial.print(uartRX);
      String com = selectToMarker(uartRX, "\r\n");
      remoutget(com);
      uartRX = deleteBeforeDelimiter(uartRX, "\r\n");
    }
  }
}
void remoutget(String command ) {
  String cmd;
  cmd = jsonRead(command, voiceS); // Прислан макрос
  if (cmd != "") {
    sendOptions(pultS, cmd);
    sendOptions(voiceS, cmd);
    flag = sendStatus(voiceS, cmd);
  }
  cmd = jsonRead(command, "cmd");   // Прислана комманда
  if (cmd != "") {
    sendOptions(pultS, cmd);
    //Serial.println(cmd);
    sCmd.readStr(cmd);
  }
}
#endif
