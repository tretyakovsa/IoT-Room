// ------------- Запись значения json String в Массив
void jsonArrAdd(String & json, String name, String volume) {
  jsonArrAddWriteVol(json, name, "\"" + volume + "\"");
}
// ------------- Запись значения json int в Массив
void jsonArrAdd(String & json, String name, int volume) {
  jsonArrAddWriteVol(json, name, (String)volume);
}
// ------------- Запись значения json int в Массив
void jsonArrAdd(String & json, String name, float volume) {
  jsonArrAddWriteVol(json, name, (String)volume);
}
// ------------- Запись значения json String в Массив
void jsonArrAddWriteVol(String & json, String name, String volume) {
  String KeyAndVol = jsonKeyAndVol(json, name);
  String arr = jsonRead(json, name);
  if (KeyAndVol != "") { // Есть ключ в json
    if (arr.startsWith("[") && arr.endsWith("]")) { // если данные в квадратных скобках то это данные массива
      if (arr != "[]") {
        arr.setCharAt(arr.lastIndexOf("]"), ','); // заменим } на ,
      }  else     arr.replace("]", "");
      arr += volume + "]";
      jsonWriteVol(json, name, arr);
    }
  } else { // нет ключа добавить данные массива
    jsonWriteVol(json, name, "[" + volume + "]"); // если массив пустой
  }
}
String jsonKeyAndVol(String &json, String name) {
  json.trim(); // пропустить все пробелы в начале
  String fullNameKey = "\"" + name + "\":";//подготовить ключ для поиска
  int lenlNameKey = fullNameKey.length();// размер ключа в символах
  int kayBegin = json.indexOf(fullNameKey); // Поиск начала ключа
  if (kayBegin == -1) return "";                    // Если ключ не найден вернем пустую строку
  else  {                                           // Иначе
    // Проверим наличие массива
    String keyStop = ",\""; // конец ключа если нет массива иначе keyStop = "]"
    if (json.substring(kayBegin + lenlNameKey, kayBegin + lenlNameKey + 1) == "[") keyStop = "]";
    int kayEnd = json.indexOf(keyStop, kayBegin);       // Поиск конца ключа
    if (keyStop == "]")    ++kayEnd;
    //if (name == "SSDP1") {
    //  Serial.print("json= ");
    //  Serial.println(json);
    //}
    String volume = json.substring(kayBegin, kayEnd); // Выделим значение ключа с ковычками
    //if (name == "SSDP1") {
    //  Serial.print("volume= ");
    //  Serial.println(volume);
    //}
    volume.replace("}", ""); // Удалим лишний символ } если элемент был последним
    return volume;
  }
}
void jsonDel(String &json, String name) {
  json.trim();
  if (!json.startsWith("{") || !json.endsWith("}")) { // если это не json создадим пустой
    json = "{}";
  }
  json.replace(jsonKeyAndVol(json, name) , "");
  json.replace(",,", ",");
}
void jsonWriteVol(String &json, String name, String volume) {
  json.trim();
  if (!json.startsWith("{") || !json.endsWith("}")) { // если это не json создадим пустой
    //Serial.println("empti");
    json = "{}";
  }
  String KeyAndVol = jsonKeyAndVol(json, name);
  String NewKeyAndVol = "\"" + name + "\":" + volume;
  if (KeyAndVol != "") {
    json.replace(KeyAndVol, NewKeyAndVol);
  } else {
    if (json != "{}") {
      json.setCharAt(json.lastIndexOf("}"), ','); // заменим } на ,
      json += NewKeyAndVol + '}';
    } else {
      json = "{" + NewKeyAndVol + "}";
    }
  }
}

// ------------- Чтение значения json String
String jsonRead(String & json, String name) {
  String volume = jsonKeyAndVol(json, name);
  volume = volume.substring(volume.indexOf(":") + 1); // получим значение пары
  if (volume.startsWith("\"") && volume.endsWith("\"")) { // если данные в кавычках
    volume = volume.substring(1, volume.length() - 1); // удалим кавачки
  }
  return volume;
}
int jsonReadToInt(String &json, String name) {
  return jsonRead(json, name).toInt();
}
float jsonReadToFloat(String &json, String name) {
  return jsonRead(json, name).toFloat();
}

