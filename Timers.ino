void initTimers() {
  loadTimer();
  modulesReg("timers");
}
void delTimer() {
  String jsonTimer = readFile(configTimerS, 4096);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& Timers = jsonBuffer.parseObject(jsonTimer);
  JsonArray& nestedArray = Timers["timer"].asArray();
  uint8_t y = 255;
  for (int i = 0; i <= nestedArray.size() - 1; i++) {
    if (Timers["timer"][i]["id"] == idTime) y = i;
  }
  if (y != 255) {
    nestedArray.removeAt(y);
    jsonTimer = emptyS;
    Timers.printTo(jsonTimer);
    writeFile(configTimerS, jsonTimer);
    //Serial.println(jsonTimer);
  }
}
// {"timer":[
// {"id":6422,"day":"1111111","time1":"20:59:58","com1":"relay not 1","run1":"0","active":1},
// {"id":3912,"day":"1111111","time1":"21:00:58","com1":"relay not 1","run1":"0","active":1},
// {"id":8167,"day":"1111111","time1":"21:01:58","com1":"relay not 1","run1":"1","active":1}
// ]}


void loadTimer() {
  Serial.println("Load Timers");
  long lminTime = 86400;
  String jsonTimer = readFile(configTimerS, 4096);
  String Weekday = GetWeekday();
  uint8_t w = indexWeekday(Weekday);
  comTime = emptyS;
  uint8_t i = 0;
  String tim;

  do {
    tim = getArray(jsonTimer, "timer", i); // получим строку таймера
    String week = jsonRead(tim, "day"); // Проверим активность в текущий день недели
    if (week.substring(w, w + 1) == "1") { // Если таймер активен сегодня
      String nextTime1 = jsonRead(tim, "time1"); // Получим время таймера
      uint8_t active = jsonReadToInt(tim, "time1"); // Получим время таймера
      if (timeToLong(nextTime1) >= timeToLong(GetTime()) && active) { // Если время таймера меньше текущего
        if (lminTime >= timeToLong(nextTime1)) {
          lminTime = timeToLong(nextTime1);
          minTime = nextTime1;
          comTime = jsonRead(tim, "com1");
          runTime = jsonRead(tim, "run1");
          idTime = jsonRead(tim, "id");
        }
      }
    }
    i++; // увеличим индекс
  } while (tim != "NAN"); // повторим пока строка не пустая
  sendOptions("timersT", minTime);
  sendOptions("timersC", comTime);
  sendOptions("timersR", runTime);

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
