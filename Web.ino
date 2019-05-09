void initHTTP() {
  SPIFFS.begin();
  initFS();
  HTTP.begin();

  // ------------------- Кэшировать файлы для быстрой работы
  HTTP.serveStatic("/css/", SPIFFS, "/css/", "max-age=31536000"); // кеширование на 1 год
  HTTP.serveStatic("/js/", SPIFFS, "/js/", "max-age=31536000"); // кеширование на 1 год
  HTTP.serveStatic("/img/", SPIFFS, "/img/", "max-age=31536000"); // кеширование на 1 год
  HTTP.serveStatic("/lang/", SPIFFS, "/lang/", "max-age=31536000"); // кеширование на 1 год

  HTTP.on("/wifi.scan.json", HTTP_GET, []() {
    HTTP.send(200, "text/plain", MyWiFi.scan(false));
  });

  // ------------------- Выдаем данные configSetup
  HTTP.on("/config.setup.json", HTTP_GET, []() {
    httpOkJson(configSetup);
  });

  // -------------------Установка конфигурации
  HTTP.on("/configs", HTTP_GET, []() {
    String set = HTTP.arg("set");
    sendSetup(configsS, set);
    saveConfigSetup();
    jsonWrite(modules, configsS, getSetup(configsS));
    String reqvest = "{\"action\": \"page.htm?configs&" + set + "\"}";
    httpOkText(reqvest);
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
    httpOkJson(modules);
  });
  // --------------------Узнать какие модули есть в устройстве
  HTTP.on("/pulse.json", HTTP_GET, []() {
    httpOkJson(pulsList);
  });
  // --------------------Выдаем данные configJson
  HTTP.on("/config.live.json", HTTP_GET, []() {
    httpOkJson(configJson);
  });
  // --------------------Выдаем данные configOptions
  HTTP.on("/config.options.json", HTTP_GET, []() {
    espInfo();
    httpOkJson(configOptions);
  });
  // --------------------Выдаем данные configOptions  config.admin.json
  HTTP.on("/config.admin.json", HTTP_GET, []() {
    espInfo();
    httpOkJson(configOptions);
  });

  // ------------------Выполнение команды из запроса
  HTTP.on("/cmd", HTTP_GET, []() {
    String com = HTTP.arg("command");
    Serial.println(com);
    sendOptions("test", com);
    sCmd.readStr(com);
    httpOkText(statusS);
    //Serial.println(statusS);
  });

  // ------------------Выполнение голосовой команды
  HTTP.on("/voice", HTTP_GET, []() {
    String com = HTTP.arg("command");
    com.replace(" ", "_");
    sendOptions(voiceS, com);
    jsonWrite(modules, voiceS, com);
    flag = sendStatus(voiceS, com);
    httpOkText(statusS);
  });
  sCmd.addCommand(voiceS.c_str(), macros); //
  sendStatus(voiceS, "");
  commandsReg(voiceS);

}

void macros() {
  flag = sendStatus(voiceS, readArgsString());
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
    if (!handleFileRead("/edit.htm")) http404send();//HTTP.send(404, "text/plain", "FileNotFound");
  });
  //Создание файла
  HTTP.on("/edit", HTTP_PUT, handleFileCreate);
  //Удаление файла
  HTTP.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  HTTP.on("/edit", HTTP_POST, []() {
    //HTTP.send(200, "text/plain", emptyS);
    httpOkText(emptyS);
  }, handleFileUpload);
  //called when the url is not defined here
  //use it to load content from SPIFFS
  HTTP.onNotFound([]() {
    if (!handleFileRead(HTTP.uri()))
      http404send();//HTTP.send(404, "text/plain", "FileNotFound");
  });
  HTTP.on("/skins", HTTP_GET, []() {
    String set = HTTP.arg("set");
    //configJson = jsonWrite(configJson, "setIndex", set);
    jsonWrite(configSetup, "setIndex", set);
    saveConfigSetup();
    HTTP.send(307, "Temporary Redirect\r\nLocation: /\r\nConnection: Close\r\n", emptyS);
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
  String setIndex =  jsonRead(configSetup, setIndexS);
  if (setIndex == emptyS) setIndex = "index.htm";
  if (path.endsWith("/")) path += setIndex;
  if (path.endsWith("/admin")) path += setIndex;
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
void http500send(String text) {
  HTTPsendHeader();
  HTTP.send(500, "text/plain", text);
}
void http404send() {
  HTTPsendHeader();
  HTTP.send(404, "text/plain", "FileNotFound");
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
      //Serial.println("web Socket disconnected");
      break;
    case WStype_CONNECTED: // Событие происходит при подключении клиента
      {
        Serial.println("web Socket Connected");
        webSocket.sendTXT(num, configJson); // Отправим в всю строку с данными используя номер клиента он в num
      }
      break;
    case WStype_TEXT: // Событие происходит при получении данных текстового формата из webSocket
      if (length > 0) {
        String command = String((const char *)payload);
        String cmd;
        Serial.print("command=");
        Serial.println(command);
        cmd = jsonRead(command, voiceS); // Прислан макрос
        Serial.print("cmd=");
        Serial.println(cmd);
        if (cmd != "") {
          Serial.print("voiceS=");
          Serial.println(cmd);
          sendOptions(voiceS, cmd);
          flag = sendStatus(voiceS, cmd);
        }
        cmd = jsonRead(command, "effect"); // Прислан эффект
        if (cmd != "") {
          effectTest(cmd);
        }
        cmd = jsonRead(command, "cmd");   // Прислана комманда
        if (cmd != "") {
          sCmd.readStr(cmd);
          //Serial.print("effect=");
          //Serial.println(cmd);
          effectTest(cmd);
        }
      }

      //webSocket.sendTXT(num, "message here"); // Можно отправлять любое сообщение как строку по номеру соединения
      // webSocket.broadcastTXT("message here");
      break;
    case WStype_BIN:      // Событие происходит при получении бинарных данных из webSocket
      // webSocket.sendBIN(num, payload, length);
      break;
  }
}
// Отправка данных в Socket всем получателям
// Параметры Имя ключа, Данные, Предыдущее значение
void SoketData (String key, String data, String data_old)  {
  if (getOptions(webSocketS) != "") {
    if (data_old != data) {
      String broadcast = "{}";
      jsonWrite(broadcast, key, data);
      webSocket.broadcastTXT(broadcast);
      //Serial.println(getOptions(webSocketS));
    }
  }
}

void SocketClient(String ipSocket) {
  WebSocketsClient.begin(ipSocket, 81, "/");
  WebSocketsClient.setReconnectInterval(5000);
  //WebSocketsClient.enableHeartbeat ( 15000 , 3000 , 2 );
}
#endif

void effectTest(String effect) {
  //sCmd.readStr("rgb off 0");
  //sCmd.readStr("rgb off 1");
  String urls = "http://evoflame.co.uk/effect/" + effect;
  effect =  MyWiFi.getURL(urls); // Получить настройки эффекта с сервера
  effect = "rgb set 0 " + effect; // Добавим комманду к первой ленте
  effect.replace("\r\n", "\r\nrgb set 1 "); // Добавим комманду ко второй ленте
  sCmd.readStr(selectToMarker (effect, "\r\n"));
  sCmd.readStr(selectToMarkerLast (effect, "\r\n"));
  sCmd.readStr("pulse on rgb0 30000");
  sCmd.readStr("pulse on rgb1 30000");

}
