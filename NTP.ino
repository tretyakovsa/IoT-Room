//#include <time.h>               //Содержится в пакете
void initNTP() {
  if (MyWiFi.modeSTA()) { //Если есть подключение к роутеру
    String ntpTemp;// = readArgsString();
    if (ntpTemp == emptyS) ntpTemp = "ntp1" + ntpServerS;
    sendOptions(ntpS + "1", ntpTemp);
    ntpTemp = ""; // = readArgsString();
    if (ntpTemp == emptyS) ntpTemp = "ntp2" + ntpServerS;
    sendOptions(ntpS + "2", ntpTemp);
    /*
        if ( getOptions(messageS) == emptyS) { // Если нет связи с интернет пробуем получить время с роутера
          sendOptions(ntpS + "1", WiFi.gatewayIP().toString()); // Для этого заменяем адрес NTP сервера на адрес роутера
        }
    */
    sCmd.addCommand("time", handle_time);
    timeSynch();
    //    Serial.println("NTP START");
    if (GetTime() != "00:00:00") { // Проверка на получение времени
      // задача проверять таймеры каждую секунду обновлять текущее время.
      test1Sec();
      //loadTimer();
      sCmd.addCommand("zone", handle_timeZone);
      modulesReg("ntp");
    }
  }
}
void test1Sec() {
  ts.add(tNTP, 1000, [&](void*) {
    //Serial.print("Time 1 sec =");
    String timeNow = GetTime();
    //Serial.println(timeNow);

    if (timeNow == "00:00:00") { // в это время синхронизируем с внешним сервером
      String timeNow = GetWeekday(); // Новая дата

      sendStatus(weekdayS, timeNow);
      timeSynch();
      loadTimer();
    }
    if (timeNow == getOptions("timersT")) {
      //Serial.println(getOptions("timersC"));
      sCmd.readStr(getOptions("timersC"));
      if (getOptions("timersR") == "1") delTimer();
      sendOptions("timersT", " ");
      sendOptions("timersC", " ");
      loadTimer();
    }
#ifdef webSoketM // #endif
    SoketData (timeS, timeNow, getStatus(timeS));
#endif
    sendStatus(timeS, timeNow);
    sendOptions(timeS, timeNow);
    jsonWrite(configSetup, timeS,  timeNow);
  }, nullptr, true);
}
// ------------------------------ Установка времянной зоны
void handle_timeZone() {
  int timezone = readArgsInt();
  //Serial.println(timezone);
  if (getSetupInt(timeZoneS)!=timezone){
  sendSetup(timeZoneS,  timezone);
  sendOptions(timeZoneS, timezone);
  timeSynch();
  saveConfigSetup ();
  }
}
// ------------------------------ Комманда синхронизации времени
void handle_time() {
  timeSynch();
  statusS = "{}";
  jsonWrite(statusS, "title",   "{{LangTime1}} <strong id=time>" + GetTime() + "</strong>");
}

void timeSynch() {
  uint8_t zone = getSetupInt(timeZoneS);
  //  Serial.print("timeSynch zone=");
  //  Serial.println(zone);
  String ntp1 = getOptions(ntpS + "1");
  String ntp2 = getOptions(ntpS + "2");
  if (ntp1 == emptyS) ntp1 = "ntp1" + ntpServerS;
  if (ntp2 == emptyS) ntp2 = "ru.pool.ntp.org";
  if (MyWiFi.modeSTA()) {
    // Инициализация соединения с NTP сервером
    //    Serial.println(ntp1);
    //    Serial.println(ntp2);
    configTime(zone * 3600, 0, ntp1.c_str(), ntp2.c_str());
    uint8_t i = 0;
    while ((time(nullptr) < NTP_MIN_VALID_EPOCH) && i < 10) {
      i++;
      //      Serial.print(".");
      delay(1000);
    }
    String timeNow = GetTime();
    jsonWrite(configSetup, timeS,  timeNow);
    sendStatus(timeS, timeNow);
    timeNow = GetWeekday();
    sendStatus(weekdayS, timeNow);
  }
}
// Получение текущего времени
String GetTime() {
  time_t now = time(nullptr); // получаем время с помощью библиотеки time.h
  //struct tm * timeinfo;
  //timeinfo = localtime(&now);
  //Serial.println(timeinfo->tm_hour);
  String Time; // Строка для результатов времени

  Time += ctime(&now); // Преобразуем время в строку формата Thu Jan 19 00:55:35 2017
  //Serial.println(Time);
  uint8_t i = Time.indexOf(":"); //Ишем позицию первого символа :
  Time = Time.substring(i - 2, i + 6); // Выделяем из строки 2 символа перед символом : и 6 символов после
  return Time; // Возврашаем полученное время
}
// Получение даты
String GetDate() {
  time_t now = time(nullptr); // получаем время с помощью библиотеки time.h
  String Data; // Строка для результатов времени
  Data += ctime(&now); // Преобразуем время в строку формата Thu Jan 19 00:55:35 2017
  Data.replace("\n", emptyS);
  uint8_t i = Data.lastIndexOf(" "); //Ишем позицию последнего символа пробел
  String Time = Data.substring(i - 8, i + 1); // Выделяем время и пробел
  Data.replace(Time, emptyS); // Удаляем из строки 8 символов времени и пробел
  return Data; // Возврашаем полученную дату
}
// Получение дня недели
String GetWeekday() {
  String Data = GetDate();
  uint8_t i = Data.indexOf(" "); //Ишем позицию первого символа пробел
  String weekday = Data.substring(i - 3, i); // Выделяем время и пробел
  return weekday;
}

uint8_t indexWeekday(String week) {
  const String weeks = "SunMonTueWedThuFriSat";
  return weeks.indexOf(week) / 3;
}
