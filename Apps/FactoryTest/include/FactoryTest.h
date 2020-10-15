#pragma once

#include <M5Sys2.h>
#include <driver/i2s.h>
#include "fft.h"
#include <Fonts/EVA_20px.h>
#include <Fonts/EVA_11px.h>
#include <line3D.h>
#include <WiFi.h>

typedef struct i2cDevice {
  i2cDevice() {
    name    = "";
    addr    = 0;
    nextPtr = nullptr;
  };
  String            name;
  uint8_t           addr;
  struct i2cDevice* nextPtr;
} i2cDevice_t;

typedef enum system_power_t {
  kPOWER_EXTERNAL = 0,
  kPOWER_INTERNAL,
  kPOWER_MAX
} system_power_t;

typedef struct i2sQueueMsg_t {
  uint8_t   state;
  void*     audioPtr;
  uint32_t  audioSize;
} i2sQueueMsg_t;

typedef struct systemState {
  RTC_TimeTypeDef Rtctime;
  system_power_t  power                   = kPOWER_MAX;
  uint16_t        clockCount              = 0;
  uint16_t        timeCount               = 0;
  uint16_t        batCount                = 0;
  uint8_t         batVoltageBuff[15];
  uint8_t         batVoltageWriteptr      = 11;
  uint8_t         batVoltageReadptr       = 0;
  uint16_t        batVoltageWriteCount    = 0;
  bool            touchState              = false;
  bool            SDCardState             = false;
  uint16_t        SDCardscaneCount        = 0;
  HotZone_t*      App1Zone[6];
  HotZone_t*      MPU6886;
  bool            soundFlag               = false;
} systemState;

extern const unsigned char CoverImage[21301];
extern const unsigned char clockImage[18401];
extern const unsigned char CoreMainImage[87169];
extern const unsigned char batPowerImage[13769];
extern const unsigned char touchImage[12262];
extern const unsigned char SDCardImage[14835];
extern const unsigned char imageMenu[14900];
extern const unsigned char previewR[120264];
extern const unsigned char wifiScanImage[28123];
extern const unsigned char TimerAppImage[59165];
extern const unsigned char SettingAppImage[50683];
extern const unsigned char bibiSig[8820];

extern const unsigned char image_rect_0006[1394];
extern const unsigned char image_rect_0005[1394];
extern const unsigned char image_rect_0004[1394];
extern const unsigned char image_rect_0003[1394];
extern const unsigned char image_rect_0002[1394];
extern const unsigned char image_rect_0001[1394];

extern const unsigned char image_DigNumber_0000_0[504];
extern const unsigned char image_DigNumber_0001_1[504];
extern const unsigned char image_DigNumber_0002_2[504];
extern const unsigned char image_DigNumber_0003_3[504];
extern const unsigned char image_DigNumber_0004_4[504];
extern const unsigned char image_DigNumber_0005_5[504];
extern const unsigned char image_DigNumber_0006_6[504];
extern const unsigned char image_DigNumber_0007_7[504];
extern const unsigned char image_DigNumber_0008_8[504];
extern const unsigned char image_DigNumber_0009_9[504];
extern const unsigned char image_DigNumber_0010_10[504];

extern const unsigned char image_DigNumber_35px_0000_0[315];
extern const unsigned char image_DigNumber_35px_0001_1[315];
extern const unsigned char image_DigNumber_35px_0002_2[315];
extern const unsigned char image_DigNumber_35px_0003_3[315];
extern const unsigned char image_DigNumber_35px_0004_4[315];
extern const unsigned char image_DigNumber_35px_0005_5[315];
extern const unsigned char image_DigNumber_35px_0006_6[315];
extern const unsigned char image_DigNumber_35px_0007_7[315];
extern const unsigned char image_DigNumber_35px_0008_8[315];
extern const unsigned char image_DigNumber_35px_0009_9[315];
extern const unsigned char image_DigNumber_35px_0010_10[315];

extern const unsigned char image_rect320_20_0001[3200];
extern const unsigned char image_rect320_20_0002[3200];
extern const unsigned char image_rect320_20_0003[3200];
extern const unsigned char image_rect320_20_0004[3200];
extern const unsigned char image_rect320_20_0005[3200];
extern const unsigned char image_rect320_20_0006[3200];
extern const unsigned char image_rect320_20_0007[3200];

extern const unsigned char image_number8x7_01[35];
extern const unsigned char image_number8x7_02[35];
extern const unsigned char image_number8x7_03[35];
extern const unsigned char image_number8x7_04[35];
extern const unsigned char image_number8x7_05[35];
extern const unsigned char image_number8x7_06[35];
extern const unsigned char image_number8x7_07[35];
extern const unsigned char image_number8x7_08[35];
extern const unsigned char image_number8x7_09[35];
extern const unsigned char image_number8x7_10[35];
extern const unsigned char image_number8x7_11[35];
extern const unsigned char image_number8x7_12[35];
extern const unsigned char image_number8x7_13[35];
extern const unsigned char image_number8x7_14[35];
extern const unsigned char image_number8x7_15[35];
extern const unsigned char image_number8x7_16[35];

