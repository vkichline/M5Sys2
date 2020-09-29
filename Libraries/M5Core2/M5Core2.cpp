// Copyright (c) M5Core2. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "M5Core2.h"

M5Core2::M5Core2() : isInited(0) {
}

void M5Core2::begin(bool LCDEnable, bool SDEnable, bool SerialEnable, bool I2CEnable) {
  // Correct init once
  if (isInited == true) {
    return;
  } else {
    isInited = true;
  }

  // UART
  if (SerialEnable == true) {
    Serial.begin(115200);
    Serial.flush();
    delay(50);
    Serial.print("M5Core2 initializing...");
  }

// I2C init
  if (I2CEnable == true) {
    Wire.begin(32, 33);
  }

  Axp.begin();

  // LCD INIT
  if (LCDEnable == true) {
    Lcd.begin();
  }

  // Touch init
  Touch.begin(); // Touch begin after AXP begin. (Reset at the start of AXP)

  // TF Card
  if (SDEnable == true) {
    SD.begin(TFCARD_CS_PIN, SPI, 40000000);
  }

  // TONE
  // Speaker.begin();

  if (SerialEnable == true) {
    Serial.println("OK");
  }

  Rtc.begin();

}

void M5Core2::update() {
  Touch.read();
  yield();
}

M5Core2 M5;
