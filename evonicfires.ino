#ifdef evonicfires //
// ------------------- Инициализация PinOut
void initAudio() {
  if (getSetup(templevelS)=="") sendSetup(templevelS,11);
  setupToStatus(templevelS);
  sCmd.addCommand(audioS.c_str(), audio); //
  commandsReg(audioS);
  modulesReg(audioS);
}
// ------------------ Управление audio
// audio set http://evoflame.co.uk/audio/test.mp3 запустит трек
// audio on 100 Включить звук с уровнем громкости
// audio off Выключить звук
// audio volume 100 Изменить громкость
void audio() {
  String com = readArgsString(); //комманда
  if (com == "set") {
    String trec = readArgsString(); // адрес
    // Отправить в сокет
    AudioData(audioS, trec);
  }
  if (com == "on") {
    String volume = readArgsString(); // Громкость
    if (volume == "") volume = "0";
      // Отправить в сокет
      AudioData(volumeS, volume);
  }
  if (com == "off") {
    // Отправить в сокет
    AudioData(volumeS, "0");
  }

}
void AudioData(String key, String data) {
  String broadcast = "{}";
  jsonWrite(broadcast, key, data);
  WebSocketsClient.sendTXT(broadcast);
}

/*
0000FF 50 - 18
ff5000 200 - 19
payed
 */
void effectTest(String effect) {
  String urls = "http://evoflame.co.uk/effect/" + effect + "/" + getSetup(mailS);
  effect =  MyWiFi.getURL(urls); // Получить настройки эффекта с сервера
  effect = "rgb set 0 " + effect; // Добавим комманду к первой ленте
  String effect0 = selectToMarker (effect, "\r\n");
  effect=deleteBeforeDelimiter(effect, "\r\n");
  effect = "rgb set 1 " + effect; // Добавим комманду к второй ленте
  String effect1 = selectToMarker (effect, "\r\n");
  if (selectToMarkerLast (effect, "\r\n") == "payed") {
  //  Serial.println("Куплен");
    sCmd.readStr(selectToMarker (effect, "\r\n"));
    sCmd.readStr(selectToMarker (effect, "\r\n"));
  } else {
    sCmd.readStr(selectToMarker (effect, "\r\n"));
    sCmd.readStr(selectToMarker (effect, "\r\n"));
    sCmd.readStr("pulse on rgb0 30000");
    sCmd.readStr("pulse on rgb1 30000");
  }
}

// ----------------- OneWire -------------------------------------------------------------------------------
void initOneWire() {
  sCmd.addCommand("AUDIO",       initAudio);
  setupToStatus(templevelS);
  sCmd.addCommand(temperatureS.c_str(), setLevel); //
 // Serial.println("Start temp");
  uint8_t pin = readArgsInt(); // Пин датчика
  pin =  pinTest(pin, HIGH); // Проверить пин в мульти режиме
  static uint16_t t = readArgsInt(); // Время чтения
  static uint8_t averageFactor = readArgsInt(); // Маштаб измерения пока не включен
  if (t < 750) t = 5000; // Если время мало увеличить
  oneWire =  new OneWire(pin); // Включить интерфейс OneWire
  sensors.setOneWire(oneWire); // Передать на каком OneWire датчики DS
  sensors.begin(); // Включиь датчики DS
  byte num = sensors.getDS18Count(); // Сколько датчиков DS18 на интерфейсе
  sensors.requestTemperatures();
  if (num > 0) {
    sensors.setResolution(9);
    String temp;
    temp = +sensors.getTempCByIndex(0);
    sendStatus(temperatureS, temp.toInt()); // Запишем значение в статус
    sendStatus(alarmS+temperatureS,0);
    alarmLoad(temperatureS); // Загрузить уровни датчика
    alarmTest(temperatureS);

    modulesReg(temperatureS); // Зарегистрировать модуль
    sCmd.addCommand(temperatureS.c_str(), setLevel); //
    sendOptions(temperatureS + "num", num); // Записать количество датчиков
    ts.add(tDS, t, [&](void*) { // Включить чтение датчика с интервалом t
      sensors.requestTemperatures();
      String tempF;
      tempF = +sensors.getTempCByIndex(0);
      int temp = tempF.toInt();
      SoketData (temperatureS, (String)temp, getStatus(temperatureS));
      sendStatus(temperatureS, (String)temp); // Запишем значение в статус
      alarmTest(temperatureS);
    }, nullptr, true);
  }
}
// -----------------  DHT
void initDHT() {
  uint8_t pin = readArgsInt(); // Пин датчика
  pin =  pinTest(pin, HIGH); // Проверить пин в мульти режиме
  dht.setup(pin); // Включиь датчики DHT
  delay(1000);
  static uint16_t t = readArgsInt(); // Время чтения
  static uint16_t test = dht.getMinimumSamplingPeriod(); // Получить время чтения датчика
  if (t < test) t = test;
  //Serial.println(t);
  String temp;
  temp += dht.getTemperature();
  //  Serial.println(temp);
  if (temp != "nan") { // проверим подключен ли датчик
    sendStatus(temperatureS, dht.getTemperature()); // Запишем значение температуры в статус
    alarmLoad(temperatureS);
    alarmTest(temperatureS);
    sendStatus(humidityS, dht.getHumidity()); // Запишем значение влажности в статус
    alarmLoad(humidityS); // Загрузить уровни датчика
    alarmTest(humidityS);
    ts.add(tDHT, test, [&](void*) { // Включить чтение датчика с интервалом t
      int temp = dht.getTemperature();
      SoketData (temperatureS, (String)temp, getStatus(temperatureS));
      sendStatus(temperatureS, temp); // Запишем значение температуры в статус
      sendStatus(humidityS, dht.getHumidity()); // Запишем значение влажности в статус
      alarmTest(temperatureS); // Проверить уровни сработки
      alarmTest(humidityS);
    }, nullptr, true);
    modulesReg(temperatureS); // Зарегистрировать модуль
    modulesReg(humidityS);
    sCmd.addCommand(temperatureS.c_str(), setLevel); //
  }
}

