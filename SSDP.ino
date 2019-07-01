// ------------- SSDP запрос
void requestSSDP () {
  if (MyWiFi.modeSTA()) {
    ssdpList = "{}";
    jsonWrite(ssdpList, getSetup(ssdpS), MyWiFi.StringIP());
    IPAddress ssdpAdress(239, 255, 255, 250);
    unsigned int ssdpPort = 1900;
    char  ReplyBuffer[] = "M-SEARCH * HTTP/1.1\r\nHost:239.255.255.250:1900\r\nST:upnp:rootdevice\r\nMan:\"ssdp:discover\"\r\nMX:3\r\n\r\n";
    udp.beginPacket(ssdpAdress, ssdpPort);
    udp.write(ReplyBuffer);
    udp.endPacket();
  }
}
// ------------- Чтение ответа от устройств SSDP слушаем порт все время
void handleSSDP() {
  if (MyWiFi.modeSTA()) { //Если есть подключение к роутеру
    String input_string;
    String chipIDremote;
    String ssdpName;
    char packetBuffer[UDP_PACKET_MAX_SIZE];
    int packetSize = udp.parsePacket();
    if (packetSize) {
      int len = udp.read(packetBuffer, UDP_PACKET_MAX_SIZE);
      if (len > 0) packetBuffer[len] = 0;
      input_string += packetBuffer;
      //Serial.println(input_string);
      int i = input_string.indexOf("Arduino"); // Ищем в полученной строке слово Arduino
      if (i > 0) {

        chipIDremote = deleteBeforeDelimiter(input_string, "Arduino"); // Откиним все до слова Arduino
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
void initUpnp() {
  sendSetup(ssdpS, defaultTestStringMAC(getSetup(ssdpS), ssdpDef));
  sendSetup(spaceS, defaultTestString(getSetup(spaceS), spaceDef));
  setupToOptions(ssdpS);
  setupToOptions(spaceS);
  jsonWrite(modules, ssdpS, getSetup(ssdpS));
  //void initSSDP() {
  if (MyWiFi.modeSTA()) { //Если есть подключение к роутеру
    unsigned int localPort = 1901;
    udp.begin(localPort);
    unsigned int ssdpPort = 1900;
    IPAddress ssdpAdress(239, 255, 255, 250);
    udpUpnp.beginMulticast(WiFi.localIP(), ssdpAdress, ssdpPort);
    HTTP.on("/description.xml", HTTP_GET, []() {
      String ssdpSend = "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">";
      String  ssdpHeder = xmlNode("major", "1");
      ssdpHeder += xmlNode("minor", "0");
      ssdpHeder = xmlNode("specVersion", ssdpHeder);
      ssdpHeder += xmlNode("URLBase", "http://" + WiFi.localIP().toString());
      String  ssdpDescription = xmlNode("deviceType", "upnp:rootdevice");
      ssdpDescription += xmlNode("friendlyName", jsonRead(configSetup, "SSDP"));
      ssdpDescription += xmlNode("presentationURL", "/");
      ssdpDescription += xmlNode("serialNumber", chipID);
      ssdpDescription += xmlNode("modelName", jsonRead(configSetup, "configs"));
      ssdpDescription += xmlNode("modelNumber", chipID + "/" + jsonRead(configSetup, "SSDP"));
      ssdpDescription += xmlNode("modelURL", "https://github.com/tretyakovsa/Sonoff_WiFi_switch");
      ssdpDescription += xmlNode("manufacturer", "Tretyakov Sergey, Kevrels Renats");
      ssdpDescription += xmlNode("manufacturerURL", "http://www.esp8266-arduinoide.ru");
      ssdpDescription += xmlNode("UDN", "uuid:38323636-4558-4dda-9188-cda0e6" + String( ESP.getChipId(), HEX ));
      ssdpDescription = xmlNode("device", ssdpDescription);
      ssdpHeder += ssdpDescription;
      ssdpSend += ssdpHeder;
      ssdpSend += "</root>";
      httpOkXML(ssdpSend);
    });
    HTTP.on("/setup.xml", HTTP_GET, []() {
      String setup_xml = "<root>"
                     "<device>"
                     "<deviceType>urn:Belkin:device:controllee:1</deviceType>"
                     "<friendlyName>" + getOptions(ssdpS) + "</friendlyName>"
                     "<manufacturer>Belkin International Inc.</manufacturer>"
                     "<modelName>Socket</modelName>"
                     "<modelNumber>3.1415</modelNumber>"
                     "<modelDescription>Belkin Plugin Socket 1.0</modelDescription>\r\n"
                     "<UDN>uuid:Socket-1_0-38323636-4558-4dda-9188-cda0e6" + String( ESP.getChipId(), HEX ) + "-80</UDN>"
                     "<serialNumber>221517K0101769</serialNumber>"
                     "<binaryState>0</binaryState>"
                     "<serviceList>"
                     "<service>"
                     "<serviceType>urn:Belkin:service:basicevent:1</serviceType>"
                     "<serviceId>urn:Belkin:serviceId:basicevent1</serviceId>"
                     "<controlURL>/upnp/control/basicevent1</controlURL>"
                     "<eventSubURL>/upnp/event/basicevent1</eventSubURL>"
                     "<SCPDURL>/eventservice.xml</SCPDURL>"
                     "</service>"
                     "</serviceList>"
                     "</device>"
                     "</root>\r\n"
                     "\r\n";
                     httpOkXML(setup_xml);
      });
HTTP.on("/eventservice.xml", HTTP_GET, []() {
      String eventservice_xml = "<scpd xmlns=\"urn:Belkin:service-1-0\">"
                            "<actionList>"
                            "<action>"
                            "<name>SetBinaryState</name>"
                            "<argumentList>"
                            "<argument>"
                            "<retval/>"
                            "<name>BinaryState</name>"
                            "<relatedStateVariable>BinaryState</relatedStateVariable>"
                            "<direction>in</direction>"
                            "</argument>"
                            "</argumentList>"
                            "</action>"
                            "<action>"
                            "<name>GetBinaryState</name>"
                            "<argumentList>"
                            "<argument>"
                            "<retval/>"
                            "<name>BinaryState</name>"
                            "<relatedStateVariable>BinaryState</relatedStateVariable>"
                            "<direction>out</direction>"
                            "</argument>"
                            "</argumentList>"
                            "</action>"
                            "</actionList>"
                            "<serviceStateTable>"
                            "<stateVariable sendEvents=\"yes\">"
                            "<name>BinaryState</name>"
                            "<dataType>Boolean</dataType>"
                            "<defaultValue>0</defaultValue>"
                            "</stateVariable>"
                            "<stateVariable sendEvents=\"yes\">"
                            "<name>level</name>"
                            "<dataType>string</dataType>"
                            "<defaultValue>0</defaultValue>"
                            "</stateVariable>"
                            "</serviceStateTable>"
                            "</scpd>\r\n"
                            "\r\n";

                     httpOkXML(eventservice_xml);
      });

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
void handleUpnp() {
  if (MyWiFi.modeSTA()) { //Если есть подключение к роутеру
    String input_string;
    String chipIDremote;
    String ssdpName;
    char packetBuffer[UDP_PACKET_MAX_SIZE];
    int packetSize = udpUpnp.parsePacket();
    IPAddress senderIP = udpUpnp.remoteIP();
    unsigned int senderPort = udpUpnp.remotePort();
    if (packetSize) {
      int len = udpUpnp.read(packetBuffer, UDP_PACKET_MAX_SIZE);
      if (len > 0) packetBuffer[len] = 0;
      input_string += packetBuffer;
      if (input_string.indexOf("M-SEARCH") >= 0) {
        //Serial.println(input_string);
        respondToSearch(senderIP, senderPort);
      }

    }
  }
}

void respondToSearch(IPAddress& senderIP, unsigned int senderPort) {
  String response =
    "HTTP/1.1 200 OK\r\n"
    "EXT:\r\n"
    "CACHE-CONTROL: max-age=1200\r\n"
    "ST: upnp:rootdevice\r\n"
    "SERVER: Arduino/1.0 UPNP/1.1 manually/" + chipID + "/" + getSetup(ssdpS) + "\r\n"
    "USN: uuid:38323636-4558-4dda-9188-cda0e6" + String( ESP.getChipId(), HEX ) + "\r\n"
    "LOCATION: http://" + getOptions(ipS) + ":80/description.xml\r\n"
    "\r\n";
    String response1 =
    "HTTP/1.1 200 OK\r\n"
    "CACHE-CONTROL: max-age=86400\r\n"
    "EXT:\r\n"
    "LOCATION: http://" + getOptions(ipS) + ":80/setup.xml\r\n"
    "OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"
    "01-NLS: b9200ebb-736d-4b93-bf03-835149d13983\r\n"
    "SERVER: Unspecified, UPnP/1.0, Unspecified\r\n"
    "ST: urn:Belkin:device:**\r\n"
    "USN: uuid:Socket-1_0-38323636-4558-4dda-9188-cda0e6" + String( ESP.getChipId(), HEX ) + "-80::urn:"+getOptions(ssdpS)+":device:**\r\n"
    "X-User-Agent: redsonic\r\n\r\n";
  udpUpnp.beginPacket(senderIP, senderPort);
  udpUpnp.write(response.c_str());
  udpUpnp.endPacket();
  udpUpnp.beginPacket(senderIP, senderPort);
  udpUpnp.write(response1.c_str());
  udpUpnp.endPacket();
  yield();
}

String xmlNode(String tags, String data) {
  String temp = "<" + tags + ">" + data + "</" + tags + ">";
  return temp;
}
