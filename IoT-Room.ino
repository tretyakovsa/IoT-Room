#include "txt.h"
#include "sets.h"
void setup() {
  //system_update_cpu_freq(SYS_CPU_160MHZ);
  //Serial.begin(115200);
  //Serial.println();
  chipID = String( ESP.getChipId() ) + "-" + String( ESP.getFlashChipId() );
  TickerScheduler(1);
  initCMD();
  initHTTP();
  configSetup = readFile(fileConfigS, 4096 );
 // Serial.println(configSetup);
  initWIFI();
  String testIp = MyWiFi.StringIP();
  if (testIp == "(IP unset)") testIp = "192.168.4.1";
  if (testIp == "") testIp = "192.168.4.1";
  jsonWrite(ssdpList, getSetup(ssdpS), testIp);
  jsonWrite(modules, ssdpS, getSetup(ssdpS));
  jsonWrite(modules, ipS, testIp);
  jsonWrite(modules, configsS, getSetup(configsS));
  jsonWrite(modules, mailS, getSetup(mailS));
  setupToOptions(configsS);
  setupToOptions(langS);
  sendStatus("firstLoad", 0);
  jsonWrite(modules, langS, getSetup(langS));
  sendOptions(urlsPathS, urlsPath);
  sendOptions(fNameSPIFFSS, fNameSPIFFS);
  sendOptions(fNameBuildS, fNameBuild);
  setupToOptions(spiffsDataS);
  setupToOptions(buildDataS);
  initPuls();
  initUpgrade();
  initSSDP();
#ifdef webSoketM // #endif
  webSoket_init();
#endif
  String configs = getSetup(configsS);
  configs.toLowerCase();
  String test = readFile(configsS + "/" + configs + ".txt", 4096);
  test.replace("\r\n", "\n");
  test += "\n";
  goCommands(test);
  test = emptyS;
  //initTimers();
  initScenary();

}

void loop() {
  handleUart();
  MyWiFi.DNSRequest();
  ts.update();
  HTTP.handleClient();
  handleSSDP();
  handleScenary();
#ifdef TachM
  handleButtons();
#endif
#ifdef irM
  handleIrReceiv();
#endif
#ifdef rgbM
  ws2812fx[0].service();
  ws2812fx[1].service();
#endif
#ifdef webSoketM // #endif
  webSocket.loop(); // Работа webSocket
  WebSocketsClient.loop();
#endif
#ifdef rfM //
handleRfReceiv();
#endif
}
