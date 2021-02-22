boolean inetTest() {
  HTTPClient http;
  http.begin(inetTestS);
  if (http.GET() == 200)return true;
  return false;
}

#ifdef I2CM
// ------------------- Инициализация I2C
void initI2C() {
  String pin1S = readArgsString(); // первый аргумент pin
  String pin2S = readArgsString(); // первый аргумент pin
  String clockFrequency = readArgsString(); // третий аргумент частота для связи I2C
  uint8_t pin1 = 17;
  uint8_t pin2 = 17;
  if (pin1S == "" || pin2S == "") { // если один из аргументов не задан используем 4 5
    pin1 = 4;
    pin2 = 5;
  } else {
    pin1 = pin1S.toInt();
    pin2 = pin2S.toInt();
  }
  pin1 =  pinTest(pin1);
  pin2 =  pinTest(pin2);
  if (pin1 == 17 || pin2 == 17) {} else {
    Wire.setClock(defaultTestString(clockFrequency, "400000").toInt());
    Wire.begin(pin1, pin2);
    modulesReg(i2cS);
    scanI2C();
  }

}
void scanI2C() {
  byte error, address;
  int nDevices;
  nDevices = 0;
  for (address = 1; address < 127; address++ )
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {

      //i2cList += "0x" + String(address, HEX) + ",";
      jsonArrAdd(configOptions, i2cS, "0x" + String(address, HEX));

      nDevices++;
    } else if (error == 4) {
      if (address < 16)  jsonArrAdd(configOptions, i2cS + "Error", "0x" + String(address, HEX));
    }
  }
  if (nDevices > 0)   {sendOptions("n"+i2cS, nDevices);} //Serial.println(i2cList);
}

#endif

// -------------- Регистрация модуля
void modulesReg(String modName) {
  String mod = jsonRead(modules, "module"); // читаем массив
  if (mod.indexOf(modName) == -1) jsonArrAdd(modules, "module", modName); // Если нет значения добавляем
}
// -------------- Регистрация модуля
void commandsReg(String comName) {
  String mod = jsonRead(regCommands, "command"); // читаем массив
  if (mod.indexOf(comName) == -1) jsonArrAdd(regCommands, "command", comName); // Если нет значения добавляем
  //jsonArrAdd(regCommands,"command",comName);
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
  if (str.indexOf(found) == -1) return "NAN"; // если строки поиск нет сразу выход
  str += found; // добавим для корректного поиска
  uint8_t i = 0; // Индекс перебора
  do {
    if (i == number) return selectToMarker(str, found); // если индекс совпал с позицией закончим вернем резултат
    str = deleteBeforeDelimiter(str, found);  // отбросим проверенный блок до разделителя
    i++; // увеличим индекс
  } while (str.length() != 0); // повторим пока строка не пустая
  return "NAN"; // Достигли пустой строки и ничего не нашли
}
//------------------Выполнить все команды по порядку из строки разделитель \r\n  \n
String goCommands(String inits) {
  File initsFile = SPIFFS.open("/" + inits, "r");
  if (!initsFile) {
    return "Failed";
  }
  String temp;
  while (initsFile.size() != initsFile.position()) {
    temp = initsFile.readStringUntil('\n');
    temp.replace("\r", "");
    sCmd.readStr(temp);
  }
  initsFile.close();
  return "OK";
}
void goCommand(String temp) {
  temp.replace("\r", "");
  do {
    String cTemp = selectToMarker(temp, "\n");
    sCmd.readStr(cTemp);
    temp = deleteBeforeDelimiter(temp, "\n");
  } while (temp != emptyS);
}

