void initWIFI() {
  String ssid = getSetup(ssidS); // Читаем имя сети из flash ячейки
  String ssidPass = getSetup(ssidPassS); // Читаем пароль сети из flash ячейки
  String ssidAP = getSetup(ssdpS);//-------------------------------------------------
  String ssidApPass = getSetup(ssidApPassS); // Читаем пароль AP из flash ячейки
  MyWiFi.init(ssid, ssidPass, ssidAP, ssidApPass); // Оснавные данные для wifi
  //MyWiFi.initIP(getSetup(checkboxIPS), getSetup(ipS), getSetup(subnetS), getSetup(getwayS));
  sendSetup(ssdpS, defaultTestStringMAC(getSetup(ssdpS), ssdpDef));
  sendSetup(spaceS, defaultTestString(getSetup(spaceS), spaceDef));
  MyWiFi.setHostname(getSetup(ssdpS));
  MyWiFi.start();        // Запустим WiFi

  if (MyWiFi.modeSTA()) {
    //statistics(statStart); // Если подключились к роутеру отправим статистику
//    statistics();
  ts.add(tDBM, nTest_Time, [&](void*) {
    int temp = MyWiFi.RSSI();
#ifdef webSoketM // #endif
    SoketData (dbmS, (String)temp, getStatus(dbmS));
#endif
    flag = sendStatus(dbmS, temp);
  }, nullptr, false);
  }
  sendSetup(ipS, MyWiFi.StringIP());
  Serial.println(MyWiFi.StringIP());
  setupToOptions(ipS);
  sendSetup(getwayS, MyWiFi.StringGatewayIP());
  sendSetup(subnetS, MyWiFi.StringSubnetMask());

  // -------------------- Основные данные общего web интерфейса WIFI
  HTTP.on("/wifi.scan.json", HTTP_GET, []() {
    httpOkJson("test");
    //httpOkJson(MyWiFi.scan(false));
  });
  // перезагрузка при помощи get запроса
  HTTP.on("/restart", HTTP_GET, []() {
    if (HTTP.arg("device") == "ok") {               // Если значение равно Ок
      httpOkText("Reset OK"); // Oтправляем ответ Reset OK
      ESP.restart();                                // перезагружаем модуль
    } else {                                        // иначе
      httpOkText("No Reset"); // Oтправляем ответ No Reset
    }
  });
  // перезагрузка при помощи post запроса с установкой новых значений WiFi
  HTTP.on("/restartWiFi", HTTP_POST, []() {
    httpOkJson(wifiSet(true));
    delay(1000);
    // Отключаем точку доступа и переподключаемся к роутеру
    ESP.restart();
  });
  HTTP.on("/ssid", HTTP_GET, []() {
    sendSetupArg(ssidS);
    sendSetupArg(ssidPassS);
    sendSetupArg(subnetS);
    sendSetupArg(getwayS);
    sendSetupArg(dnsS);
    sendSetupArg(ipS);
    sendSetupArg(checkboxIPS);
    saveConfigSetup(); // --------------------------------------------------------------------
    httpOkText("OK");
    //httpOkJson(wifiSet(false));
  });
  HTTP.on("/ssidap", HTTP_GET, []() {
    sendSetupArg(ssidAPS);
    sendSetupArg(ssidApPassS);
    httpOkText();
    saveConfigSetup();
  });
  Serial.println("Модуль включен");
}
//ssidap?ssidAP=test1&ssidApPass=123456789
String wifiSet(boolean mode) {
  if (mode) {
    WiFi.mode(WIFI_AP_STA);
  }
  // Не отключаясь от точки доступа подключаемся к роутеру для получения будущего IP
  String ssid = getSetup(ssidS);
  String pass = getSetup(ssidPassS);
  //saveConfigSetup();// --------------------------------------------------------------------------
  WiFi.persistent(true);
  WiFi.begin(ssid.c_str(), pass.c_str());
  MyWiFi.isConnect();
  String state = "{\"ip\":\"" + MyWiFi.StringIP() + "\"}";
  return state;
}


// ------------- Данные статистики -----------------------------------------------------------
// 0 Данные переагрузки при включении, 1 данные токена, 2 данные обнавления
void statistics(uint8_t save) {
  String urls;
  urls += WiFi.macAddress().c_str();// mac адрес устройства
  //urls += "&";
  //urls += MyWiFi.StringIP(); // локальный ip для сервиса
  //urls += "&";
  //urls += product; // Линия продукта
  urls += "&";
  urls += getSetup(configsS); // тип Конфиурация
  urls += "&";
  String onPower;
  if (save == statStart) {  //  при старте причину запуска
    onPower = ESP.getResetReason();;
    onPower.replace("/", " ");
  }
  if (save == statToken) { // получение токена
    onPower = "token";
    onPower.replace("/", " ");
  }
  if (save == statUpdate) { // обнавление
    onPower = getOptions(onPowerS);
  }
  urls += onPower; // поле статистики Start Info
  urls += "&";
  //urls += getSetup(buildDataS);
    Serial.print("typeUpdate ");
  Serial.println(typeUpdate);
  if (typeUpdate == "spiffs")  {
    urls += getSetup(spiffsDataS);
  } else {
    if (typeUpdate == "build")  {
      urls += getSetup(buildDataS);
    } else urls += getSetup(buildDataS)+" "+getSetup(spiffsDataS);
  }
 // urls += "&";
  //urls += getSetup(mailS);
 // urls += "&";
 // urls += getSetup(passS);
  urls = urlsStat + urls;
  Serial.print("urls ");
  Serial.println(urls);
  String stat = MyWiFi.getURL(urls);
  stat=deleteBeforeDelimiterTo(stat,"{");
  stat=deleteToMarkerLast(stat,"}");
  if (stat == "") {
    sendOptions(tokenS, "error");
  } else sendOptions(serverS, jsonRead(stat, serverS));
  if (jsonRead(stat, "role") == "Admin") sCmd.readStr("ADMIN");
  sendOptions(tokenS, jsonRead(stat, tokenS));
  jsonWrite(modules, messageS, jsonRead(stat, messageS));
  sendOptions(messageS, jsonRead(stat, messageS));
}
