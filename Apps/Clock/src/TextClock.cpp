#include "TextClock.h"

#define   Y_LINE_1  20
#define   Y_LINE_1A 70
#define   Y_LINE_2  120
#define   Y_LINE_3  148
#define   Y_LINE_4  176


TextClock::TextClock() {
  VERBOSE("TextClock::TextClock()\n");
  time_width  = 0;
}


// When settings are changed, call this routine to redraw everything
//
void TextClock::draw_maximum() {
  VERBOSE("TextClock::draw_maximum()\n");
  last_date   = { 0, 0, 0, 0 };
  last_time   = { 0, 0, 0    };
  time_width  = 0;
  draw_minimum();
}


// The workhorse routine; draw as much of the time as needed directly to the screen.
// Flicker-free fonts make buffering unnecessary
//
void TextClock::draw_minimum() {
  VERBOSE("TextClock::draw_minimum()\n");
  RTC_TimeTypeDef   time;
  RTC_DateTypeDef   date;
  char              buffer[32];
  static bool       pm = false;
  bool              minute_changed = false;
  bool              second_changed = false;

  M5.Lcd.setTextColor(colors[cur_color].fg_color, colors[cur_color].bg_color);
  M5.Rtc.GetTime(&time);
  minute_changed = last_time.Minutes != time.Minutes || last_time.Hours != time.Hours;
  second_changed = last_time.Seconds != time.Seconds;
  last_time      = time;

  // Draw the hours and the minutes once a minute
  if(minute_changed) {
    DEBUG("updating hours/minutes\n");
    if(time.Hours >= 12) pm         = true;
    if(0 == time.Hours) time.Hours  = 12;   // Midnight is hour 0
    if(time.Hours > 12) time.Hours -= 12;   // 12 hour am/pm time
    sprintf(buffer, "%d:%02d", time.Hours, time.Minutes);

    // Special case, when time changes from 12:59:59 to 1:00:00, or from 9:59:59 to 10:00:00,
    // display width changes and erase is needed. It could be smaller. Needed?
    int16_t new_wid = M5.Lcd.textWidth(buffer, 8);
    if(time_width  != new_wid) {
      DEBUG("Erasing time background\n");
      M5.Lcd.fillRect(0, Y_LINE_1, X_WIDTH, 80, colors[cur_color].bg_color);
    }
    time_width = new_wid;
    DEBUG("time_width = %d\n", time_width);
    M5.Lcd.drawCentreString(buffer, 130, Y_LINE_1, 8);
  }

  // Draw the seconds whenever seconds change.
  if(second_changed) {
    VERBOSE("updating seconds\n");
    sprintf(buffer, "%02d", time.Seconds);
    M5.Lcd.drawString(buffer, 190 + time_width / 2, Y_LINE_1, 6);
    strcpy(buffer, pm ? "PM" : "AM");
    M5.Lcd.drawString(buffer, 180 + time_width / 2, Y_LINE_1A, 4);
  }

  // Draw the day specific info only once a day.
  // It too can expand or shrink. Erase edge-to-edge to eliminate artifacts.
  if(minute_changed) {
    M5.Rtc.GetData(&date);
    if(last_date.Date != date.Date) {
      DEBUG("updating day\n");
      last_date = date;
      String str = weekdays[date.WeekDay];
      M5.Lcd.drawCentreString(str, X_CENTER, Y_LINE_2, 4);

      str  = months[date.Month];
      str += " ";
      str += date.Date;
      switch(date.Date) {
        case 1:   str += "st"; break;
        case 2:   str += "nd"; break;
        case 3:   str += "rd"; break;
        default:  str += "th"; break;
      }
      M5.Lcd.drawCentreString(str,               X_CENTER, Y_LINE_3, 4);
      M5.Lcd.drawCentreString(String(date.Year), X_CENTER, Y_LINE_4, 4);
    }
  }
}
