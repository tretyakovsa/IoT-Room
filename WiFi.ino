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
  ts.add(tDBM, nTest_Time, [&](void*) {
    int temp = MyWiFi.RSSI();
#ifdef webSoketM // #endif
    SoketData (dbmS, (String)temp, getStatus(dbmS));
    //ClientSoketData (dbmS, (String)temp, getStatus(dbmS));
#endif
    flag = sendStatus(dbmS, temp);
  }, nullptr, true);
  if (MyWiFi.modeSTA()) {
    //statistics(statStart); // Если подключились к роутеру отправим статистику
    statistics();
  }
  sendSetup(ipS, MyWiFi.StringIP());
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
void statistics() {
  //Serial.println("statistics");
  String urls = "http://backup.privet.lv/visitors/?";
  urls += WiFi.macAddress().c_str();
  urls += "&";
  urls += getSetup(configsS);
  urls += "&";
  urls += ESP.getResetReason();
  urls += "&";
  urls += getSetup(spiffsDataS);
  String stat = MyWiFi.getURL(urls);
  sendOptions(messageS, jsonRead(stat, messageS));
  //sendOptions(messageS, stat);
  //sendOptions("message1", urls);
}
void statistics(uint8_t save) {
  //Serial.println("statistics");
  String urls = "http://backup.privet.lv/visitors/?";
  urls += WiFi.macAddress().c_str();
  urls += "&";
  urls += getSetup(configsS);
  urls += "&";
  urls += ESP.getResetReason();
  urls += "&";
  urls += getOptions(spiffsDataS);
  String stat = MyWiFi.getURL(urls);
  sendOptions(messageS, jsonRead(stat, messageS));
  //sendOptions(messageS, stat);
  //sendOptions("message1", urls);
}
