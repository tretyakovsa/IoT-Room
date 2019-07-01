void initScenary() {
  sCmd.addCommand("if",  ifCommand);
  sCmd.addCommand("or",  orCommand);
  sCmd.addCommand("and",  andCommand);
  sCmd.addCommand("id",  idNot);
  sCmd.addCommand("then", thenCommand);
  HTTP.on("/setscenary", HTTP_GET, []() {
    loadScenary();
    //alarmLoadModules();
    loadTimer();
    httpOkText();
  });
  loadScenary();
}
void loadScenary() {
  //String scen = ScenaryS + getSetup(configsS) + ".txt";
  String scen = ScenaryS;
  Scenary = readFile(scen, 4096);
  Scenary.replace("\r\n", "\n");
  Scenary.replace("\n\n", "\n");
  Scenary += "\n";
}
// Ничего не делать если комманда id
void idNot() {}

void handleScenary() {
  //  Serial.println(flag);
  if (flag) { // если произошло изменение в данных config.live.json
    goCommands(Scenary); // Делаем разбор сценариев
#ifdef webSoketM // #endif
    webSocket.broadcastTXT(configJson);
#endif
    sendStatus("voice", emptyS);
    flag = false;
  }
}
// Разбор команды if
void ifCommand() {
  thenOk = false; // сброс признака
  orCommand();
}
void andCommand() {
  String Name =  readArgsString();      // Какой параметр проверяем
  String Condition =  readArgsString(); // Операция
  String Volume =  readArgsString();    // Значение параметра
  String test = getStatus(Name);        // получить текущее значение параметра

  if (thenOk) {
    thenOk = false; // сброс признака
    testCommand(Volume, Condition, test);
  }
}

void orCommand() {
  String Name =  readArgsString();      // Какой параметр проверяем
  String Condition =  readArgsString(); // Операция
  String Volume =  readArgsString();    // Значение параметра
  String test = getStatus(Name);        // получить текущее значение параметра
  testCommand(Volume, Condition, test);
}

void testCommand(String Volume, String Condition, String test) {
  // последовательно проверяем параметр на соответствие операции сравнения
  // и поднимаем признак исполнения then
  if (Condition == "=") {
    if (Volume == test) thenOk = true;
  }
  if (Condition == ">") {
    if (Volume.toFloat() < test.toFloat()) thenOk = true;
  }
  if (Condition == "<") {
    if (Volume.toFloat() > test.toFloat()) thenOk = true;
  }
  if (Condition == "<=") {
    if (Volume.toFloat() >= test.toFloat()) thenOk = true;
  }
  if (Condition == ">=") {
    if (Volume.toFloat() <= test.toFloat()) thenOk = true;
  }
  if (Condition == "!=") {
    if (Volume != test) thenOk = true;
  }
}


// Выполнение then
void thenCommand() {
  if (thenOk) {
    String ssdp = jsonRead(configSetup, ssdpS);
    String test = readArgsString();
    String comm = readArgsString();
    comm += " " + readArgsString();
    comm += " " + readArgsString();
    comm += " " + readArgsString();
    comm += " " + readArgsString();
    comm += " " + readArgsString();
    comm += " " + readArgsString();
      //Serial.println("test= ");
      //Serial.println(test);
    // Если это локальное устройство
    if (ssdp == test or test == "this") {
      //Serial.print("comm= ");
      //Serial.println(comm);
      sendOptions("test", comm);
      sCmd.readStr(comm);
    }
    else {

      //http://192.168.0.91/cmd?command=relay on 1
      String urls = "http://";
      //Serial.println("test1= ");
      //Serial.println(test);
      String ip = jsonRead(ssdpList, test);
      //Serial.println("ip= ");
      //Serial.println(ip);
      urls += ip;
      urls += "/cmd?command=" + comm;
      urls.replace(" ", "%20");
     // Serial.println("urls= ");
     // Serial.println(urls);
      if (ip != emptyS) {
        sendOptions("test", urls);
        Serial.println(MyWiFi.getURL(urls));
      }
    }
  }
}