extern const unsigned char image_changing_0001[149];
extern const unsigned char image_changing_0002[149];
extern const unsigned char image_changing_0003[149];
extern const unsigned char image_changing_0004[149];
extern const unsigned char image_changing_0005[149];
extern const unsigned char image_changing_0006[149];
extern const unsigned char image_changing_0007[149];

extern const unsigned char image_Sysinit_0000s_0000_L1[875];
extern const unsigned char image_Sysinit_0000s_0001_L2[875];
extern const unsigned char image_Sysinit_0001s_0000_R1[875];
extern const unsigned char image_Sysinit_0001s_0001_R2[875];

extern const unsigned char image_TouchFish_0001[1672];
extern const unsigned char image_TouchFish_0002[1672];
extern const unsigned char image_TouchFish_0003[1672];
extern const unsigned char image_TouchFish_0004[1672];
extern const unsigned char image_TouchFish_0005[1672];
extern const unsigned char image_TouchFish_0006[1672];

extern const unsigned char image_External[1425];
extern const unsigned char image_Internal[1425];
extern const unsigned char image_PowerC[310];
extern const unsigned char image_power[924];

uint8_t *rectptrBuff[6] = {
  (uint8_t*)image_rect_0001,
  (uint8_t*)image_rect_0002,
  (uint8_t*)image_rect_0003,
  (uint8_t*)image_rect_0004,
  (uint8_t*)image_rect_0005,
  (uint8_t*)image_rect_0006,
};

uint8_t *rect320ptrBuff[7] = {
  (uint8_t*)image_rect320_20_0001,
  (uint8_t*)image_rect320_20_0002,
  (uint8_t*)image_rect320_20_0003,
  (uint8_t*)image_rect320_20_0004,
  (uint8_t*)image_rect320_20_0005,
  (uint8_t*)image_rect320_20_0006,
  (uint8_t*)image_rect320_20_0007,
};

uint8_t *DigNumber[11] = {
  (uint8_t*)image_DigNumber_0000_0,
  (uint8_t*)image_DigNumber_0001_1,
  (uint8_t*)image_DigNumber_0002_2,
  (uint8_t*)image_DigNumber_0003_3,
  (uint8_t*)image_DigNumber_0004_4,
  (uint8_t*)image_DigNumber_0005_5,
  (uint8_t*)image_DigNumber_0006_6,
  (uint8_t*)image_DigNumber_0007_7,
  (uint8_t*)image_DigNumber_0008_8,
  (uint8_t*)image_DigNumber_0009_9,
  (uint8_t*)image_DigNumber_0010_10,
};

uint8_t *DigNumber_35px[11] = {
  (uint8_t*)image_DigNumber_35px_0000_0,
  (uint8_t*)image_DigNumber_35px_0001_1,
  (uint8_t*)image_DigNumber_35px_0002_2,
  (uint8_t*)image_DigNumber_35px_0003_3,
  (uint8_t*)image_DigNumber_35px_0004_4,
  (uint8_t*)image_DigNumber_35px_0005_5,
  (uint8_t*)image_DigNumber_35px_0006_6,
  (uint8_t*)image_DigNumber_35px_0007_7,
  (uint8_t*)image_DigNumber_35px_0008_8,
  (uint8_t*)image_DigNumber_35px_0009_9,
  (uint8_t*)image_DigNumber_35px_0010_10,
};

uint8_t *Number_7x10px[16] = {
  (uint8_t*)image_number8x7_01, // 0
  (uint8_t*)image_number8x7_02, // 1
  (uint8_t*)image_number8x7_03, // 2
  (uint8_t*)image_number8x7_04, // 3
  (uint8_t*)image_number8x7_05, // 4
  (uint8_t*)image_number8x7_06, // 5
  (uint8_t*)image_number8x7_07, // 6
  (uint8_t*)image_number8x7_08, // 7
  (uint8_t*)image_number8x7_09, // 8
  (uint8_t*)image_number8x7_10, // 9
  (uint8_t*)image_number8x7_11, // V
  (uint8_t*)image_number8x7_12, // X
  (uint8_t*)image_number8x7_13, // Y
  (uint8_t*)image_number8x7_14, // :
  (uint8_t*)image_number8x7_15, // CHARGING
  (uint8_t*)image_number8x7_16, // G
};

uint8_t *batRect[7] = {
  (uint8_t*)image_changing_0001,
  (uint8_t*)image_changing_0002,
  (uint8_t*)image_changing_0003,
  (uint8_t*)image_changing_0004,
  (uint8_t*)image_changing_0005,
  (uint8_t*)image_changing_0006,
  (uint8_t*)image_changing_0007,
};

uint8_t *TouchFishBuff[7] = {
  (uint8_t*)image_TouchFish_0001,
  (uint8_t*)image_TouchFish_0002,
  (uint8_t*)image_TouchFish_0003,
  (uint8_t*)image_TouchFish_0004,
  (uint8_t*)image_TouchFish_0005,
  (uint8_t*)image_TouchFish_0006,
};
