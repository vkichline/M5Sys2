#include <vector>
#include <M5Sys2.h>
using namespace std;

// Determine what minimum button size works for you, or the stylus you're using.
// Starts with large sized buttons, user can adjust size and spacing.
// Asks the user to touch eight random squares in each resolution.
// Keeps score and helps determine minimum button size and spacing.
//
// To use: Start app; 60 X 60 buttons with no spacing are displayed.
// Touch all 6 red buttons w/o touching anything else on the screen.
// When complete, another tests starts right away.
// To change configuration, A button switches between width, height and spacing.
// B button increases property selected by A button (up to maximum)
// C button reduces property select by A button (down to minimum)
// Scores from each test are written in Serial in json-like syntax.
// To convert output to json, just wrap it in {} or [].
//
// By Van Kichline
// In the year of the plague


#define SCREEN_TOP      20
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   200
#define BANNER_HEIGHT   20
#define MAX_WIDTH       80
#define MAX_HEIGHT      MAX_WIDTH
#define MIN_WIDTH       20
#define MIN_HEIGHT      MIN_WIDTH
#define NUM_ACTIVE      6


typedef struct Score {
  char          start[20];
  int16_t       height;
  int16_t       width;
  int16_t       spacing;
  int16_t       hits;
  int16_t       misses;
  float         seconds;
} Score;


TFT_eSprite     banner(&M5.Lcd);
ButtonColors    offColors       = {BLUE, WHITE, WHITE};
ButtonColors    onColors        = {BLUE, WHITE, WHITE};
int16_t         height          = 60;
int16_t         width           = 60;
int16_t         spacing         = 0;
int16_t         hits            = 0;
int16_t         misses          = 0;
const char*     modes[]         = { "Width", "Height", "Spacing" };
const int16_t   max_mode        = ((sizeof(modes) / (sizeof(char*))) - 1);
uint8_t         mode            = 0;
bool            test_starting   = false;
char            start_time[20];
uint32_t        start_ticks;


// Draw the button size and hit/miss total at top of screen
//
void draw_info() {
  VERBOSE("draw_info()\n");
  char buffer[64];
  banner.fillRect(0, 0, 320, BANNER_HEIGHT, BLACK);
  sprintf(buffer, "Width: %d,  Height: %d,  Spacing: %d", width, height, spacing);
  banner.drawString(buffer, 2, 2, 2);
  sprintf(buffer, "%d/%d", hits, misses);
  banner.drawRightString(buffer, 318, 2, 2);
  banner.pushSprite(0, 0);
}


// Draw the button lables at the bottom of the screen
//
void draw_labels() {
  VERBOSE("draw_labels()\n");
  banner.fillRect(0, 0, SCREEN_WIDTH, BANNER_HEIGHT, BLACK);
  banner.setTextColor(WHITE, BLACK);
  banner.drawCentreString(modes[mode], 54, 322, 2);
  banner.drawCentreString("+", 160, 0, 4);
  banner.drawCentreString("-", 266, 0, 4);
  banner.pushSprite(0, 240 - BANNER_HEIGHT);
}


// Make a string from date/time in the format: YYYY/MM/DD HH:MM:SS
// Exactly 20 charaters long. Use global start_time as buffer.
//
void store_start_time() {
  VERBOSE("store_start_time()\n");
  RTC_DateTypeDef date;
  RTC_TimeTypeDef time;
  start_ticks = millis();
  M5.Rtc.GetData(&date);
  M5.Rtc.GetTime(&time);
  snprintf(start_time, 20, "%4d/%02d/%02d %02d:%02d:%02d", date.Year, date.Month, date.Date, time.Hours, time.Minutes, time.Seconds);
}


// Touch Event Handler
//
void button_pressed(Event& e) {
  VERBOSE("button_pressed(Event&)\n");
  Button& button = *e.button;

  if(test_starting) {
    test_starting = false;
    store_start_time();   // Consider the test started when the first touch occurs.
  }

  if(0 == strcmp(button.getName(), "background")) {
    misses++;
  }
  else if(1 == button.userData) {
    hits++;
    button.off = {BLUE, WHITE, WHITE};
    button.userData = 0;
    button.draw();
  }
  else {
    misses++;
    button.off = {BLACK, WHITE, WHITE};
    button.draw();
  }
  draw_info();
}


// Return how many buttons still have their userData set to 1.
//
int16_t active_count(vector<vector<Button*>> buttons) {
  // VERBOSE("active_count(buttons)\n"); Too frequent to monitor
  int16_t result = 0;
  for(auto i = buttons.begin(); i != buttons.end(); ++i)
    for(auto j = i->begin(); j != i->end(); ++j)
        if(1 == (*j)->userData) result++;
  return result;
}


