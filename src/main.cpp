#include <Arduino.h>
#include <Ethernet.h>
#include <GyverEncoder.h>
#include <LiquidCrystal_I2C.h>

#define CLK 2
#define DT 3
#define SW 5

struct {
  // Network
  bool fDHCP = 0;
  byte myIp[4] = {192, 168, 1, 177};

  uint8_t mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

  uint16_t protocolPortIn = 8000;

  // Pc connect
  bool fPcConnectEnable = 0;
  uint16_t pcPortIn = 3545, pcPortOut = 3546;

  // Protocol
  byte fProtocol = 0; // 0 - OSC, 1 - ArtNet

  // USER
  byte locMinute = 5;

  // EEPROM VER
  uint32_t version = 0;
} Setings;

const char main0[] = "Network";
const char main1[] = "Protocol";
const char main2[] = "Pc Linc";
const char main3[] = "DMX";

Encoder enc1(CLK, DT, SW);
LiquidCrystal_I2C lcd(0x27, 20, 4);

void locScreen();
bool initDHCP();
byte getIpLength();
byte getLengthInt(int value);

void setup() {
  enc1.setType(TYPE2);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(3, 0);
  lcd.print("DMX CONTROLLER");
  lcd.setCursor(5, 1);
  lcd.print("By KVESTEN");
  lcd.setCursor(5, 2);
  lcd.print("Loading...");
  lcd.setCursor(4, 3);
  lcd.print("Please wait.");
  if (!Setings.fDHCP) {
    Ethernet.begin(Setings.mac, Setings.myIp);
  } else {
    if (!initDHCP()) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Failed DHCP");
      delay(10000);
      Ethernet.begin(Setings.mac, Setings.myIp);
    }
  }

  lcd.clear();
  locScreen();
}

void loop() {
  static bool activeMenu = 0;
  static bool fLock = 0;
  static uint32_t locTime = 0;
  static bool updateLcd = 0;
  static byte selectedPoint = 0, menuListPosition = 0;

  enc1.tick();

  if (!activeMenu and enc1.isHolded()) {
    activeMenu = 1;
    locTime = millis();
    fLock = 0;
    updateLcd = 1;
    lcd.clear();
  }

  if (!fLock && (millis() - locTime > Setings.locMinute * 1000)) {
    activeMenu = 0;
    fLock = 1;
    locScreen();
  }

  if (activeMenu) {
    if (enc1.isTurn()) {
      locTime = millis();
      if (enc1.isRight())
        selectedPoint++;
      if (enc1.isLeft())
        selectedPoint--;

      switch (menuListPosition) {
      case 0:
        if (selectedPoint == 255)
          selectedPoint = 3;
        else if (selectedPoint > 3)
          selectedPoint = 0;
        break;
      }

      updateLcd = 1;
    }

    if (enc1.isClick()) {
      switch (menuListPosition) {
      case 0:
        menuListPosition = selectedPoint + 1;
        selectedPoint = 0;
        break;
      }
      updateLcd = 1;
    }

    if (enc1.isHolded()) {
      activeMenu = 0;
      locScreen();
    }
    if (updateLcd) {
      lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print("Settings");

      lcd.setCursor(0, 1 + selectedPoint % 3);
      lcd.print("->");

      switch (menuListPosition) {
      case 0:
        switch (selectedPoint / 3) {
        case 0:
          lcd.setCursor(4, 1);
          lcd.print(main0);
          lcd.setCursor(4, 2);
          lcd.print(main1);
          lcd.setCursor(4, 3);
          lcd.print(main2);
          break;
        case 1:
          lcd.setCursor(4, 1);
          lcd.print(main3);
        }
        break;
      }

      updateLcd = 0;
    }
  }
}

void locScreen() {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Hold to settings");
  lcd.setCursor(0, 1);
  lcd.print("Protocol:");
  switch (Setings.fProtocol) {
  case 0:
    lcd.setCursor(17, 1);
    lcd.print("OSC");
    break;
  case 1:
    lcd.setCursor(14, 1);
    lcd.print("ArtNet");
    break;
  }
  lcd.setCursor(0, 2);
  lcd.print("IP:");
  lcd.setCursor(20 - getIpLength(), 2);
  lcd.print(Setings.myIp[0]);
  for (size_t i = 1; i < 4; i++) {
    lcd.print(".");
    lcd.print(Setings.myIp[i]);
  }
  lcd.setCursor(0, 3);
  lcd.print("Port IN:");
  lcd.setCursor(20 - getLengthInt(Setings.protocolPortIn), 3);
  lcd.print(Setings.protocolPortIn);
}

bool initDHCP() {
  if (Ethernet.begin(Setings.mac) == 0) {
    return false;
  }
  for (size_t i = 0; i < 4; i++) {
    Setings.myIp[i] = Ethernet.localIP()[i];
  }
  return true;
}

byte getIpLength() {
  byte tmpLength = 3;
  for (size_t i = 0; i < 4; i++) {
    byte tmp = Setings.myIp[i];
    do {
      tmp /= 10;
      tmpLength++;
    } while (tmp);
  }
  return tmpLength;
}

byte getLengthInt(int value) {
  byte tmpLength = 0;
  do {
    value /= 10;
    tmpLength++;
  } while (value);
  return tmpLength;
}
