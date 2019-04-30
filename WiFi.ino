void initWIFI() {
  String ssid = MyWiFi.SSID();
  sendSetup(ssidS, ssid);
  String ssidPass = MyWiFi.psk();
  sendSetup(ssidPassS, ssidPass);
  String ssidAP = MyWiFi.softAPSSID();
  Serial.println(ssidAP.indexOf("ESP"));
  if (ssidAP.indexOf("ESP")!= -1)  {
    ssidAP = ssidAPDef;
  }
  Serial.println(ssidAP);
  sendSetup(ssidAPS, ssidAP);
  String ssidApPass = MyWiFi.softAPPSK();
  sendSetup(ssidApPassS, ssidApPass);
  MyWiFi.init(ssid, ssidPass, ssidAP, ssidApPass, ssidAPDef);
  MyWiFi.start();
  if (MyWiFi.modeSTA()) statistics();
  Serial.println(WiFi.SSID());
  HTTP.on("/restart", HTTP_GET, []() {
    if (HTTP.arg("device") == "ok") {               // Если значение равно Ок
      httpOkText("Reset OK"); // Oтправляем ответ Reset OK
      ESP.restart();                                // перезагружаем модуль
    } else {                                        // иначе
      httpOkText("No Reset"); // Oтправляем ответ No Reset
    }
  });
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
    saveConfigSetup();
    httpOkJson(wifiSet(false));
  });
}
String wifiSet(boolean mode) {
  if (mode) {
    WiFi.mode(WIFI_AP_STA);
  }
  // Не отключаясь от точки доступа подключаемся к роутеру для получения будущего IP
  String ssid = getSetup(ssidS);
  String pass = getSetup(ssidPassS);
  saveConfigSetup();
  WiFi.persistent(true);
  WiFi.begin(ssid.c_str(), pass.c_str());
  MyWiFi.isConnect();
  String state = "{\"ip\":\"" + MyWiFi.StringIP() + "\"}";
  Serial.println(state);
  return state;
}
void sendSetupArg(String argS) {
  sendSetup(argS, HTTP.arg(argS));
}
