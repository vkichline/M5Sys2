#pragma once

#include <M5Sys2.h>

#define X_WIDTH           320
#define X_CENTER          160
#define Y_HEIGHT          220
#define Y_CENTER          110
#define HIDE_BUTTON_MS    3000

struct ColorCombo {
  uint16_t  fg_color;
  uint16_t  bg_color;
};

// Globals that can be used by renderers
extern  const ColorCombo  colors[];
extern  const uint8_t     num_colors;
extern  uint8_t           cur_color;
