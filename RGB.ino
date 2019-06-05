#ifdef rgbM // #endif
// RGB 4 1 60 0 ff5000 200 255 0
void initRGB() {
  byte pin = readArgsInt(); // pin
  pin =  pinTest(pin);
 // if (pin == 1 || pin == 3 ) {
    String num = readArgsString(); // второй аргумент прификс RGB 0 1 2
    byte numI = num.toInt();
    sendOptions("rgbNum", getOptionsInt("rgbNum") + 1); // увеличим количество лент +1
    // Реагирует на комманду rgb
    sCmd.addCommand(rgbS.c_str(),  rgb);
    commandsReg(rgbS);
    ws2812fx[numI].setSegment(0,  0,  readArgsInt() - 1, FX_MODE_CUSTOM, 0xFF0000, 50, false); // segment 0 is leds 0 - 9
    int state = readArgsInt(); //состояние
    sendStatus(rgbS + num, state);
    sendStatus(colorRGBS + num,  readArgsString()); //цвет
    sendStatus(speedRGBS + num, readArgsInt()); //скорость
    sendStatus(brightnessRGBS + num, readArgsInt()); //яркость
    sendStatus(modeRGBS + num, readArgsInt()); //режим
    ws2812fx[numI].init();
    if (pin==1){
    //if (numI == 0) {
      dma.Initialize();
      ws2812fx[numI].setCustomShow(myCustomShow);
    }
    if (pin==3){
    //if (numI == 1) {
      dma1.Initialize();
      ws2812fx[numI].setCustomShow(myCustomShow1);
    }

    ws2812fx[numI].setColor(setColorStringI(colorRGBS + num, numI));
    int temp = getStatusInt(speedRGBS + num);
    ws2812fx[numI].setSpeed(convertSpeed(temp)); // Скорость
    ws2812fx[numI].setBrightness(getStatusInt(brightnessRGBS + num)); //Яркость
    int modeI = getStatusInt(modeRGBS + num);
    ws2812fx[numI].setMode(modeI); // Режим
    if (numI == 0)  ws2812fx[0].setCustomMode(myCustomEffect0);
    if (numI == 1)  ws2812fx[1].setCustomMode(myCustomEffect1);
    //регистрируем модуль
    modulesReg(rgbS + num);
    actionsReg(rgbS + num);
    if (state) ws2812fx[numI].start();
    if (!state) ws2812fx[numI].stop();
 // }
}
void myCustomShow(void) {
  if (dma.IsReadyToUpdate()) {
    // copy the ws2812fx pixel data to the dma pixel data
    memcpy(dma.getPixels(), ws2812fx[0].getPixels(), dma.getPixelsSize());
    dma.Update();
  }
}
void myCustomShow1(void) {
  if (dma1.IsReadyToUpdate()) {
    // copy the ws2812fx pixel data to the dma pixel data
    memcpy(dma1.getPixels(), ws2812fx[1].getPixels(), dma1.getPixelsSize());
    dma1.Update();
  }
}

uint32_t setColorStringI(String color, int num) {
  uint32_t   tmp = strtol(("0x" + getStatus(color)).c_str(), NULL, 0);
  if (tmp >= 0x000000 && tmp <= 0xFFFFFF) {
    return tmp;
  }
}