// M5.update() has already been called. Any changes in settings?
// Return true if settings changed, not if category changed.
//
bool test_abc() {
  // VERBOSE("test_abc()\n"); Too frequent to monitor
  if(M5.BtnA.wasPressed()) {
    mode++;
    if(mode > max_mode) mode = 0;
    draw_labels();
    return false;
  }
  if(M5.BtnB.wasPressed()) {
    bool changed = false;
    switch(mode) {
      case 0:
        if(++width > MAX_WIDTH) width = MAX_WIDTH;
        else changed = true;
        break;
      case 1:
        if(++height > MAX_HEIGHT) height = MAX_HEIGHT;
        else changed = true;
        break;
      case 2:
        spacing++;
        changed = true;
        break;
    }
    draw_info();
    return changed;
  }
  if(M5.BtnC.wasPressed()) {
    bool changed = false;
    switch(mode) {
      case 0:
        if(--width < MIN_WIDTH) width = MIN_WIDTH;
        else changed = true;
        break;
      case 1:
        if(--height < MIN_HEIGHT) height = MIN_HEIGHT;
        else changed = true;
        break;
      case 2:
        spacing--;
        if(spacing < 0) spacing = 0;
        else changed = true;
    }
    draw_info();
    return changed;
  }
  return false;
}


// Return true if the test was completed w/o interruption
//
bool take_test(vector<vector<Button*>> buttons) {
  VERBOSE("take_test(buttons)\n");
  int16_t rows  = buttons.size();     // Number of rows in the 2D vector
  int16_t cols  = buttons[0].size();  // assumes 2D vector is rectangular
  int16_t count = rows * cols;        // Total number of buttons

  DEBUG("rows = %d, cols = %d, count = %d\n", rows, cols, count);
  hits = misses = 0;
  int16_t num = min(NUM_ACTIVE, count);
  draw_info();
  while(num > active_count(buttons)) {
    Button* button   = buttons[random(rows)][random(cols)];
    button->userData = 1;
    button->off = {RED, WHITE, WHITE};
  }
  M5.Buttons.draw();
  test_starting = true;
  while(active_count(buttons)) {
    M5.update();
    if(test_abc()) return false;  // User changed settings; forget about it.
    delay(10);
  }
  delay(500);
  return true;
}


// Get the scores for the session just completed. Write scores to serial port,
// and return a structure for good measure.
// (Nothing is done with the structure at this point, may be handy in the future.)
//
void collect_score(Score& score) {
  VERBOSE("collect_score(Score)\n");
  score.seconds = (float)(millis() - start_ticks) / 1000.0;
  strncpy(score.start, start_time, 20);
  score.height  = height;
  score.width   = width;
  score.spacing = spacing;
  score.hits    = hits;
  score.misses  = misses;
  INFO("{ \"date\": \"%s\", \"width\": %2d, \"height\": %2d, \"spacing\": %2d, \"seconds\": %2.2f, \"hits\": %d, \"misses\": %d }\n",
       score.start, score.width, score.height, score.spacing, score.seconds, score.hits, score.misses);
}


// Create a regular 2D vector of as many buttons as fit the display area..
//
vector<vector<Button*>> create_buttons(int16_t h_pixels, int16_t v_pixels, int16_t spacing = 0) {
  VERBOSE("create_buttons(%d, %d, %d)\n", h_pixels, v_pixels, spacing);
  vector<vector<Button*>> buttons;
  uint8_t h_count   = (int16_t)((SCREEN_WIDTH  + spacing) / (h_pixels + spacing));
  uint8_t v_count   = (int16_t)((SCREEN_HEIGHT + spacing) / (v_pixels + spacing));
  uint8_t h_offset  = (SCREEN_WIDTH  - ((h_pixels + spacing) * h_count) + spacing) / 2;
  uint8_t v_offset  = (SCREEN_HEIGHT - ((v_pixels + spacing) * v_count) + spacing) / 2;
  DEBUG("h_count = %d, v_count = %d, h_offset = %d, v_offset = %d\n", h_count, v_count, h_offset, v_offset);

  for(uint8_t v = 0; v < v_count; v++) {
    vector<Button*> row;
    for(uint8_t h = 0; h < h_count; h++) {
      Button* button = new Button(h_offset + (h * (h_pixels + spacing)),
                                  SCREEN_TOP + v_offset + (v * (v_pixels + spacing)),
                                  h_pixels, v_pixels, false, "", offColors, onColors, MC_DATUM, 0, 0, 0);
      button->userData = 0;
      button->addHandler(button_pressed, E_TOUCH);
      row.push_back(button);
      DEBUG("New button: x = %3d, y = %3d, w = %2d, h = %2d\n", button->x, button->y, button->w, button->h);
    }
    buttons.push_back(row);
  }
  DEBUG("Button::instances length = %d\n\n", Button::instances.size());
  M5.Buttons.draw();
  return buttons;
}


// Delete every button in the regular 2D vector.
//
void delete_buttons(vector<vector<Button*>> buttons) {
  VERBOSE("delete_buttons(buttons)\n");
  for(auto i = buttons.begin(); i != buttons.end(); ++i)
    for(auto j = i->begin(); j != i->end(); ++j)
        delete *j;
}


// Arduino startup routine, called once at initialization
//
void setup() {
  sys.begin("ButtonSizeTest");
  banner.createSprite(SCREEN_WIDTH, BANNER_HEIGHT);
  M5.background.addHandler(button_pressed, E_TOUCH);
}


// Arduino loop routine. Run one test per loop.
//
void loop() {
  VERBOSE("loop()\n");
  Score score;
  M5.Lcd.fillRect(0, SCREEN_TOP, SCREEN_WIDTH, SCREEN_HEIGHT, BLUE);
  draw_info();
  draw_labels();
  auto buttons  = create_buttons(width, height, spacing);
  if(take_test(buttons)) collect_score(score);
  delete_buttons(buttons);
}