// ------------- Запись значения json String
void jsonWrite(String &json, String name, String volume) {
  volume = "\"" + volume + "\"";
  jsonWriteVol(json, name, volume);
}

// ------------- Запись значения json int
void jsonWrite(String &json, String name, int volume) {
  String volSrt;
  volSrt += volume;
  jsonWriteVol(json, name, volSrt);
}

// ------------- Запись значения json float
void jsonWrite(String &json, String name, float volume) {
  String volSrt;
  volSrt += volume;
  jsonWriteVol(json, name, volSrt);
}


// -------------- Чтение configSetup -------------------------------
String getSetup(String Name) {
  return jsonRead(configSetup, Name);
}
int getSetupInt(String Name) {
  return jsonReadToInt(configSetup, Name);
}
float getSetupFloat(String Name) {
  return jsonReadToFloat(configSetup, Name);
}
// -------------- Чтение configOptions --------------------
String getOptions(String Name) {
  return jsonRead(configOptions, Name);
}
int getOptionsInt(String Name) {
  return jsonReadToInt(configOptions, Name);
}
float getOptionsFloat(String Name) {
  return jsonReadToFloat(configOptions, Name);
}
// -------------- Чтение configJson     -----------------------------------------------
String getStatus(String Name) {
  return jsonRead(configJson, Name);
}
int getStatusInt(String Name) {
  return jsonReadToInt(configJson, Name);
}
float getStatusFloat(String Name) {
  return jsonReadToFloat(configJson, Name);
}

// ------------- Отправить данные в Setup String ------------------------------
void sendSetup(String Name, String volume) {
  jsonWrite(configSetup, Name, volume);
}
// ------------- Отправить данные в Setup int -----------------------
void sendSetup(String Name, int volume) {
  jsonWrite(configSetup, Name, volume);
}
// ------------- Отправить данные в Setup float ------------
void sendSetupF(String Name, float volume) {
  jsonWrite(configSetup, Name, volume);
}
// ------------- Отправить данные в Status String ------------------------------
boolean sendStatus(String Name, String volume, boolean fifo) {
  jsonWrite(configJson, Name, volume);
  return true;
}
boolean sendStatus(String Name, String volume) {
  jsonWrite(configJson, Name, volume);
  scensrioFIFO+=Name+",";
  return true;
}
// ------------- Отправить данные в Status int -----------------------
boolean sendStatus(String Name, int volume) {
  jsonWrite(configJson, Name, volume);
  scensrioFIFO+=Name+",";
  return true;
}
boolean sendStatus(String Name, int volume,boolean fifo) {
  jsonWrite(configJson, Name, volume);
  return true;
}
// ------------- Отправить данные в Status float ----------
boolean sendStatusF(String Name, float volume) {
  jsonWrite(configJson, Name, volume);
  return true;
}
// ------------- Отправить данные в Options String ------------------------------
void sendOptions(String Name, String volume) {
  jsonWrite(configOptions, Name, volume);
}
// ------------- Отправить данные в Options int -----------------------
void sendOptions(String Name, int volume) {
  jsonWrite(configOptions, Name, volume);
}
// ------------- Отправить данные в Options float ----------
void sendOptionsF(String Name, float volume) {
  jsonWrite(configOptions, Name, volume);
}
// ------------- Копируем параметр из Setup Options
void setupToOptions(String Name) {
  sendOptions(Name, getSetup(Name));
}
// ------------- Копируем параметр из Setup Options
void setupToStatus(String Name) {
  sendStatus(Name, getSetup(Name));
}

String getArray(String Name, String kay, int index) {
  String tmp = deleteBeforeDelimiter(Name, "\"" + kay + "\":[");
  tmp = selectToMarker (tmp, "]") + ",";
  if (tmp == ",") return "NAN";
  tmp = selectFromMarkerToMarker(tmp, "},", index);
  if (tmp == "") {
    tmp = "NAN";
    return tmp;
  }
  return tmp + "}";
}
