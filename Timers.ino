void initTimers() {
  loadTimer();
  modulesReg("timers");
}
void delTimer() {
  String jsonTimer = readFile(configTimerS, 4096);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& Timers = jsonBuffer.parseObject(jsonTimer);
  JsonArray& nestedArray = Timers["timer"].asArray();
  uint8_t y=255;
  for (int i = 0; i <= nestedArray.size() - 1; i++) {
    if (Timers["timer"][i]["id"] == idTime) y = i;
  }
  if (y!=255){
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


void loadTimer(){
  Serial.println("Load Timers");
  long lminTime = 86400;
  String jsonTimer = readFile(configTimerS, 4096);
  Serial.println(jsonTimer);
  Serial.println(getArray(jsonTimer, "timer",0));
  String Weekday = GetWeekday();
  Serial.println(Weekday);
  //Serial.println(indexWeekday(Weekday));
  comTime = emptyS;
  uint8_t i=0;
  String tim;

 do {

   tim = getArray(jsonTimer, "timer",i);
   Serial.println(tim);
    i++; // увеличим индекс
  } while (tim !="NAN"); // повторим пока строка не пустая


  }
void loadTimer1(String timers, uint8_t index){


  }



void loadTimer2() {
  Serial.println("Load Timers");
  long lminTime = 86400;
  String jsonTimer = readFile(configTimerS, 4096);
  Serial.println(jsonTimer);
  Serial.println(getArray(jsonTimer, "timer",1));
  String Weekday = GetWeekday();
  Serial.println(Weekday);
  comTime = emptyS;
  //  Получаем текущий день недели в виде числа
  uint8_t iDay = 7;
  if (Weekday == "Sun") iDay = 0;
  if (Weekday == "Mon") iDay = 1;
  if (Weekday == "Tue") iDay = 2;
  if (Weekday == "Wed") iDay = 3;
  if (Weekday == "Thu") iDay = 4;
  if (Weekday == "Fri") iDay = 5;
  if (Weekday == "Sat") iDay = 6;
  Serial.println(iDay);
  // Приготовились читать массив таймеров
  DynamicJsonBuffer jsonBuffer;
  JsonObject& Timers = jsonBuffer.parseObject(jsonTimer);
  JsonArray& nestedArray = Timers["timer"].asArray();
  // Сколько тамеров в массиве
  int j = nestedArray.size();
  // Если есть таймеры
  if (j != 0) {
    // Перебор всех по порядку
    for (int i = 0; i <= j - 1; i++) {
      // Возьмем только те которые соответствуют текущему дню недели
      String week = Timers["timer"][i]["day"].as<String>(); // признак принодлежности к дню недели
      int ind = week.substring(iDay, iDay + 1).toInt(); // Выделяем нужный день недели
      if (ind) { // Если день недели совпадает
        String idTimes = Timers["timer"][i]["id"].as<String>();
        // Serial.println(idTimes);
        if (idTimes.indexOf(":")==-1) idTimes=emptyS;
        String nextTime1 = Timers["timer"][i]["time1"].as<String>();
       // Serial.println(nextTime1);
        String nextcom1 = Timers["timer"][i]["com1"].as<String>();
        //Serial.println(nextcom1);
        String nextRun1 = Timers["timer"][i]["run1"].as<String>();
       // Serial.println(nextRun1);
        String nextId = Timers["timer"][i]["id"].as<String>();
       // Serial.println(nextId);
        int active = Timers["timer"][i]["active"];
       Serial.println(active);
        if (timeToLong(nextTime1) >= timeToLong(GetTime()) && active) {
          Serial.println("Test");
          if (lminTime >= timeToLong(nextTime1)) {
            lminTime = timeToLong(nextTime1);
            minTime = nextTime1;
            comTime = nextcom1;
            runTime = nextRun1;
            idTime = nextId;
          }
        }
        sendOptions("timersT", minTime);
        sendOptions("timersC", comTime);
        sendOptions("timersR", runTime);
      }
    }
  }
  Serial.println(minTime);
}



long  timeToLong(String Time) {
  //"00:00:00"  время в секунды
  Serial.print(Time+"=");
  long  second = selectToMarker(Time, ":").toInt() * 3600; //общее количество секунд в полных часах
  Time = deleteBeforeDelimiter (Time, ":"); // Теперь здесь минуты секунды
  second += selectToMarker(Time, ":").toInt() * 60; // Добавим секунды из полных минут
  Time = deleteBeforeDelimiter (Time, ":"); // Теперь здесь секунды
  second += selectToMarker(Time, ":").toInt(); // Добавим секунды
  Serial.println(second);
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
