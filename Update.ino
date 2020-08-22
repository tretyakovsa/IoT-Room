void initUpgrade() {
  // Добавляем функцию Update для перезаписи прошивки по Wi-Fi при 1М(256K SPIFFS) и выше
  httpUpdater.setup(&HTTP);
  HTTP.on("/upgrade", webUpgrade);                // запустить обновление
  modulesReg("upgrade");
}
// ----------------------- Обновление с сайта
void webUpgrade() {
  if (inetTest()) {
    httpOkHtml("Ok"); // Сразу отправить ответ редиректа
    String spiffsData = HTTP.arg(spiffsS); // Получим путь к файловой системе
    String buildData = HTTP.arg(buildS);  // Получим путь к файлу прошивки
    if (buildData.indexOf("beta") != -1) {
      sendSetup(relizS, "beta");
    } else if (buildData == "evonicfires.ino.generic.bin") {
      sendSetup(relizS, "alpha");
    } else sendSetup(relizS, "");
    if ((spiffsData != emptyS) || (buildData != emptyS)) {
      sendSetup(flagUpdateS, "1");
      saveConfigSetup();
      //client.close();
      ESP.restart();
    }
  }
}



void webUpdate() {
  getVershion();
  String spiffs = getOptions(spiffsS);
  String build = getOptions(buildS);
  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
  ESPhttpUpdate.onStart(update_started);
  ESPhttpUpdate.onEnd(update_finished);
  ESPhttpUpdate.onProgress(update_progress);
  ESPhttpUpdate.onError(update_error);
  if (getSetup("spiffsData") != spiffs || getSetup(relizS) == "alpha") {
    String Timerset = readFile(configTimerS, 4096); // Сохраним все таймеры в глобальной переменной
    typeUpdate = "spiffs";
    //SPIFFS.format();
    SPIFFS.end();
    //Serial.println(patch + spiffs);
    updateHTTP(patch + spiffs, true); // Обновим файловую систему
    SPIFFS.begin();
    SPIFFS.remove(fileConfigS); // удалим файл конфигурации
    saveConfigSetup();
    SPIFFS.remove(configTimerS); // удалим файл таймеров
    writeFile(configTimerS, Timerset );
  }
  //if (getSetup("buildData") != build || getSetup(relizS) =="alpha") {
  typeUpdate = "build";

  Serial.println(patch + build);
  //updateFirmware();
  updateHTTP(patch + build, false); //Обновим булд
  SPIFFS.remove(fileConfigS); // удалим файл конфигурации
  sendSetup(flagUpdateS, "0");
  saveConfigSetup();

  ESP.restart();
}

void getVershion() {
  const String MODEL = ""; //halo, ilusion, remotec
  String RELIZ = getSetup(relizS); //пусто стабильный релиз /alpha, /beta
  if (RELIZ != "")  RELIZ = "/" + RELIZ;
  const String http = "http://";
  const String server = "backup.privet.lv/iotroom/";
  const String urlsUpdateData = "production.json/";
  String adress = http + server + urlsUpdateData + MODEL + RELIZ;
  Serial.println(adress);
  adress = MyWiFi.getURL(adress);
  Serial.println(adress);
  adress = deleteBeforeDelimiterTo(adress, "{");
  adress = deleteToMarkerLast(adress, "}");
  String build = jsonRead(adress, "build");
  sendOptions(buildS, build);
  String spiffs = jsonRead(adress, "spiffs");
  sendOptions(spiffsS, spiffs);
  patch = "http://" + jsonRead(adress, "url") + "/";
  patch.replace("\\", "");
  Serial.println(patch);
}
// ------------------ Обновление по url
// url файла для обновления. mode признак типа данных FS - true или Code - false
void updateHTTP(String url, boolean mode) {
  if (url == "") return;
  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
  ESPhttpUpdate.rebootOnUpdate(false); // Отключим перезагрузку после обновления
  url="";
  if (mode) {
    t_httpUpdate_return ret = ESPhttpUpdate.updateSpiffs(url);
    UpdateStatus(ret , "Spiffs");
  } else {
    t_httpUpdate_return ret = ESPhttpUpdate.update(url);
    UpdateStatus(ret , "build");
  }
}

void UpdateStatus(t_httpUpdate_return & set , String mode) {
  String spiffs = getOptions(spiffsS);
  String build = getOptions(buildS);
  switch (set) {
    case HTTP_UPDATE_FAILED:
      sendOptions(onPowerS, mode + "_FAILED");
      statistics(statUpdate);
      break;
    case HTTP_UPDATE_NO_UPDATES:
      sendOptions(onPowerS, mode + "_NO_UPDATES");
      statistics(statUpdate);
      break;
    case HTTP_UPDATE_OK:
      sendOptions(onPowerS, mode + "_UPDATE_OK");
      if (mode == "Spiffs") sendSetup(spiffsDataS, spiffs); // Записать имя файла для сохраненной версии FS в конфиг
      if (mode == "build") sendSetup(buildDataS, build); // Записать имя файла для сохраненной версии FS в конфиг
      statistics(statUpdate);
      break;
  }
}

void update_started() {
  //Serial.println("CALLBACK:  HTTP update process started");
}

void update_finished() {
  //Serial.println("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total) {
  //Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err) {
  //Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}
