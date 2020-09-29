#pragma once

#include "Clock.h"
#include "BaseRenderer.h"


class TextClock : public BaseRenderer {
  public:
    TextClock();
    void            draw_minimum();  // Call as often as possible, draws only what's needed
    void            draw_maximum();  // Call when settings change to redraw everything
  protected:
    String          weekdays[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
    String          months[12]  = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
    RTC_DateTypeDef last_date   = { 0, 0, 0, 0 };
    RTC_TimeTypeDef last_time   = { 0, 0, 0    };
    int16_t         time_width  = 0;
};
