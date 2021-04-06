/*
 * Pop'n Arcade Controller
 * https://github.com/it9gamelog/popncontroller
 *
 * MIT License
 * 
 * Copyright (c) 2020 IT9GameLog
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "NintendoSwitchController.h"
#include <EEPROM.h>

typedef enum {
  D1_TOP        = 1,
  D1_LEFT       = 2,
  D1_RIGHT      = 3,
  D1_BOTTOM     = A5,
  D2_TOP        = A4,
  D2_LEFT       = A3,
  D2_RIGHT      = A2,
  D2_BOTTOM     = A1,
  MASTERCON     = A0,
  MASTERCON_VCC = MOSI
} Button_Mappings;

const int SAVE_ADDRESS = 0;
struct SAVE_t {
  long stickMin;
  long stickMax;
} Save;

bool save_dirty = 0;
long save_cd = 0;

long capture_cd = 0;

long now = 0;

typedef enum {
  ACCEL_NORMAL = 0,
  ACCEL_103 = 1
} Accel_Modes;
Accel_Modes accel_mode = ACCEL_NORMAL;

void setup() {
  Serial.begin(9600);
  pinMode(D1_TOP, INPUT_PULLUP);
  pinMode(D1_LEFT, INPUT_PULLUP);
  pinMode(D1_RIGHT, INPUT_PULLUP);
  pinMode(D1_BOTTOM, INPUT_PULLUP);
  pinMode(D2_TOP, INPUT_PULLUP);
  pinMode(D2_LEFT, INPUT_PULLUP);
  pinMode(D2_RIGHT, INPUT_PULLUP);
  pinMode(D2_BOTTOM, INPUT_PULLUP);
  pinMode(MASTERCON, INPUT);
  pinMode(MASTERCON_VCC, OUTPUT);
  digitalWrite(MASTERCON_VCC, 1);
  EEPROM.get(SAVE_ADDRESS, Save);
  if (Save.stickMin > Save.stickMax || Save.stickMax == 0) {
    Save.stickMin = 512;
    Save.stickMax = 512;
  }
}

int8_t mapStickToDetent(long stick) {
  if (stick < 20) return -9;
  if (stick < 105) return -8;
  if (stick < 164) return -7;
  if (stick < 229) return -6;
  if (stick < 278) return -5;
  if (stick < 352) return -4;
  if (stick < 405) return -3;
  if (stick < 467) return -2;
  if (stick < 542) return -1;
  if (stick < 655) return 0;
  if (stick < 703) return 1;
  if (stick < 785) return 2;
  if (stick < 848) return 3;
  if (stick < 922) return 4;
  return 5;
}

int8_t mastercon_pos = 0;
uint8_t mastercon_at_eb = 0;

void loop() {
  now = millis();

  uint8_t hat = 0x0F & ~(
    digitalRead(D1_TOP) * 1 +
    digitalRead(D1_LEFT) * 2 +
    digitalRead(D1_RIGHT) * 4 +
    digitalRead(D1_BOTTOM) * 8);

  switch (hat) {
    case 1: NintendoSwitchController.setHat(HAT_TOP); break;
    case 2: NintendoSwitchController.setHat(HAT_LEFT); break;
    case 8: NintendoSwitchController.setHat(HAT_BOTTOM); break;
    case 4: NintendoSwitchController.setHat(HAT_RIGHT); break;
    default: NintendoSwitchController.setHat(HAT_CENTER); break;
  }
  
  if (digitalRead(D2_TOP)) NintendoSwitchController.release(SWITCH_X); else NintendoSwitchController.press(SWITCH_X);
  if (digitalRead(D2_LEFT)) { 
    NintendoSwitchController.release(SWITCH_Y); 
  } else { 
    NintendoSwitchController.press(SWITCH_Y);
    if (hat & 0x01) accel_mode = ACCEL_103;
  }
  if (digitalRead(D2_RIGHT)) NintendoSwitchController.release(SWITCH_A); else NintendoSwitchController.press(SWITCH_A);
  if (digitalRead(D2_BOTTOM)) {
    NintendoSwitchController.release(SWITCH_B);
  } else {
    NintendoSwitchController.press(SWITCH_B);
    if (hat & 0x01) accel_mode = ACCEL_NORMAL;
  }

  if (hat == (2|4)) NintendoSwitchController.press(SWITCH_L | SWITCH_R); else NintendoSwitchController.release(SWITCH_L | SWITCH_R);
  if (hat == (1|8)) NintendoSwitchController.press(SWITCH_HOME); else NintendoSwitchController.release(SWITCH_HOME);
  if (hat == (1|2)) NintendoSwitchController.press(SWITCH_L); else NintendoSwitchController.release(SWITCH_L);
  if (hat == (1|4)) NintendoSwitchController.press(SWITCH_R); else NintendoSwitchController.release(SWITCH_R);
  if (hat == (1|2|4)) NintendoSwitchController.press(SWITCH_PLUS); else NintendoSwitchController.release(SWITCH_PLUS);
  if (hat == (8|2)) { NintendoSwitchController.press(SWITCH_CAPTURE); capture_cd = now + 100; }
  if (hat == (8|4)) { NintendoSwitchController.press(SWITCH_CAPTURE); capture_cd = now + 500; }
  if (capture_cd - now < 0) NintendoSwitchController.release(SWITCH_CAPTURE);

  int stick = analogRead(MASTERCON);
  // Reverse the numbers
  // Might not need this if the potential meter VCC/GND is connected the other way
  stick = 1024 - stick; 
  
  if (hat == (1|2|4|8)) {    
    Save.stickMin = stick;
    Save.stickMax = stick;
  }
  if (stick > Save.stickMax) { Save.stickMax = stick; save_dirty = true; save_cd = now + 1000; }
  if (stick < Save.stickMin) { Save.stickMin = stick; save_dirty = true; save_cd = now + 1000; }
  if (save_dirty && save_cd - now < 0) {
    EEPROM.put(SAVE_ADDRESS, Save);
    save_dirty = 0;
  }
  
  stick = map(stick, Save.stickMin, Save.stickMax, 0, 1023);
  auto new_pos = mapStickToDetent(stick);
  if (new_pos <= -9) {
    if (mastercon_at_eb == 0) {
      NintendoSwitchController.press(SWITCH_ZL);
      NintendoSwitchController.setLy(0);
      NintendoSwitchController.setRx(255);
      mastercon_at_eb = 1;
    }
  } else {
    if (mastercon_at_eb) NintendoSwitchController.release(SWITCH_ZL);
    mastercon_at_eb = 0;
    if (new_pos < 0) {      
      uint8_t pos;
      if (accel_mode == ACCEL_NORMAL) {
        pos = map(new_pos, -8, -1, 0, 127-30);
      } else {
        if (pos < -7) pos = -7;
        pos = map(new_pos, -7, -1, 0, 127-30);
      }
      NintendoSwitchController.setRx(255 - pos);
      NintendoSwitchController.setLy(pos);
      NintendoSwitchController.setLx(128);
    } else if (new_pos > 0) {
      NintendoSwitchController.setRx(128);
      if (accel_mode == ACCEL_NORMAL) {
        NintendoSwitchController.setLy(map(new_pos, 1, 5, 127+30, 250));
      } else {
        NintendoSwitchController.setLx(map(new_pos, 1, 5, 127-30, 0));
      }
    } else {
      NintendoSwitchController.setRx(128);
      NintendoSwitchController.setLy(128);
      NintendoSwitchController.setLx(128);
    }
  }

  /*
  Serial.print(stick);
  Serial.print('\t');
  Serial.print(mapStickToDetent(stick));
  Serial.println('\t');
  */
    
  NintendoSwitchController.send();

  delay(20 + now - millis());
}
