#ifdef TimersM
void initTimers() {
  if (MyWiFi.modeSTA()) {
    loadTimer();
    // --------------------Установка таймеров с веб страницы
    HTTP.on("/settimer", HTTP_GET, []() {
      loadTimer();
    });
    modulesReg("timers");
    sCmd.addCommand("timeTest", timeTest);
  }
}

// {"timer":[
// {"id":6422,"day":"1111111","time1":"20:59:58","com1":"relay not 1","run1":"0","active":1},
// {"id":3912,"day":"1111111","time1":"21:00:58","com1":"relay not 1","run1":"0","active":1},
// {"id":8167,"day":"1111111","time1":"21:01:58","com1":"relay not 1","run1":"1","active":1}
// ]}
void loadTimer() {
  long lminTime = 86400;
  String jsonTimer = readFile(configTimerS, 4096);
  if (jsonTimer == "Failed") {
    jsonTimer = "{\"timer\":[]}";
    writeFile(configTimerS, jsonTimer); // Востановим файл
  }
  String Weekday = GetWeekday();
  uint8_t w = indexWeekday(Weekday);
  uint8_t i = 0;
  String tim;
  sendOptions(timersTS, "");
  do {
    tim = getArray(jsonTimer, "timer", i); // получим строку таймера
    if (jsonReadToInt(tim, "active")==1){// Если таймер активен
    String week = jsonRead(tim, "day"); // Проверим активность в текущий день недели
    if (week.substring(w, w + 1) == "1") { // Если таймер активен в текущий день недели
      String nextTime1 = jsonRead(tim, "time1"); // Получим время таймера
      if (timeToLong(nextTime1) > timeToLong(GetTime())) { // Если время таймера больше текущего
        long lnextTime=timeToLong(nextTime1);
        if (lminTime >= lnextTime) {
          lminTime = timeToLong(nextTime1);
          sendOptions("timersT", jsonRead(tim, "run1")+" "+jsonRead(tim, "id")+";"+nextTime1+" "+ jsonRead(tim, "com1"));
        }
      }
    }
    }
    i++; // увеличим индекс
  } while (tim != "NAN"); // повторим пока строка не пустая
}

long  timeToLong(String Time) {
  //"00:00:00"  время в секунды

  long  second = selectToMarker(Time, ":").toInt() * 3600; //общее количество секунд в полных часах
  Time = deleteBeforeDelimiter (Time, ":"); // Теперь здесь минуты секунды
  second += selectToMarker(Time, ":").toInt() * 60; // Добавим секунды из полных минут
  Time = deleteBeforeDelimiter (Time, ":"); // Теперь здесь секунды
  second += selectToMarker(Time, ":").toInt(); // Добавим секунды

  return second;
}

int  timeToMin(String Time) {
  //"00:00:00"  время в секунды
  long  min = selectToMarker(Time, ":").toInt() * 60; //общее количество секунд в полных часах
  Time = deleteBeforeDelimiter (Time, ":"); // Теперь здесь минуты секунды
  min += selectToMarker(Time, ":").toInt(); // Добавим секунды из полных минут
  return min;
}

String timeToString(long Time) {
  String str;
  uint8_t temp = (Time / 3600);
  if (temp < 10) str += "0";
  str += temp; // здесь часы
  str += ":";
  Time = (Time - temp * 3600); // отбросим часы
  temp = (Time / 60);
  if (temp < 10) str += "0";
  str += temp; // здесь минуты
  str += ":";
  Time = (Time - temp * 60); // отбросим минуты здесь секунды
  if (Time < 10) str += "0";
  str += Time;
  return str;
}

void timeTest() {
  String timeNow = GetTime();
  String timersT = getOptions(timersTS);
  String timeTimer = selectToMarkerLast(timersT, ";");
  String command =deleteBeforeDelimiter(timeTimer, " ");
  timeTimer = selectToMarker(timeTimer, " ");
  if (timeNow == timeTimer) {
    sCmd.readStr(command);
    loadTimer();
    // добавить удаление одноразового таймера
  }
}
#endif