void rgb() {
  String com = readArgsString(); //комманда
  String num = readArgsString(); // номер RGB
  byte numI = num.toInt();
  if (numI > 1) return;
  String color = readArgsString(); //цвет
  String speed = readArgsString(); //скорость
  String brightness = readArgsString(); //яркость
  String mode = readArgsString(); //режим
  uint8_t state = getStatusInt(stateRGBS + num);
  if (com == "set") {
    if (color != emptyS) {
      if (color == "-") {}
      else {
        sendStatus(colorRGBS + num,  color);
        ws2812fx[numI].setColor(setColorStringI(colorRGBS + num, numI));
      }
    }
    if (speed != emptyS) {
      if (speed == "-") {}
      else {
        int temp = speed.toInt();
        flag = sendStatus(speedRGBS + num,  speed);
        ws2812fx[numI].setSpeed(convertSpeed(temp));
      }
    }
    if (brightness != emptyS) {
      if (brightness == "-") {}
      else {
        int temp = brightness.toInt();
        flag = sendStatus(brightnessRGBS + num,  temp);
        ws2812fx[numI].setBrightness(temp);
      }
    }
    if (mode != emptyS) {
      if (mode == "-") {}
      else {
        int temp = mode.toInt();
        sendStatus(modeRGBS + num,  temp);
        ws2812fx[numI].setMode(temp);
      }
    }
    if (!ws2812fx[numI].isRunning())    ws2812fx[numI].start();
    sendStatus(stateRGBS + num, 1);
  } else {
    if (com == "on" || com == "1") {
      if (!ws2812fx[numI].isRunning()) ws2812fx[numI].start();
      sendStatus(stateRGBS + num, 1);
    }
    if (com == "off" || com == "0") {
      if (ws2812fx[numI].isRunning()) ws2812fx[numI].stop();
      sendStatus(stateRGBS + num, 0);
    }
    if (com == "not") {

      sendStatus(stateRGBS + num, !state);
      if (state) {
        ws2812fx[numI].stop();
      }
      else {
        ws2812fx[numI].stop();
        ws2812fx[numI].start();
      }
    }
  }
  statusS = htmlStatus(configJson, stateRGBS + num, langOnS, langOffS);
}

uint16_t convertSpeed(uint8_t mcl_speed) {
  //long ws2812_speed = mcl_speed * 256;
  uint16_t ws2812_speed = 61760 * (exp(0.0002336 * mcl_speed) - exp(-0.03181 * mcl_speed));
  ws2812_speed = SPEED_MAX - ws2812_speed;
  if (ws2812_speed < SPEED_MIN) {
    ws2812_speed = SPEED_MIN;
  }
  if (ws2812_speed > SPEED_MAX) {
    ws2812_speed = SPEED_MAX;
  }
  return ws2812_speed;
}
uint16_t myCustomEffect0(void) { // random chase
  WS2812FX::Segment* seg = ws2812fx[0].getSegment(); // get the current segment
  int pixels = pgm_read_byte_near(data + 0);
  int frames = pgm_read_byte_near(data + 1);
  int fps = pgm_read_byte_near(data + 2);
  static  int currentFrame = 0;

  for (int i = seg->stop; i > seg->start; i--) {
    int pixelIndex = i % pixels;
    int index = currentFrame * pixels * 3 + pixelIndex * 3;

    //Note: We're using pgm_read_byte_near to read bytes out of the data array stored in PROGMEM. These functions are not required for all configurations
    ws2812fx[0].setPixelColor(i, pgm_read_byte_near(data + index + 3), pgm_read_byte_near(data + index + 4), pgm_read_byte_near(data + index + 5));
  }
  currentFrame ++;
  if (currentFrame >= frames) currentFrame = 0;
  return seg->speed; // return the delay until the next animation step (in msec)
}

uint16_t myCustomEffect1(void) { // random chase
  WS2812FX::Segment* seg = ws2812fx[1].getSegment(); // get the current segment
  int pixels = pgm_read_byte_near(data + 0);
  int frames = pgm_read_byte_near(data + 1);
  int fps = pgm_read_byte_near(data + 2);
  static  int currentFrame = 0;

  for (int i = seg->stop; i > seg->start; i--) {
    int pixelIndex = i % pixels;
    int index = currentFrame * pixels * 3 + pixelIndex * 3;

    //Note: We're using pgm_read_byte_near to read bytes out of the data array stored in PROGMEM. These functions are not required for all configurations
    ws2812fx[1].setPixelColor(i, pgm_read_byte_near(data + index + 3), pgm_read_byte_near(data + index + 4), pgm_read_byte_near(data + index + 5));
  }
  currentFrame ++;
  if (currentFrame >= frames) currentFrame = 0;
  return seg->speed; // return the delay until the next animation step (in msec)
}
#endif
