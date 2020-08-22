#ifdef irM
// ----------------------Приемник ИK
void irReceived() {
  byte pin = readArgsInt();
  pin =  pinTest(pin);
  irReceiver = new IRrecv(pin);  // Create a new IRrecv object. Change to what ever pin you need etc.
  irReceiver->enableIRIn(); // Start the receiver
  // задача опрашивать IR код
  ts.add(tIR, 100, [&](void*) {
    handleIrReceiv();
  }, nullptr, true);
  sendStatus(irReceivedS, "ffffffff");
  modulesReg(irReceivedS);
}
void handleIrReceiv() {
  if (irReceiver->decode(&results)) {
    dump(&results);
    flag = sendStatus(irReceivedS, uint64ToString(results.value, HEX));
    irReceiver->resume();  // Continue looking for IR codes after your finished dealing with the data.
  }
}
void dump(decode_results *results) {
  uint16_t count = results->rawlen;
  //Serial.println(count);
  sendOptions(irDecodeTypeS, results->decode_type);
}
#endif


//--------------------------------------------------------------------------------------------------------

#ifdef irTransmitterM
// ----------------------Передатчик ИK
void irTransmitter() {
  String moduleName = "irTransmitter";
  //  Serial.println(moduleName);
  byte pin = readArgsInt();
  pin =  pinTest(pin);
  irSender = new IRsend(pin);  // Create a new IRrecv object. Change to what ever pin you need etc.
  irSender->begin();
  sCmd.addCommand(irsendS.c_str(), handleIrTransmit);
  commandsReg(irsendS);
  modulesReg(moduleName);
}

