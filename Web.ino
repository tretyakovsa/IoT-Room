// Включить модуль ADMIN
void initAdmin() {
  modulesReg("admin");
}
void sendSetupArg(String argS) {
  if (HTTP.hasArg(argS)) {
    sendSetup(argS, HTTP.arg(argS));
  }
}

void sendSetupArg(String argS, String key) {
  if (HTTP.hasArg(argS)) {
    sendSetup(key, HTTP.arg(argS));
  }
}

void initHTTP() {
  SPIFFS.begin();
  initFS();
  HTTP.begin();

  // ------------------- Кэшировать файлы для быстрой работы
  HTTP.serveStatic("/css/", SPIFFS, "/css/", "max-age=31536000"); // кеширование на 1 год
  HTTP.serveStatic("/js/", SPIFFS, "/js/", "max-age=31536000"); // кеширование на 1 год
  HTTP.serveStatic("/img/", SPIFFS, "/img/", "max-age=31536000"); // кеширование на 1 год
  HTTP.serveStatic("/lang/", SPIFFS, "/lang/", "max-age=31536000"); // кеширование на 1 год

  /*  // -------------------Сброс к заводским настройкам
    HTTP.on("/configs", HTTP_GET, []() {
      if (HTTP.arg("restore") == "ok") {
      } else {
        sendSetupArg("set", configsS);
        //sendSetupArg(configsS);

        saveConfigSetup();
        httpOkJson(configSetup);
      }
    });
  */
  // -------------------Установка конфигурации
  HTTP.on("/configs", HTTP_GET, []() {
    //sendSetupArg("set");
    String set = HTTP.arg("set");
    sendSetup(configsS, set);
    saveConfigSetup();
    String reqvest = "{\"action\": \"page.htm?configs&" + getSetup(configsS) + "\"}";
    httpOkText(reqvest);
  });

  HTTP.on("/wifi.scan.json", HTTP_GET, []() {
    httpOkJson(MyWiFi.scan(false));
    //HTTP.send(200, "text/plain", MyWiFi.scan(false));
  });

  // ------------------- Выдаем данные configSetup
  HTTP.on("/config.setup.json", HTTP_GET, []() {
    //configSetup.replace(",", ",\r\n");
    httpOkJson(configSetup);
  });


  // --------------------Выдаем данные ssdpList
  HTTP.on("/ssdp.list.json", HTTP_GET, []() {
    httpOkJson(ssdpList);
  });
  // -------------------Выдаем данные regCommands
  HTTP.on("/command.json", HTTP_GET, []() {
    httpOkJson(regCommands);
  });
  // --------------------Узнать какие модули есть в устройстве
  HTTP.on("/modules.json", HTTP_GET, []() {
    //Serial.println(configJson);
    httpOkJson(modules);
  });
  // --------------------Выдаем данные configJson
  HTTP.on("/config.live.json", HTTP_GET, []() {
    //configJson.replace(",", ",\r\n");
    httpOkJson(configJson);
  });
  // --------------------Выдаем данные configOptions
  HTTP.on("/config.options.json", HTTP_GET, []() {
    httpOkJson(configOptions);
  });
  // --------------------Выдаем данные configOptions  config.admin.json
  HTTP.on("/config.admin.json", HTTP_GET, []() {
    espInfo();
    //httpOkJson(configOptions);
  });

  // ------------------Выполнение команды из запроса
  HTTP.on("/cmd", HTTP_GET, []() {
    String com = HTTP.arg("command");
    sendOptions("test", com);
    sCmd.readStr(com);
    httpOkText(statusS);
  });
  // ------------------Выполнение голосовой команды макроса
  HTTP.on("/voice", HTTP_GET, []() {
    String com = HTTP.arg("command");
    com.replace(" ", "_");
    sendOptions(voiceS, com);
    flag = sendStatus(voiceS, com);
    httpOkText(statusS);
  });

  sCmd.addCommand(voiceS.c_str(), macros); //
  sendOptions(voiceS, "");
  sendStatus(voiceS, "");
  commandsReg(voiceS);
  
  // Установить имя устройства
  HTTP.on("/device", HTTP_GET, []() {
    String  ssdpName = HTTP.arg("ssdp");
    sendSetup(ssdpS, ssdpName);
    sendOptions(ssdpS, ssdpName);
    SSDP.setName(ssdpName);
    SSDP.setModelNumber(chipID + "/" + getSetup(ssdpS));
    String  space = HTTP.arg("space");
    sendSetup(spaceS, space);
    sendOptions(spaceS, space);
    jsonWrite(modules, ssdpS, getSetup(ssdpS));
    jsonWrite(modules, spaceS, space);
    httpOkText();
    saveConfigSetup();
    requestSSDP();
  });
  // -------------------построение графика
  HTTP.on("/charts.json", HTTP_GET, []() {
    String pFile = HTTP.arg("data");
    String message = "{";
    if (pFile.indexOf(".") == -1) {
      for (uint8_t i = 0; i < HTTP.args(); i++) {
        //message += " " + HTTP.argName(i) + ": " + HTTP.arg(i) + "\n";
        message += "\"" + HTTP.argName(i) + "\":[";
        String key = getOptions(HTTP.arg(i));
        if (key != emptyS)  {
          message += key;
          key = emptyS;
        } else {
          key = getStatus(HTTP.arg(i));
          if (key != emptyS)  {
            message += key;
            key = emptyS;
          }
        }
        message += "],";
      }
    }
    message += "\"points\":\"10\",\"refresh\":\"1000\"}";

    httpOkText(message);
  });
}
void macros() {
  String tem = readArgsString();
  flag = sendStatus(voiceS, tem);
}
// Инициализация FFS
void initFS() {
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
    }
    // Создаем список файлов каталога /lang
    Lang = FileList("/lang");
  }
  //HTTP страницы для работы с FFS
  //list directory
  HTTP.on("/list", HTTP_GET, handleFileList);
  //загрузка редактора editor
  HTTP.on("/edit", HTTP_GET, []() {
    String editTest = jsonRead(modules, "module");
    if (editTest.indexOf("admin") != -1) { //Serial.println("Откроем доступ");
      if (!handleFileRead("/edit.htm")) http404send();
    } else if (!handleFileRead(htm403S)) http403send();
  });
  HTTP.on("/admin", HTTP_GET, []() {
    String editTest = jsonRead(modules, "module");
    if (editTest.indexOf("admin") != -1) { //Serial.println("Откроем доступ");
      if (!handleFileRead("/admin/index.htm")) http404send();
    } else if (!handleFileRead(htm403S)) http403send();
  });
  //Создание файла
  HTTP.on("/edit", HTTP_PUT, handleFileCreate);
  //Удаление файла
  HTTP.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  HTTP.on("/edit", HTTP_POST, []() {
    httpOkText(emptyS);
  }, handleFileUpload);
  HTTP.onNotFound([]() {
    if (!handleFileRead(HTTP.uri()))
      if (MyWiFi.status() == 3) {
        if (!handleFileRead(htm404S)) http404send();
      } else http404send();

  });
  HTTP.on("/skins", HTTP_GET, []() {
    String set = HTTP.arg("set");
    //configJson = jsonWrite(configJson, "setIndex", set);
    sendSetup("setIndex", set);
    saveConfigSetup();
    HTTPsendHeader();
    HTTP.send(307, "Temporary Redirect\r\nLocation: /\r\nConnection: Close\r\n", emptyS);
  });
  HTTP.on("/lang", HTTP_GET, []() {
    sendSetup(langS, HTTP.arg("set"));
    setupToOptions(langS);
    setupToStatus(langS);
    jsonWrite(modules, langS, getSetup(langS));
    saveConfigSetup();
    httpOkText();
  });
}