void setLevel() {
  int temp = readArgsInt();

  String high = highS + AlarmS + temperatureS;
  String low = lowS + AlarmS + temperatureS;
  //int temp = getStatusInt(templevelS);
  //int temp = com;
  //if (com == "+") temp++;
  //if (com == "-") temp--;
  SoketData (templevelS, (String)temp, getStatus(templevelS));
  sendStatus(templevelS, temp);
  //Serial.println(getStatus(templevelS));
  sendOptionsF(high, temp);
  sendOptionsF(low, temp);

  //Serial.println(temp);
  alarmLoad(temperatureS);
  //sCmd.readStr("param not Save");
  sendSetup(templevelS, temp);
  saveConfigSetup();

}
// ----------------------- Загрузка данных уровней сработки ------------------------------------------
// Имя параметра теста и имена границ
void alarmLoad(String sName) {

  String high = highS + AlarmS + sName;
  String low = lowS + AlarmS + sName;
  sendOptionsF(high, getStatusInt(templevelS));
  sendOptionsF(low, getStatusInt(templevelS));
}

// ------------------------------- Проверка уровней ------------------------------------------------
// Текущее значение сенсора уровни признак датчика
void alarmTest(String value) {
  String high = highS + AlarmS + value;
  String low = lowS + AlarmS + value;
  String sAlarm = alarmS + value;
//  Serial.print(getStatusFloat(value));
//  Serial.print(" ");
//  Serial.print(getOptionsFloat(high));
//  Serial.print(" and ");
//  Serial.println(getOptionsInt(sAlarm));
  if (getOptionsFloat(high) != 0 || getOptionsFloat(low) != 0) { //Работает если уровни не равны нулю (неправильно нужен флаг)
    // нужно добавить флаг остановки теста
    if (getStatusFloat(value) > getOptionsFloat(high) && getOptionsInt(sAlarm) == LOW) { //если Текущий стстус 0 и текущее значение больше верхнего
      SoketData (sAlarm, (String)HIGH, getStatus(sAlarm));
      sendOptions(sAlarm, HIGH); // Текущий стстус в 1
      sendStatus(sAlarm, HIGH); // Текущий стстус в 1
      flag = sendStatus(value, getStatusFloat(value)); // Запустить скрипт
    }
    if (getStatusFloat(value) < getOptionsFloat(low) && getOptionsInt(sAlarm) == HIGH) { //если Текущий стстус 1 и текущее значение меньше нижнего
      SoketData (sAlarm, (String)LOW, getStatus(sAlarm));
      sendOptions(sAlarm, LOW); //Текущий стстус в 0
      sendStatus(sAlarm, LOW); //Текущий стстус в 0
      flag = sendStatus(value, getStatusFloat(value)); // Запустить скрипт
    }
  }
}
#endif
