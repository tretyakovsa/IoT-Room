void initSSDP() {
  setupToOptions(ssdpS);
  setupToOptions(spaceS);
  jsonWrite(modules, ssdpS, getSetup(ssdpS));
  if (MyWiFi.modeSTA()) { //Если есть подключение к роутеру
    // Включаем определение имени для Windows
    //LLMNR.begin(temp.c_str());
    //NBNS.begin(temp.c_str());
    unsigned int localPort = 1901;
    udp.begin(localPort);
    // SSDP дескриптор
    HTTP.on("/description.xml", HTTP_GET, []() {
      SSDP.schema(HTTP.client());
    });
    SSDP.setDeviceType("upnp:rootdevice");
    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(80);
    SSDP.setName(getSetup(ssdpS));
    SSDP.setSerialNumber(chipID);
    SSDP.setURL("/");
    SSDP.setModelName(getSetup(configsS));
    SSDP.setModelNumber(chipID + "/" + getSetup(ssdpS));
    SSDP.setModelURL(modelURL);
    SSDP.setManufacturer(manufacturer);
    SSDP.setManufacturerURL(manufacturerURL);
    SSDP.begin();

    // Каждые 60 секунд проверяем не изиенился ли адрес ip
    ts.add(tIP, tIP_Time, [&](void*) {
      ipChanges();
    }, nullptr, true);
    // задача проверять наличие устройств в сети каждые 60 секунд.
    ts.add(tSSDP, tSSDP_Time, [&](void*) {
      requestSSDP();
    }, nullptr, true);
  }
}
// ------------- SSDP запрос
void requestSSDP () {
  if (MyWiFi.modeSTA()) {
    if (getSetup(shopS) == "0")  ssdpList = "{}";
    jsonWrite(ssdpList, getSetup(ssdpS), MyWiFi.StringIP());
    IPAddress ssdpAdress(239, 255, 255, 250);
    unsigned int ssdpPort = 1900;
    char  ReplyBuffer[] = "M-SEARCH * HTTP/1.1\r\nHost:239.255.255.250:1900\r\nST:upnp:rootdevice\r\nMan:\"ssdp:discover\"\r\nMX:3\r\n\r\n";
    udp.beginPacket(ssdpAdress, ssdpPort);
    //udp.write(ReplyBuffer);
    udp.printf(ReplyBuffer);
    udp.endPacket();
  }
}
// ------------- Чтение ответа от устройств SSDP слушаем порт все время
void handleSSDP() {
  if (MyWiFi.modeSTA()) { //Если есть подключение к роутеру
    String input_string;
    String chipIDremote;
    String ssdpName;
    char packetBuffer[512];
    int packetSize = udp.parsePacket();
    if (packetSize) {
      int len = udp.read(packetBuffer, 512);
      //heapSend();
      if (len > 0) packetBuffer[len] = 0;
      input_string += packetBuffer;
      //Serial.println(input_string);
      int i = input_string.indexOf("Arduino"); // Ищем в полученной строке слово Arduino
      if (i > 0) {
        //Serial.println(input_string);
        chipIDremote = deleteBeforeDelimiter(input_string, "Arduino"); // Откиним все до слова Arduino
        //Serial.println(input_string);
        String IpRemote = deleteBeforeDelimiter(chipIDremote, "LOCATION:");// Найдем строку начинающуюся с LOCATION
        IpRemote = deleteBeforeDelimiter(IpRemote, "//"); // Выделим IP адрес найденого устройства
        IpRemote = selectToMarker(IpRemote, ":");
        chipIDremote = selectToMarker(chipIDremote, "\n"); // Выделим строку до конца
        ssdpName = selectToMarkerLast(chipIDremote, "/");  // Получим имя оно в конце
        ssdpName = selectToMarker(ssdpName, "\r");

        chipIDremote = selectToMarkerLast(chipIDremote, "/");
        chipIDremote = selectToMarker(chipIDremote, "\r");
        // строку input_string сохраняю для расширения
        jsonWrite(ssdpList, chipIDremote, IpRemote);
      }
    }
  }
}
// Каждые 30 секунд проверяем не изиенился ли адрес ip
void ipChanges() {
  String ip = MyWiFi.StringIP();
  if (jsonRead(configSetup, ipS) != ip) {
    sendStatus(ipS, ip);
    jsonWrite(configSetup, ipS, ip);
    jsonWrite(modules, ipS, ip);
  }
}