String goCommands(String inits, String key) {
  File initsFile = SPIFFS.open("/" + inits, "r");
  if (!initsFile) {
    return "Failed";
  }
  String scenOne = "";
  String temp;
  while (initsFile.size() != initsFile.position()) {
    temp = initsFile.readStringUntil('\n') + '\n'; // читаем секцию сценария в переменную
    scenOne += temp;
    if (temp.indexOf("id ") != -1 ) { // если считано
      if (scenOne.indexOf("if " + key + " ") != -1 || scenOne.indexOf("and " + key + " ") != -1 ) { // проверяем параметр очереди на наличие и если есть ключ выполняем сценарий
        goCommand(scenOne);
      }
      scenOne = "";
    }
  }
  initsFile.close();
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
  //Serial.println(configSetup);
  writeFile(fileConfigS, configSetup );
}
// ------------- Значение по умолчанию
String defaultTestString(String test, String def) {
  if (test == emptyS or test == def or test == "null") {
    test = def;
  }
  return test;
}
// ------------- Значение по умолчанию
String defaultTestStringMAC(String test, String def) {
  if (test == emptyS or test == def) {
    //test = def + "-" + WiFi.macAddress().c_str();
    test = def + "-" + String( ESP.getChipId());
  }
  return test;
}
// ------------- Проверка занятости пина --------------------------
/*
   Алгоритм
   Провнряем свободен ли пин если нет вернем 17
   Если свободен то займем pins[pin] = true;

*/
uint8_t pinTest(uint8_t pin) {
  if (pin > maxPin) {
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
  return pin;
}
uint8_t pinTest(uint8_t pin, boolean multi) {
  if (pin > maxPin) {
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
  return pin;
}

// -------------- Регистрация actions для функции пульс
void actionsReg(String actionsName) {
#ifdef PulsM // 
  jsonWrite(pulsList, actionsName, pulsNum);
  pulsNum++;
#endif
}

// param add fire 0
// param set fire 1
// param on fire
// param off fire
// param not fire
// param sum fire 1
// param dif fire 1
// param inc fire
// param dec fire
void initParam() {
  String com = readArgsString(); //Комманда
  String key = readArgsString(); // Имя
  String volume = readArgsString();   // Данные
  String oldVolume = getStatus(key);
  if (com == "add") sendStatus(key, volume);
  if (com == "set") flag = sendStatus(key, volume);
  if (com == "on") flag = sendStatus(key, 1);
  if (com == "off") flag = sendStatus(key, 0);
  if (com == "not") flag = sendStatus(key, !getStatusInt(key));
  if (com == "sum") flag = sendStatus(key, getStatusInt(key) + volume.toInt());
  if (com == "dif") flag = sendStatus(key, getStatusInt(key) - volume.toInt());
  if (com == "inc") flag = sendStatus(key, getStatusInt(key) + 1);
  if (com == "dec") flag = sendStatus(key, getStatusInt(key) - 1);
}

// ------------- Информация о ESP

void espInfo() {
  String adminS = "{}";
  //   jsonWrite(adminS, configsEEPROMS, getOptions(configsEEPROMS));
  jsonWrite(adminS, heapS, String(ESP.getFreeHeap()));
  jsonWrite(adminS, "flashChip", String(ESP.getFlashChipId(), HEX));
  jsonWrite(adminS, "ideFlashSize", (String)ESP.getFlashChipSize());
  jsonWrite(adminS, "realFlashSize", (String)ESP.getFlashChipRealSize());
  jsonWrite(adminS, "flashChipSpeed", (String)(ESP.getFlashChipSpeed() / 1000000));
  jsonWrite(adminS, "cpuFreqMHz", ESP.getCpuFreqMHz());
  FlashMode_t ideMode = ESP.getFlashChipMode();
  jsonWrite(adminS, "FreeSketchSpace", (String)ESP.getFreeSketchSpace());
  jsonWrite(adminS, "getSketchSize", (String)ESP.getSketchSize());
  jsonWrite(adminS, "flashChipMode", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
  FSInfo fs_info;
  SPIFFS.info(fs_info);
  jsonWrite(adminS, "totalBytes", (String)fs_info.totalBytes);
  jsonWrite(adminS, "usedBytes", (String)fs_info.usedBytes);
  jsonWrite(adminS, "blockSize", (String)fs_info.blockSize);
  jsonWrite(adminS, "pageSize", (String)fs_info.pageSize);
  jsonWrite(adminS, "maxOpenFiles", (String)fs_info.maxOpenFiles);
  jsonWrite(adminS, "maxPathLength", (String)fs_info.maxPathLength);
  jsonWrite(adminS, "buildDay", (String)__DATE__);
  jsonWrite(adminS, "buildTime", (String)__TIME__);
  jsonWrite(adminS, "mac", MyWiFi.macAddress());
  jsonWrite(adminS, "sdk", (String)ESP.getSdkVersion());
  jsonWrite(adminS, "boot", ESP.getBootVersion());
  String versionS = ESP.getCoreVersion();
  versionS.replace("_", ".");
  jsonWrite(adminS, "coreversion", versionS);
  jsonWrite(adminS, "userbin_addr", (String)system_get_userbin_addr());
  jsonWrite(adminS, "boot_mode", ESP.getBootMode() == 0 ? F("SYS_BOOT_ENHANCE_MODE") : F("SYS_BOOT_NORMAL_MODE"));
  jsonWrite(adminS, spiffsDataS, getSetup(spiffsDataS));
  jsonWrite(adminS, buildDataS, getSetup(buildDataS));
  uint8_t phyMode = MyWiFi.getPhyMode();
  String phyModeS;
  switch (phyMode) {
    case 1:
      phyModeS = "MODE_11B";
      break;
    case 2:
      phyModeS = "MODE_11G";
      break;
    case 3:
      phyModeS = "MODE_11N";
      break;
  }
  jsonWrite(adminS, "PhyMode", phyModeS);
  httpOkJson(adminS);
}