// Здесь функции для работы с файловой системой
String getContentType(String filename) {
  if (HTTP.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return texthtmlS;
  else if (filename.endsWith(".html")) return texthtmlS;
  else if (filename.endsWith(".json")) return "application/json";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) {
  //Serial.print("Вход на страницу =");
  //Serial.println(path);
  String editTest = jsonRead(modules, "module");
  if (editTest.indexOf("admin") == -1) {
    //Serial.println("Закрыть страницы admin");
    if (path.indexOf("admin") != -1 || path.indexOf(htmEditS) != -1) {
      path = htm403S;
      //Serial.println("Содержит страницы admin");
    }
  }

  String setIndex =  jsonRead(configSetup, setIndexS);
  if (setIndex == emptyS) setIndex = "index.htm";
  if (path.endsWith("/")) path += setIndex;
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    HTTP.sendHeader("Access-Control-Allow-Origin", "*");
    size_t sent = HTTP.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload() {
  if (HTTP.uri() != "/edit") return;
  HTTPUpload& upload = HTTP.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile)
      fsUploadFile.close();
  }
}

void handleFileDelete() {
  if (HTTP.args() == 0) return http500send("BAD ARGS");// HTTP.send(500, "text/plain", "BAD ARGS");
  String path = HTTP.arg(0);
  if (path == "/")
    return http500send("BAD PATH");// HTTP.send(500, "text/plain", "BAD PATH");
  if (!SPIFFS.exists(path))
    return http404send();//HTTP.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  //HTTP.send(200, "text/plain", emptyS);
  httpOkText(emptyS);
  path = String();
}

