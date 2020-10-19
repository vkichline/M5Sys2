#include <M5Sys2.h>
#include "KeyCalculator.h"

#define       FG_COLOR        WHITE
#define       BG_COLOR        BLACK
#define       DISPLAY_HEIGHT  40


KeyCalculator calc;
Button        key[16];
Button        display(0, 0, M5.Lcd.width(), DISPLAY_HEIGHT, true, "0.0", {BLACK, WHITE, NODRAW}, {BLACK, WHITE, NODRAW}, TR_DATUM, 0, 0, 0);
ButtonColors  off_colors     = {BLACK, WHITE, WHITE};
ButtonColors  on_colors      = {WHITE, BLACK, WHITE};
const char*   key_labels[16] = {"7", "8", "9", "/",
                                "4", "5", "6", "*",
                                "1", "2", "3", "-",
                                "0", ".", "=", "+"};


void display_touched(Event& e) {
  if(E_DBLTAP == e) {
    calc.key(CLEAR_OPERATOR);
    display.setLabel("0.0");
    display.draw();
  }
}


void button_touched(Event& e) {
  if(E_TOUCH == e) {
    Button& button = *e.button;
    calc.key(button.userData);
    String disp_value = calc.get_display(dispValue);
    display.setLabel(disp_value.c_str());
    display.draw();
  }
}


void set_up_keyboard() {
  M5.Buttons.setFont(&FreeSansBold18pt7b);
  uint8_t margin = 6;
  uint16_t scr_w = M5.Lcd.width();
  uint16_t scr_h = M5.Lcd.height();
  uint8_t  btn_w = (scr_w / 4) - margin;
  uint8_t  btn_h = ((scr_h - DISPLAY_HEIGHT) / 4) - margin;

  display.w = scr_w;
  for (uint8_t r = 0; r < 4; r++) {
    for (uint8_t c = 0; c < 4; c++) {
      uint8_t i       = (r * 4) + c;
      key[i].setLabel(key_labels[i]);
      key[i].userData = key_labels[i][0];
      key[i].x        = c * (scr_w / 4) + (margin / 2);
      key[i].y        = r * ((scr_h - DISPLAY_HEIGHT) / 4) + (margin / 2) + DISPLAY_HEIGHT;
      key[i].w        = btn_w;
      key[i].h        = btn_h;
      key[i].off      = off_colors;
      key[i].on       = on_colors;
      key[i].dy       = -2;
    }
  }
  key[8].dx  = -2;   // Center the 1 better
  key[7].dy  =  6;   // Lower the '*' to center
  key[11].dy = -6;   // Raise the '-' to center
  key[13].dy = -12;  // Raise the '.' to center
  key[14].dy = -6;   // Raise the '=' to center
  key[15].dy = -6;   // Raise the '+' to center
  M5.Buttons.draw();
}


void setup() {
  sys.begin("TouchCalc");
  set_up_keyboard();
  for(int i = 0; i < 16; i++) key[i].addHandler(button_touched);
  display.addHandler(display_touched);
}


void loop() {
  M5.update();
}
