// Плагин для загрузки FS https://github.com/esp8266/arduino-esp8266fs-plugin/releases/download/0.3.0/ESP8266FS-0.3.0.zip
//---------- Системные библиотеки
#include <ArduinoJson.h>             //Ставим через менеджер библиотек
#include <TickerScheduler.h>         //https://github.com/Toshik/TickerScheduler
enum { tNTP, tSSDP, tIP, tA0, tDS, tDHT, tSI, tIR, tRC, tPOW, tRSSI, tMQTT, tDDNS, tCRIB };
TickerScheduler ts(15);
#include <Ticker.h>
Ticker flipper[8];
#include <StringCommand.h>           //https://github.com/tretyakovsa/ESP8266-StringCommand
StringCommand sCmd;
#include <time.h>                    //Содержится в пакете
#include <FS.h>                      //Содержится в пакете
//---------- WiFi сеть
#include <wificonnect.h>             //https://github.com/tretyakovsa/wificonnect
WIFICONNECT MyWiFi;
//---------- Службы обнаружения
//#include <ESP8266LLMNR.h>
//#include <ESP8266NetBIOS.h>
#include <ESP8266SSDP.h>             //Содержится в пакете
//---------- Сетевой обмен Библиотеки
#include <WiFiUdp.h>                 //Содержится в пакете
WiFiUDP udp;
#include <ESP8266HTTPClient.h>       //Содержится в пакете
WiFiClient wclient; //?
//---------- WEB сервера
#include <ESP8266WebServer.h>        //Содержится в пакете
ESP8266WebServer HTTP(80);
ESP8266WebServer HTTPWAN(8080);
File fsUploadFile;

//---------- Обновление системы
#include <ESP8266HTTPUpdateServer.h> //Содержится в пакете
ESP8266HTTPUpdateServer httpUpdater;
#include <ESP8266httpUpdate.h>       //Содержится в пакете


//---------- Глобальные переменные
String configJson = "{}";            // Здесь все статусы /config.live.json
String configOptions = "{}";         // Здесь данные для всех страниц /config.options.json
String configSetup = "{}";           // Здесь данные для setup /config.setup.json
String ssdpList = "{}";              // SSDP имя ip всех модулей в сети /ssdp.list.json
boolean pins[21];
String modules = "{\"module\":[]}";
String regCommands = "{\"command\":[]}"; //список доступных комманд для различных модулей /command.json

//----------   Здесь будут храниться все текстовые константы для сокращения кода
const String fileConfigS   = "config.save.json"; // Имя файла конфигурации
const String ScenaryS   = "scenary.save.txt";    // Имя файла сценарии
const String configTimerS   = "timer.save.json"; // Имя файла таймеры

//---------- Общие
String chipID;
const String emptyS   = emptyS;
const String onS   = "on";
const String offS   = "off";
const String notS   = "not";
const String messageS   = "message";
const String configsS   = "configs";
const String urlsStat = "http://backup.privet.lv/visitors/?";
const String voiceS   = "voice";

// ----------------- Web
String Lang = "ru";                  // файлы языка web интерфейса
String statusS = "{}";
const String texthtmlS   = "text/html";
const String setIndexS   = "setIndex";
const String langS   = "lang";
const String ssdpS   = "SSDP";
const String spaceS   = "space";
const String langOffS = "{{LangOff}}";
const String langOnS = "{{LangOn}}";
const String titleS   = "title";
const String classS   = "class";
const String btnS   = "btn btn-block btn-lg btn-";
const String infoS   = "info";
const String primaryS   = "primary";

// ----------------- wifi
const String wifiS   = "wifi";
const String ssidS   = "ssid";
const String ssidPassS   = "ssidPass";
const String wifiConnectS   = "wifiConnect";
const String wifiBlinkS   = "wifiBlink";
const String checkboxIPS   = "checkboxIP";
const String ipS   = "ip";
const String subnetS   = "subnet";
const String getwayS   = "getway";
const String dnsS   = "dns";
const String ssidAPS   = "ssidAP";
const String ssidApPassS   = "ssidApPass";


// --------- пульс
String pulsList = "{}";
uint8_t pulsNum =1;
const String pulseS   = "pulse";
const String pulseComS ="pulseCom";
const String pulseTimeS ="pulseTime";

// ---------- Scenary
String Scenary;                      // Строка обработки сценариев

// --------- Update
 const String Uploader = ""; // Если перепрошивка происходит без Uploader раскоментировать и закоментировать следующую
//const String Uploader = "http://backup.privet.lv/esp/sonoff/Sonoff_Updater.ino.generic.bin";
const String spiffsDataS   = "spiffsData";
const String buildDataS   = "buildData";
const String spiffsS   = "spiffs";
const String buildS   = "build";
const String urlsPath = "http://backup.privet.lv/iotroom/";
const String fNameSPIFFS = "IoT-Room.spiffs.bin";
const String fNameBuild = "IoT-Room.ino.generic.bin";
const String urlsPathS = "urlsPath";
const String fNameSPIFFSS = "fSPIFFS";
const String fNameBuildS = "fBuild";

