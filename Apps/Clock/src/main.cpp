// A simple (but free to grow complex) Clock display program for M5Stack Core2

#include "Clock.h"
#include "BaseRenderer.h"
#include "TextClock.h"
#include "WallClock.h"


const ColorCombo    colors[]          = { {WHITE, 0x0004}, {RED, BLACK}, {BLACK, WHITE} };
const uint8_t       num_colors        = sizeof(colors) / sizeof(ColorCombo);
uint8_t             cur_color         = 0;

BaseRenderer*       renderers[]       = { new TextClock(), new WallClock() };
uint8_t             num_renderers     = sizeof(renderers) / sizeof(BaseRenderer*);
uint8_t             cur_renderer      = 0;
String              button_string     = "color # back | home # design";
unsigned long       show_time         = 0;


// Set the colors, timezone (if synchronized).
// Call this every time settings are changed.
//
void use_settings() {
  ez.buttons.show("");
  ez.screen.clear(colors[cur_color].bg_color);
  ez.buttons.show(button_string);
  show_time = millis();
  renderers[cur_renderer]->draw_maximum();
}


// If more than HIDE_BUTTON_MS have passed since the buttons were shown,
// hide them. show_time == 0 when buttons are hidden.
//
void check_hide_buttons() {
  VERBOSE("check_hide_buttons()\n");
  if(show_time) {
    if(HIDE_BUTTON_MS < millis() - show_time) {
      DEBUG("check_hide_buttons: hiding the buttons after timeout\n");
      show_time = 0;
      // Inexpensive wipe-down effect:
      for(uint16_t y = Y_HEIGHT; y < M5.Lcd.height(); y++) {
        M5.Lcd.drawFastHLine(0, y, M5.Lcd.width(), colors[cur_color].bg_color);
        delay(10);
      }
    }
  }
}


// If the buttons are hidden, if there's a touch anywhere, show the buttons
// and return true. Else return false.
//
bool check_show_buttons() {
  VERBOSE("check_show_buttons()\n");
  if(0 == show_time) {
    if(0 < M5.Touch.points) {
      DEBUG("check_show_buttons: making buttons visible in response to touch\n");
      ez.buttons.show("");
      ez.buttons.show(button_string);
      show_time = millis();
      return true;
    }
  }
  return false;
}


// Check to see if the A, B or C buttons have been pressed.
// If A, loop through the list of colors to change color settings
// If C, loop through the list of renderers to change the drawing routine
//
void check_for_buttons() {
  VERBOSE("check_for_buttons()\n");
  if(check_show_buttons()) return;
  bool  changed = false;
  String button_name = ez.buttons.poll();
  if(0 < button_name.length()) { DEBUG("button_name = %s\n", button_name.c_str()); }
  if(button_name == "color") {
    cur_color = (++cur_color >= num_colors) ? 0 : cur_color;
    DEBUG("changing cur_color to %d\n", cur_color);
    changed = true;
  }
  else if(button_name == "design") {
    cur_renderer = (++cur_renderer >= num_renderers) ? 0 : cur_renderer;
    DEBUG("changing cur_renderer to %d\n", cur_renderer);
    changed = true;
  }
  else if(button_name == "back") {
    ez.header.title("");  // Hack: I need to do this to get the box drawn more than one time.
    String response = ez.msgBox("Go Home", "Are you sure you want to go Home?", "Cancel ## OK");
    if(response == "OK") sys.load_home_program();
    changed = true;
  }
  if(changed) {
    use_settings();
  }
  check_hide_buttons();
}


void setup() {
  sys.begin("Clock");
  M5.Lcd.setTextFont(2);
  M5.Lcd.setTextSize(1);
  use_settings();
}


void loop() {
  check_for_buttons();
  renderers[cur_renderer]->draw_minimum();
  delay(250); // Call more than once per second for higher precision
}
