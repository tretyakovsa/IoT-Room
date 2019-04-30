// ------------- Значение по умолчанию
String defaultTestString(String test, String def) {
  if (test == emptyS or test == def) {
    test = def;
  }
  return test;
}
// ------------- Значение по умолчанию
String defaultTestStringMAC(String test, String def) {
  if (test == emptyS or test == def) {
    test = def+"-"+WiFi.macAddress().c_str();
  }
  return test;
}

// --------------------Выделяем строку до маркера --------------------------------------------------
String selectToMarker (String str, String found) {
  int p = str.indexOf(found);
  return str.substring(0, p);
}

// -------------------Выделяем строку от конца строки до маркера ---------------------
String selectToMarkerLast (String str, String found) {
  int p = str.lastIndexOf(found);
  return str.substring(p + found.length());
}

//----------------------Удаляем все до символа разделителя -----------------
String deleteBeforeDelimiter(String str, String found) {
  int p = str.indexOf(found) + found.length();
  return str.substring(p);
}
//----------------------Удаляем все до символа разделителя -----------------
String deleteBeforeDelimiterTo(String str, String found) {
  int p = str.indexOf(found);
  return str.substring(p);
}
// -------------------Удаляем строку от конца строки до маркера ---------------------
String deleteToMarkerLast (String str, String found) {
  int p = str.lastIndexOf(found);
  return str.substring(0, p);
}

//--------Выделяем строку от маркера до маркера
String selectFromMarkerToMarker(String str, String found, int number) {
  if (str.indexOf(found) == -1) return "not found"; // если строки поиск нет сразу выход
  str += found; // добавим для корректного поиска
  uint8_t i = 0; // Индекс перебора
  do {
    if (i == number) return selectToMarker(str, found); // если индекс совпал с позицией законцим вернем резултат
    str = deleteBeforeDelimiter(str, found);  // отбросим проверенный блок до разделителя
    i++; // увеличим индекс
  } while (str.length() != 0); // повторим пока строка не пустая
  return "not found"; // Достигли пустой строки и ничего не нашли
}

// ------------- Данные статистики -----------------------------------------------------------
void statistics() {
  String urls = urlsStat;
  if (urls == emptyS) return; // Если urlsStat пустой статистику не отправлять
  urls += MyWiFi.macAddress().c_str();
  urls += "&";
  urls += getSetup(configsS);
  urls += "&";
  urls += ESP.getResetReason();
  urls += "&";
  urls += getSetup(spiffsDataS);
  String stat = MyWiFi.getURL(urls);
  Serial.println(stat);
  sendOptions(messageS, jsonRead(stat, messageS)); // Вернем статус регистрации или пусто если нет сети
}

//------------------Выполнить все команды по порядку из строки разделитель \r\n  \n
String goCommands(String inits) {
  //Serial.println(inits);
  String temp;
  String rn = "\n";
  inits += rn;
  //  Serial.println(writeFile("inits.txt", inits));
  do {
    temp = selectToMarker (inits, rn);

    //    Serial.println(temp);
    sCmd.readStr(temp);
    inits = deleteBeforeDelimiter(inits, rn);
  } while (inits.indexOf(rn) != 0);
  return "OK";
}

// ------------- Чтение файла в строку --------------------------------------
String readFile(String fileName, size_t len ) {
  File configFile = SPIFFS.open("/" + fileName, "r");
  if (!configFile) {
    return "Failed";
  }
  size_t size = configFile.size();
  if (size > len) {
    configFile.close();
    return "Large";
  }
  String temp = configFile.readString();
  configFile.close();
  return temp;
}
// ------------- Запись строки в файл --------------------------
String writeFile(String fileName, String strings ) {
  File configFile = SPIFFS.open("/" + fileName, "w");
  if (!configFile) {
    return "Failed to open file";
  }
  configFile.print(strings);
  configFile.close();
  return "Write sucsses";
}
// ------------- Запись файла конфигурации ----------------------------------
void saveConfigSetup () {
  writeFile(fileConfigS, configSetup );
}

// ------------- Проверка занятости пина --------------------------
/*
   Алгоритм
   Провнряем свободен ли пин если нет вернем 17
   Если свободен то займем pins[pin] = true;

*/
uint8_t pinTest(uint8_t pin) {
  //Serial.print("pin");
  //Serial.print("=");
  if (pin > 20) {
    pin = 17;
  } else {
    if (pins[pin]) {
      pin = 17 ;
    }
    else {
      pins[pin] = true;
      if (getOptions("flashChipMode") != "DOUT") {
        if (pin > 5 && pin < 12) pin = 17 ;
        if (pin == 1 || pin == 3)  Serial.end();
      } else {
        if ( (pin > 5 && pin < 9) ||  pin == 11) pin = 17 ;
      }
    }
  }
  //Serial.println(pin);
  return pin;
}
uint8_t pinTest(uint8_t pin, boolean multi) {
  //Serial.print("multiPin");
  //Serial.print("=");
  if (pin > 20) {
    pin = 17;
  } else {
    pins[pin] = !multi;
    if (pins[pin]) {
      pin = 17 ;
    }
    else {
      pins[pin] = true;
      if (getOptions("flashChipMode") != "DOUT") {
        if (pin > 5 && pin < 12) pin = 17 ;
        if (pin == 1 || pin == 3)  Serial.end();
      } else {
        if ( (pin > 5 && pin < 9) ||  pin == 11) pin = 17 ;
      }
    }
  }
  //Serial.println(pin);
  return pin;
}
// -------------- Добавить действие
void addAction(String nameAction, String num){
  commandsReg(nameAction);
  actionsReg(nameAction + num);
  modulesReg(nameAction + num);
  }
// -------------- Добавить действие
void addAction(String nameAction){
  addAction(nameAction,"");
  }
// -------------- Регистрация модуля
void modulesReg(String modName) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(modules);
  json[ssdpS] = jsonRead(configJson, ssdpS);
  json[spaceS] = jsonRead(configJson, spaceS);
  JsonArray& data = json["module"].asArray();
  data.add(modName);
  modules = emptyS;
  json.printTo(modules);
}

// -------------- Регистрация команд
void commandsReg(String comName) {
  if (regCommands.indexOf(comName) == -1) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.parseObject(regCommands);
    JsonArray& data = json["command"].asArray();
    data.add(comName);
    regCommands = emptyS;
    json.printTo(regCommands);
  }
}
// -------------- Регистрация actions
void actionsReg(String actionsName) {
  jsonWrite(pulsList, actionsName, pulsNum);
  pulsNum++;
}

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

// param add fire 0
// param send fire 1
// param on fire
// param off fire
// param not fire
// param + fire 1
// param - fire 1
void initParam() {
  String com = readArgsString(); //Комманда
  String key = readArgsString(); // Имя
  String volume = readArgsString();   // Данные
  String oldVolume = getStatus(key);
  if (com == "add") sendStatus(key, volume);
  if (com == "send") flag = sendStatus(key, volume);
  if (com == "on") flag = sendStatus(key, 1);
  if (com == "off") flag = sendStatus(key, 0);
  if (com == "not") flag = sendStatus(key, !getStatusInt(key));
  if (key != "") {
    //Serial.println(key);
    //SoketData (key, getStatus(key), oldVolume);
  }
}