// --------- NTP
const String timeZoneS   = "timeZone";
const String ntpS   = "NTP";
const String ntpServerS   = ".vniiftri.ru";
const String weekdayS   = "weekday";


// --------- Timers
String idTime;
String comTime;
String minTime;
String runTime;
const String timeS   = "time";

// --------- SSDP
uint16_t tIP_Time = 60000;
uint16_t tSSDP_Time = 60000;
const String ssdpDef   = "IoT-Room";
const String spaceDef   = "home";
const String modelURL   = "https://github.com/tretyakovsa/Sonoff_WiFi_switch";
const String manufacturer = "Tretyakov Sergey, Kevrels Renats";
const String manufacturerURL = "http://www.esp8266-arduinoide.ru";

// -------- Scenary
boolean flag = false;
boolean thenOk;

#ifdef pinOutM
// -------- relay
const String relayS   = "relay"; //"stateRelay"; // Состояние реле
const String PinS   = "Pin"; // Для хранения ножек реле
const String NotS   = "Not"; // Для хранения признака инверсии
// -------- PinOut
const String pinOutS   = "pinout"; //"stateRelay"; // Состояние реле
#endif

#ifdef buzzerM
// -------- buzzer
//const String stateBuzzerS   = "stateBuzzer";
const String buzzerPinS   = "buzzerPin";
const String toneS   = "tone";
#endif

#ifdef TachM
// -------- Tach
#include <Bounce2.h>                 // https://github.com/thomasfredericks/Bounce2
#define NUM_BUTTONS 8
boolean but[NUM_BUTTONS];
Bounce * buttons = new Bounce[NUM_BUTTONS];
const String stateTachS   = "Tach"; // Состояние кнопки
const String tachS   = "tach";
const String invTachS   = "invTach";
#endif

#ifdef irM
// -------- IR
#include <IRremoteESP8266.h>         //https://github.com/markszabo/IRremoteESP8266
#include <IRrecv.h>
#include <IRutils.h>
#include <IRsend.h>
IRrecv *irReceiver;
decode_results results;
IRsend *irSender;
const String irReceivedS   = "irReceived";
const String irDecodeTypeS   = "irDecodeType";
const String irTransmitterS   = "irTransmitter";
const String rfReceivedS   = "rfReceived";
const String rfBitS   = "rfBit";
const String rfProtocolS   = "rfProtocol";
const String irsendS   = "irsend";
#endif
// -------- RGB
#ifdef rgbM // #endif
#define LED_COUNT 86
#include <WS2812FX.h>                //https://github.com/renat2985/WS2812FX
WS2812FX ws2812fx[2] = WS2812FX(LED_COUNT, 5, NEO_GRB + NEO_KHZ800);
#include <NeoPixelBus.h>
NeoEsp8266Dma800KbpsMethod dma = NeoEsp8266Dma800KbpsMethod(LED_COUNT, 3);
//NeoEsp8266Uart1800KbpsMethod dma1 = NeoEsp8266Uart1800KbpsMethod(LED_COUNT, 3);
NeoEsp8266Uart0800KbpsMethod dma1 = NeoEsp8266Uart0800KbpsMethod(LED_COUNT, 3);
const String stateRGBS   = "rgb"; //"stateRGB"; Состояние RGB WS
const String colorRGBS   = "colorRGB";
const String brightnessRGBS   = "brightnessRGB";
const String modeRGBS   = "modeRGB";
const String speedRGBS   = "speedRGB";
const String rgbS   = "rgb";
#endif

#ifdef webSoketM //
#include <WebSocketsServer.h>    //https://github.com/Links2004/arduinoWebSockets
WebSocketsServer webSocket = WebSocketsServer(81);
#include <WebSocketsClient.h>
WebSocketsClient WebSocketsClient;
const String webSocketS   = "webSocket";
#endif

#ifdef A0M //
// Ocвещенность
const String stateA0S   = "A0";
const String highalarmA0S   = "highAlarmA0";
const String lowalarmA0S   = "lowAlarmA0";
const String alarmA0S   = "alarmA0";
#endif

#ifdef DS18B20M //
#include <OneWire.h>                 //Ставим через менеджер библиотек
#include <DallasTemperature.h>       //Ставим через менеджер библиотек
OneWire *oneWire;
DallasTemperature sensors;
#endif

#ifdef DHTM // #endif
#include <DHT.h>                     //https://github.com/markruys/arduino-DHT
DHT dht;
#endif

// Температура
const String temperatureS   = "temperature";
const String highalarmtempS   = "highAlarmTemp";
const String lowalarmtempS   = "lowAlarmTemp";
const String alarmtempS   = "alarmTemp";

// Влажность
const String humidityS   = "humidity";
const String highalarmhumS   = "highAlarmHum";
const String lowalarmhumS   = "lowAlarmHum";
const String alarmhumS   = "alarmHum";
