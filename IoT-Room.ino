#include "txt.h"
#include "sets.h"
void setup() {
  Serial.begin(115200);
  Serial.println();
   chipID = String( ESP.getChipId() ) + "-" + String( ESP.getFlashChipId() );
  TickerScheduler(1);
  initCMD();
  initHTTP();
  configSetup = readFile(fileConfigS, 4096 );
  configSetup.replace("\r\n", "");
  if (configSetup == "") configSetup = "{}";
  //sendOptions(configsEEPROMS, loadStringEEPROM());
  //Serial.print("EEPROM=");
  //Serial.println(loadStringEEPROM());
  if (getSetup(langS) == "") sendSetup(langS, "en"); // Язык
  if (getSetup(configsS) == "") sendSetup(configsS, "manually"); // конфигурация
  if (getSetup(setIndexS) == "") sendSetup(setIndexS, setIndexDef); // Скин
  if (getSetup(logoS) == "") sendSetup(logoS, "0"); // Логотип
  if (getSetup(shopS) == "") sendSetup(shopS, "0"); // Режим поиска дом магазин
  if (getSetup(fahrenheitS) == "") sendSetup(fahrenheitS, 0); // e-mail
 // if (getSetup(templevelS) == "") sendSetup(templevelS, 21); // e-mail
  if (getSetup(ssidS) == "") sendSetup(ssidS, emptyS); // e-mail
  if (getSetup(ssidPassS) == "") sendSetup(ssidPassS, emptyS); // e-mail
  if (getSetup(ssidAPS) == "") sendSetup(ssdpS, emptyS); // e-mail
  if (getSetup(ssidApPassS) == "") sendSetup(ssidApPassS, emptyS); // e-mail
  if (getSetup(dnsS) == "") sendSetup(dnsS, emptyS); // e-mail
//  if (getSetup(mailS) == "") sendSetup(mailS, emptyS); // e-mail
//  if (getSetup(passS) == "") sendSetup(passS, emptyS); // Пароль алеха
  Serial.println(configSetup);
  initWIFI();
  String testIp = MyWiFi.StringIP();
  if (MyWiFi.modeSTA()) {
    if (getSetup(flagUpdateS) == "1") webUpdate();
  }
  jsonWrite(ssdpList, getSetup(ssdpS), testIp);
  sendOptions(ipS, testIp);
  jsonWrite(modules, ssdpS, getSetup(ssdpS));
  jsonWrite(modules, configsS, getSetup(configsS));
 // jsonWrite(modules, mailS, getSetup(mailS));
//  setupToModules(logoS);          // Передать значение mail из Setup в Modules
  jsonWrite(modules, buildDataS, getSetup(buildDataS));
  jsonWrite(modules, shopS, getSetup(shopS));
  jsonWrite(modules, langS, getSetup(langS));
  setupToOptions(configsS);
  setupToOptions(langS);
  sendStatus("firstLoad", 0,1);
  setupToOptions("SSDP1");
  // Начальные параметры для сенсора температуры
  setupToStatus(fahrenheitS);
  sendStatus(temperatureS, -127,1);
//  setupToStatus(templevelS);
  jsonWrite(modules, langS, getSetup(langS));
//  sendOptions(urlsPathS, urlsPath);
  setupToOptions(spiffsDataS);
  setupToOptions(buildDataS);
  initUpgrade();
  initSSDP();
//  heapSend();
#ifdef webSoketM // #endif
  webSoket_init();
#endif
  String configs = getSetup(configsS);
  configs.toLowerCase();
  Serial.println(configs);
  String test = configsS + "/" + configs + ".txt";
  Serial.println(test);
  goCommands(test);
  //goCommands1(test);
  initScenary();
#ifdef PulsM // 
  initPuls();
#endif
#ifdef webSoketM // 
  webSoket_init();
#endif
}

void loop() {
  MyWiFi.loop();
  ts.update();
  handleSSDP();
  HTTP.handleClient();
  handleScenary();
#ifdef TachM
  handleButtons();
#endif
#ifdef webSoketM // #endif
  webSocket.loop(); // Работа webSocket
  if (MyWiFi.status() == 3)  client.poll();
#endif
#ifdef rgbM // 
  ws2812fx[0].service();
  ws2812fx[1].service();
#endif
#ifdef pultM // 
  handlePult();
#endif
#ifdef rfM // #endif
handleRfReceiv();
#endif
}