// команда irsend
void handleIrTransmit() {
  uint8_t ir_type = readArgsInt(); // Тип сигнала
  String code_str = readArgsString(); // Код
  uint16_t repeat = readArgsInt();    // Повтор
  uint8_t bits = readArgsInt();       // Длинна кода
  uint32_t  code = strtoul(("0x" + code_str).c_str(), NULL, 0);
  //irSender->sendNEC(tmp, 32);
bool success = true;  // Assume success.
   // send the IR message.
  switch (ir_type) {
#if SEND_RC5
    case RC5:  // 1
      if (bits == 0)
        bits = kRC5Bits;
      irSender->sendRC5(code, bits, repeat);
      break;
#endif
#if SEND_RC6
    case RC6:  // 2
      if (bits == 0)
        bits = kRC6Mode0Bits;
      irSender->sendRC6(code, bits, repeat);
      break;
#endif
#if SEND_NEC
    case NEC:  // 3
      if (bits == 0)
        bits = kNECBits;
      irSender->sendNEC(code, bits, repeat);
      break;
#endif
#if SEND_SONY
    case SONY:  // 4
      if (bits == 0)
        bits = kSony12Bits;
      repeat = std::max(repeat, kSonyMinRepeat);
      irSender->sendSony(code, bits, repeat);
      break;
#endif
#if SEND_PANASONIC
    case PANASONIC:  // 5
      if (bits == 0)
        bits = kPanasonicBits;
      irSender->sendPanasonic64(code, bits, repeat);
      break;
#endif
#if SEND_JVC
    case JVC:  // 6
      if (bits == 0)
        bits = kJvcBits;
      irSender->sendJVC(code, bits, repeat);
      break;
#endif
#if SEND_SAMSUNG
    case SAMSUNG:  // 7
      if (bits == 0)
        bits = kSamsungBits;
      irSender->sendSAMSUNG(code, bits, repeat);
      break;
#endif
#if SEND_WHYNTER
    case WHYNTER:  // 8
      if (bits == 0)
        bits = kWhynterBits;
      irSender->sendWhynter(code, bits, repeat);
      break;
#endif
#if SEND_AIWA_RC_T501
    case AIWA_RC_T501:  // 9
      if (bits == 0)
        bits = kAiwaRcT501Bits;
      repeat = std::max(repeat, kAiwaRcT501MinRepeats);
      irSender->sendAiwaRCT501(code, bits, repeat);
      break;
#endif
#if SEND_LG
    case LG:  // 10
      if (bits == 0)
        bits = kLgBits;
      irSender->sendLG(code, bits, repeat);
      break;
#endif
#if SEND_MITSUBISHI
    case MITSUBISHI:  // 12
      if (bits == 0)
        bits = kMitsubishiBits;
      repeat = std::max(repeat, kMitsubishiMinRepeat);
      irSender->sendMitsubishi(code, bits, repeat);
      break;
#endif
#if SEND_DISH
    case DISH:  // 13
      if (bits == 0)
        bits = kDishBits;
      repeat = std::max(repeat, kDishMinRepeat);
      irSender->sendDISH(code, bits, repeat);
      break;
#endif
#if SEND_SHARP
    case SHARP:  // 14
      if (bits == 0)
        bits = kSharpBits;
      irSender->sendSharpRaw(code, bits, repeat);
      break;
#endif
#if SEND_COOLIX
    case COOLIX:  // 15
      if (bits == 0)
        bits = kCoolixBits;
      irSender->sendCOOLIX(code, bits, repeat);
      break;
#endif
    case DAIKIN:  // 16
    case DAIKIN2:  // 53
    case KELVINATOR:  // 18
    case MITSUBISHI_AC:  // 20
    case GREE:  // 24
    case ARGO:  // 27
    case TROTEC:  // 28
    case TOSHIBA_AC:  // 32
    case FUJITSU_AC:  // 33
    case HAIER_AC:  // 38
    case HAIER_AC_YRW02:  // 44
    case HITACHI_AC:  // 40
    case HITACHI_AC1:  // 41
    case HITACHI_AC2:  // 42
    case WHIRLPOOL_AC:  // 45
    case SAMSUNG_AC:  // 46
    case ELECTRA_AC:  // 48
    case PANASONIC_AC:  // 49
    case MWM:  // 52
//      success = parseStringAndSendAirCon(ir_type, code_str);
      break;
#if SEND_DENON
    case DENON:  // 17
      if (bits == 0)
        bits = DENON_BITS;
      irSender->sendDenon(code, bits, repeat);
      break;
#endif
#if SEND_SHERWOOD
    case SHERWOOD:  // 19
      if (bits == 0)
        bits = kSherwoodBits;
      repeat = std::max(repeat, kSherwoodMinRepeat);
      irSender->sendSherwood(code, bits, repeat);
      break;
#endif
#if SEND_RCMM
    case RCMM:  // 21
      if (bits == 0)
        bits = kRCMMBits;
      irSender->sendRCMM(code, bits, repeat);
      break;
#endif
#if SEND_SANYO
    case SANYO_LC7461:  // 22
      if (bits == 0)
        bits = kSanyoLC7461Bits;
      irSender->sendSanyoLC7461(code, bits, repeat);
      break;
#endif
#if SEND_RC5
    case RC5X:  // 23
      if (bits == 0)
        bits = kRC5XBits;
      irSender->sendRC5(code, bits, repeat);
      break;
#endif
#if SEND_PRONTO
    case PRONTO:  // 25
//      success = parseStringAndSendPronto(code_str, repeat);
      break;
#endif
#if SEND_NIKAI
    case NIKAI:  // 29
      if (bits == 0)
        bits = kNikaiBits;
      irSender->sendNikai(code, bits, repeat);
      break;
#endif
#if SEND_RAW
    case RAW:  // 30
//      success = parseStringAndSendRaw(code_str);
      break;
#endif
#if SEND_GLOBALCACHE
    case GLOBALCACHE:  // 31
//      success = parseStringAndSendGC(code_str);
      break;
#endif
#if SEND_MIDEA
    case MIDEA:  // 34
      if (bits == 0)
        bits = kMideaBits;
      irSender->sendMidea(code, bits, repeat);
      break;
#endif
#if SEND_MAGIQUEST
    case MAGIQUEST:  // 35
      if (bits == 0)
        bits = kMagiquestBits;
      irSender->sendMagiQuest(code, bits, repeat);
      break;
#endif
#if SEND_LASERTAG
    case LASERTAG:  // 36
      if (bits == 0)
        bits = kLasertagBits;
      irSender->sendLasertag(code, bits, repeat);
      break;
#endif
#if SEND_CARRIER_AC
    case CARRIER_AC:  // 37
      if (bits == 0)
        bits = kCarrierAcBits;
      irSender->sendCarrierAC(code, bits, repeat);
      break;
#endif
#if SEND_MITSUBISHI2
    case MITSUBISHI2:  // 39
      if (bits == 0)
        bits = kMitsubishiBits;
      repeat = std::max(repeat, kMitsubishiMinRepeat);
      irSender->sendMitsubishi2(code, bits, repeat);
      break;
#endif
#if SEND_GICABLE
    case GICABLE:  // 43
      if (bits == 0)
        bits = kGicableBits;
      repeat = std::max(repeat, kGicableMinRepeat);
      irSender->sendGICable(code, bits, repeat);
      break;
#endif
#if SEND_LUTRON
    case LUTRON:  // 47
      if (bits == 0)
        bits = kLutronBits;
      irSender->sendLutron(code, bits, repeat);
      break;
#endif
#if SEND_PIONEER
    case PIONEER:  // 50
      if (bits == 0)
        bits = kPioneerBits;
      irSender->sendPioneer(code, bits, repeat);
      break;
#endif

#if SEND_LG
    case LG2:  // 51
      if (bits == 0)
        bits = kLgBits;
      irSender->sendLG2(code, bits, repeat);
      break;
#endif
    default:
      // If we got here, we didn't know how to send it.
      success = false;
  }
}

#endif