void handleFileCreate() {
  if (HTTP.args() == 0)
    return http500send("BAD ARGS");//  HTTP.send(500, "text/plain", "BAD ARGS");
  String path = HTTP.arg(0);
  if (path == "/")
    return http500send("BAD PATH");//  HTTP.send(500, "text/plain", "BAD PATH");
  if (SPIFFS.exists(path))
    return http500send("FILE EXISTS");//  HTTP.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if (file)
    file.close();
  else
    return http500send("CREATE FAILED");// HTTP.send(500, "text/plain", "CREATE FAILED");
  //HTTP.send(200, "text/plain", emptyS);
  httpOkText(emptyS);
  path = String();

}

void handleFileList() {
  if (!HTTP.hasArg("dir")) {
    //HTTP.send(500, "text/plain", "BAD ARGS");
    http500send("BAD ARGS");//
    return;
  }
  String path = HTTP.arg("dir");
  //HTTP.send(200, "application/json", FileList(path));
  httpOkJson(FileList(path));
}

// Создаем список файлов каталога
String FileList(String path) {
  Dir dir = SPIFFS.openDir(path);
  path = String();
  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }
  output += "]";
  return output;
}

void httpOkText() {
  HTTPsendHeader();
  HTTP.send(200, "text/plain", "Ok");
}
void httpOkText(String text) {
  HTTPsendHeader();
  HTTP.send(200, "text/plain", text);
}
void httpOkHtml(String text) {
  HTTPsendHeader();
  HTTP.send(200, "text/html", text);
}
void httpOkJson(String text) {
  HTTPsendHeader();
  HTTP.send(200, "application/json", text);
}
void httpOkXML(String text) {
  HTTPsendHeader();
  HTTP.send(200, "text/xml", text);
}
void http500send(String text) {
  HTTPsendHeader();
  HTTP.send(500, "text/plain", text);
}
void http404send() {
  HTTPsendHeader();
  HTTP.send(404, "text/plain", "FileNotFound");
}
void http403send() {
  HTTPsendHeader();
  HTTP.send(403, "text/plain", "You don't have permission to access");
}

void HTTPsendHeader() {
  HTTP.sendHeader("Access-Control-Allow-Origin", "*");
}

#ifdef webSoketM // #endif

void webSoket_init() {
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  sendOptions(webSocketS, 1);
}
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:  // Событие происходит при отключени клиента
      break;
    case WStype_CONNECTED: // Событие происходит при подключении клиента
      {
        webSocket.sendTXT(num, configJson); // Отправим в всю строку с данными используя номер клиента он в num
      }
      break;
    case WStype_TEXT: // Событие происходит при получении данных текстового формата из webSocket
      if (length > 0) {
        String command = String((const char *)payload);
        //remoutget(command);
      }
      break;
    case WStype_BIN:      // Событие происходит при получении бинарных данных из webSocket
      // webSocket.sendBIN(num, payload, length);
      break;
  }
}
// Отправка данных в Socket всем получателям
// Параметры Имя ключа, Данные, Предыдущее значение
void SoketData (String key, String data, String data_old)  {
  if (webSocket.connectedClients(false) > 0)  {
    if (data_old != data) {
      String broadcast = "{}";
      jsonWrite(broadcast, key, data);
      webSocket.broadcastTXT(broadcast);
    }
  }
}
#endif
