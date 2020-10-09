// Copyright (c) GWENDESIGN. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _GDTOUCHKEYBOARD_H_
#define _GDTOUCHKEYBOARD_H_

#include <M5Core2.h>

class GDTouchKeyboard
{
public:
  GDTouchKeyboard();
  ~GDTouchKeyboard();
  void setIdle(void(*idle)(void));
  String run(String text = "");
private:
  void (*idle)(void);
};

extern GDTouchKeyboard GDTK;

#endif // _GDTOUCHKEYBOARD_H_
