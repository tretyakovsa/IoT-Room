void initUpgrade() {
  // Добавляем функцию Update для перезаписи прошивки по Wi-Fi при 1М(256K SPIFFS) и выше
  httpUpdater.setup(&HTTP);
  HTTP.on("/upgrade", webUpgrade);                // запустить обновление
  modulesReg("upgrade");
}
// ----------------------- Обновление с сайта
void webUpgrade() {
 // String refresh = "<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"./css/build.css\"></head><body><br><br><center><div class=\"loader\"></div><h1>Update module... <br><span id=\"countdownt\">90 </span> seconds...</h1></center>   <script>var timeleft=90;var downloadTimer=setInterval(function(){timeleft--;document.getElementById(\"countdownt\").textContent=timeleft;if(timeleft <= 0){clearInterval(downloadTimer);window.location.href=\"/\"}},1000);</script></body></html>";
  String refresh = "<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"./css/display.css\"></head><body><br><br><center><div class=\"loader\"></div><h1>Updating Module... <br><span id=\"countdownt\">90 </span> seconds...</h1></center>   <script>var timeleft=90;var downloadTimer=setInterval(function(){timeleft--;document.getElementById(\"countdownt\").textContent=timeleft;if(timeleft <= 0){clearInterval(downloadTimer);window.location.href=\"/\"}},1000);</script></body></html>";
  httpOkHtml(refresh); // Сразу отправить ответ редиректа
  String spiffsData = HTTP.arg(spiffsS); // Получим путь к файловой системе
  String buildData = HTTP.arg(buildS);  // Получим путь к файлу прошивки
  if (spiffsData != emptyS) { // Если нужно прошить FS
    //Serial.println(spiffsData);
    String Timerset = readFile(configTimerS, 4096); // Сохраним все таймеры в память
    // Сценарии уже хранятся в памяти их загружать не нужно
    // ?? Попробовать сделать чтение сценарием из файла
    sendSetup(spiffsDataS, spiffsData.substring(spiffsData.lastIndexOf("/") + 1)); // Записать имя файла для сохраненной версии
    ESPhttpUpdate.rebootOnUpdate(false); // Отключим перезагрузку после обновления
    // Обновим FS
    updateHTTP(spiffsData, true);
    writeFile(configTimerS, Timerset); // Востановим файл таймеров
    if (Scenary != emptyS) { // Если есть сценарии d Памяти
      writeFile(ScenaryS, Scenary); // Сохранить сценарии
    }
    saveConfigSetup (); // Сохранить в файле Setup
  }
  if (buildData != emptyS) { // Если нужно прошить build
   // Serial.println(buildData);
    if (Uploader != emptyS) { // Если включен Uploader
      writeFile("buildPach.txt", buildData); // Сохраним путь к файлу в файл которым воспользуется Uploader
      buildData = Uploader; // Изменим url для работы Uploader
    }
    sendSetup(buildDataS, buildData.substring(buildData.lastIndexOf("/") + 1)); // Записать имя файла для сохраненной версии
    saveConfigSetup ();
    ESPhttpUpdate.rebootOnUpdate(true); // Отключим перезагрузку после обновления ??
    // Обновим build
    updateHTTP(buildData, false);
  }
}
// ------------------ Обновление по url
// url файла для обновления. mode признак типа данных FS - true или Code - false
void updateHTTP(String url, boolean mode) {
  if (url == "") return;
  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
  if (mode) {
//    Serial.println("Update Spiffs...");
    t_httpUpdate_return ret = ESPhttpUpdate.updateSpiffs(url);
    UpdateStatus(ret , "Spiffs");
  } else {
//    Serial.println("Update Build...");
    t_httpUpdate_return ret = ESPhttpUpdate.update(url);
    UpdateStatus(ret , "build");
  }
}
void UpdateStatus(t_httpUpdate_return set , String mode) {
  switch (set) {
    case HTTP_UPDATE_FAILED:
//    Serial.println(mode + "_FAILED");
      sendSetup(spiffsDataS, mode + "_FAILED");
      statistics();
      break;
    case HTTP_UPDATE_NO_UPDATES:
//    Serial.println(mode + "_NO_UPDATES");
      sendSetup(spiffsDataS, mode + "_NO_UPDATES");
      statistics();
      break;
    case HTTP_UPDATE_OK:
//    Serial.println(mode + "_UPDATE_OK");
      sendSetup(spiffsDataS, mode + "_UPDATE_OK");
      statistics();
      break;
  }
}
