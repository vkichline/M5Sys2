
#include <FactoryTest.h>

#define FAILED_COLOR                      255,  35,  35
#define SUCCESS_COLOR                     255, 255, 255
#define CONFIG_I2S_BCK_PIN                12
#define CONFIG_I2S_LRCK_PIN               0
#define CONFIG_I2S_DATA_PIN               2
#define CONFIG_I2S_DATA_IN_PIN            34
#define SPEAKER_I2S_NUMBER                I2S_NUM_0
#define MODE_MIC                          0
#define MODE_SPK                          1



static QueueHandle_t  fft_value_queue     = nullptr;
static QueueHandle_t  i2s_state_queue     = nullptr;

i2cDevice_t i2cParentptr;
systemState sysState;
uint16_t    imageCount                    = 0;
uint16_t    timecount                     = 0;
TFT_eSprite disp_buff                     = TFT_eSprite(&M5.Lcd);
TFT_eSprite disp_3d_buff                  = TFT_eSprite(&M5.Lcd);
TFT_eSprite disp_clock_buff               = TFT_eSprite(&M5.Lcd);
TFT_eSprite disp_power_buff               = TFT_eSprite(&M5.Lcd);
TFT_eSprite disp_fft_buff                 = TFT_eSprite(&M5.Lcd);
TFT_eSprite disp_bat_buff                 = TFT_eSprite(&M5.Lcd);
TFT_eSprite disp_touch_buff               = TFT_eSprite(&M5.Lcd);
TFT_eSprite disp_sd_card_buff             = TFT_eSprite(&M5.Lcd);
TFT_eSprite disp_warning_buff             = TFT_eSprite(&M5.Lcd);
TFT_eSprite disp_cover_scroll_buff        = TFT_eSprite(&M5.Lcd);
TFT_eSprite menu_buff                     = TFT_eSprite(&M5.Lcd);
Line_3d_t   rect_source[12];
Line_3d_t   rect[12]                      = {
              { .start_point = {-1, -1,  1}, .end_point = { 1, -1,  1} },
              { .start_point = { 1, -1,  1}, .end_point = { 1,  1,  1} },
              { .start_point = { 1,  1,  1}, .end_point = {-1,  1,  1} },
              { .start_point = {-1,  1,  1}, .end_point = {-1, -1,  1} },
              { .start_point = {-1, -1,  1}, .end_point = {-1, -1, -1} },
              { .start_point = { 1, -1,  1}, .end_point = { 1, -1, -1} },
              { .start_point = { 1,  1,  1}, .end_point = { 1,  1, -1} },
              { .start_point = {-1,  1,  1}, .end_point = {-1,  1, -1} },
              { .start_point = {-1, -1, -1}, .end_point = { 1, -1, -1} },
              { .start_point = { 1, -1, -1}, .end_point = { 1,  1, -1} },
              { .start_point = { 1,  1, -1}, .end_point = {-1,  1, -1} },
              { .start_point = {-1,  1, -1}, .end_point = {-1, -1, -1} }
            };
HotZone_t   touch_btn_0( 10, 241, 120, 280);  // 40-80-119
HotZone_t   touch_btn_1(130, 241, 200, 280);  // 130-160-200
HotZone_t   touch_btn_2(230, 241, 310, 280);  // 230-270-310

void set_check_state(int number, bool state, bool flush = false);


bool init_i2s_speaker_or_mic(int mode) {
  VERBOSE("init_i2s_speaker_or_mic(%d)\n", mode);
  esp_err_t err = ESP_OK;
  i2s_driver_uninstall(SPEAKER_I2S_NUMBER);
  i2s_config_t i2s_config = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER),
    .sample_rate          = 44100,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
    .channel_format       = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 2,
    .dma_buf_len          = 128,
  };
  if(mode == MODE_MIC) {
    i2s_config.mode       = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM);
  }
  else {
    i2s_config.mode               = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
    i2s_config.use_apll           = false;
    i2s_config.tx_desc_auto_clear = true;
  }
  DEBUG("Init i2s_driver_install\n");
  err += i2s_driver_install(SPEAKER_I2S_NUMBER, &i2s_config, 0, NULL);
  i2s_pin_config_t tx_pin_config;

  tx_pin_config.bck_io_num   = CONFIG_I2S_BCK_PIN;
  tx_pin_config.ws_io_num    = CONFIG_I2S_LRCK_PIN;
  tx_pin_config.data_out_num = CONFIG_I2S_DATA_PIN;
  tx_pin_config.data_in_num  = CONFIG_I2S_DATA_IN_PIN;

  DEBUG("Init i2s_set_pin\n");
  err += i2s_set_pin(SPEAKER_I2S_NUMBER, &tx_pin_config);
  DEBUG("Init i2s_set_clk\n");
  err += i2s_set_clk(SPEAKER_I2S_NUMBER, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
  return true;
}


void cover_scroll_text(String strNext, uint32_t color) {
  VERBOSE("cover_scroll_text(%s, %d)\n", strNext.c_str(), color);
  static String strLast;
  uint32_t colorLast = 0xffff;
  uint32_t bkColor16 = disp_cover_scroll_buff.color565(0x22, 0x22, 0x22);
  disp_cover_scroll_buff.setFreeFont(&EVA_20px);
  disp_cover_scroll_buff.setTextSize(1);
  disp_cover_scroll_buff.setTextColor(disp_buff.color565(255, 0, 0), bkColor16);
  disp_cover_scroll_buff.setTextDatum(TC_DATUM);
  disp_cover_scroll_buff.fillRect(0, 0, 320, 60, bkColor16);
  disp_cover_scroll_buff.setTextColor(color);
  for(int i = 0; i < 20 ; i++) {
    disp_cover_scroll_buff.fillRect(0, 20, 320, 20, bkColor16);
    disp_cover_scroll_buff.setTextColor(colorLast);
    disp_cover_scroll_buff.drawString(strLast, 160, 20 - i);
    disp_cover_scroll_buff.setTextColor(color);
    disp_cover_scroll_buff.drawString(strNext, 160, 40 - i);
    disp_cover_scroll_buff.fillRect(0,  0, 320, 20, bkColor16);
    disp_cover_scroll_buff.fillRect(0 ,40, 320, 20, bkColor16);
    delay(5);
    disp_cover_scroll_buff.pushSprite(0, 150);
  }
  strLast   = strNext;
  colorLast = color;
}


void sys_error_skip() {
  VERBOSE("sys_error_skip()\n");
  uint32_t bkColor16 = disp_buff.color565(0x22, 0x22, 0x22);
  disp_buff.setFreeFont(&EVA_20px);
  disp_buff.setTextSize(1);
  disp_buff.setTextColor(disp_buff.color565(0xff, 0, 0), bkColor16);
  disp_buff.setTextDatum(TC_DATUM);
  disp_buff.pushInSprite(&disp_cover_scroll_buff, 0, 0, 320, 60, 0, 150);
  disp_buff.drawString("Touch to Skip", 160, 200);
  disp_buff.pushSprite(0, 0);
  M5.Axp.SetLDOEnable(3, false);

  HotZone_t touchZone(0, 0, 320, 280);
  while(true) {
    if(M5.Touch.ispressed()) {
      TouchPoint_t point = M5.Touch.getPressPoint();
      if(touchZone.inHotZone(point)) break;
    }
    delay(10);
  }
}


void add_i2c_device(String name, uint8_t addr) {
  VERBOSE("add_i2c_device(%s, %d\n", name.c_str(), addr);
  i2cDevice_t *lastptr = &i2cParentptr;
  while(lastptr->nextPtr != nullptr) {
    lastptr = lastptr->nextPtr;
  }
  i2cDevice_t* ptr = ( i2cDevice_t * )calloc(1,sizeof(i2cDevice_t));
  ptr->name = name;
  ptr->addr = addr;
  ptr->nextPtr = nullptr;
  lastptr->nextPtr = ptr;
}


int check_psram() {
  VERBOSE("check_psram()\n");
  uint8_t* testbuff = ( uint8_t* )ps_calloc( 100 * 1024 , sizeof(uint8_t));
  if(testbuff == nullptr) {
    cover_scroll_text("PSRAM malloc failed", M5.Lcd.color565(FAILED_COLOR));
    sys_error_skip();
    return -1;
  }
  else {
    cover_scroll_text("PSRAM malloc Successful", M5.Lcd.color565(SUCCESS_COLOR));
  }
  delay(100);

  for(size_t i = 0; i < 102400; i++) {
    testbuff[i] = 0xA5;
    if(testbuff[i] != 0xA5) {
      cover_scroll_text("PSRAM read failed", M5.Lcd.color565(FAILED_COLOR));
      sys_error_skip();
      return -1;
    }
  }
  cover_scroll_text("PSRAM W&R Successful", M5.Lcd.color565(SUCCESS_COLOR));
  return 0;
}


int check_i2c_addr() {
  VERBOSE("check_i2c_addr()\n");
  uint8_t count = 0;
  i2cDevice_t *lastptr = &i2cParentptr;
  do {
    lastptr = lastptr->nextPtr;
    Serial.printf("Addr:0x%02X - Name:%s\r\n", lastptr->addr, lastptr->name.c_str());
    Wire1.beginTransmission(lastptr->addr);
    if(Wire1.endTransmission() == 0) {
      String log = "I2C " + lastptr->name + " Found";
      cover_scroll_text(log, M5.Lcd.color565(SUCCESS_COLOR));
      INFO("%s\n", log.c_str());
    }
    else {
      String log = "I2C " + lastptr->name + " Find failed";
      cover_scroll_text(log, M5.Lcd.color565(FAILED_COLOR));
      INFO("%s\n", log.c_str());
      sys_error_skip();
    }
    delay(100);
    count++;
  } while(lastptr->nextPtr != nullptr );
  return 0;
}


int check_imu_init() {
  VERBOSE("check_imu_init()\n");
  if(M5.IMU.Init() == 0) {
    cover_scroll_text("IMU Check Successful", M5.Lcd.color565(SUCCESS_COLOR));
  }
  else {
    cover_scroll_text("IMU Check failed", M5.Lcd.color565(FAILED_COLOR));
    sys_error_skip();
  }
  return 0;
}


int check_sd_card() {
  VERBOSE("check_sd_card()\n");
  sdcard_type_t Type = SD.cardType();
  if( Type == CARD_UNKNOWN || Type == CARD_NONE ) {
    cover_scroll_text("SDCard Find failed",M5.Lcd.color565(FAILED_COLOR));
    sys_error_skip();
  }
  else {
    cover_scroll_text("SDCard Found",M5.Lcd.color565(SUCCESS_COLOR));
    Serial.printf("SDCard Type = %d \r\n", Type);
    Serial.printf("SDCard Size = %d \r\n", (int)(SD.cardSize() / 1024 / 1024));
  }
  return 0;
}


void i2s_task(void *arg) {
  VERBOSE("i2s_task()\n");
  size_t bytes_written = 0;
  i2s_write(SPEAKER_I2S_NUMBER, previewR, 120264, &bytes_written, portMAX_DELAY);
  vTaskDelete(NULL);
}


void power_setup() {
  VERBOSE("power_setup()\n");
  disp_power_buff.createSprite(77,102);
  disp_power_buff.fillRect(0,0,77,102,TFT_BLACK);
  disp_power_buff.drawColorBitmap(0, 39, 77, 24, (uint8_t*)image_power, 0xff9c00, 0x000000);
  disp_power_buff.pushSprite(240, 3);
}


void choose_power() {
  VERBOSE("choose_power()\n");
  uint32_t        color1 = 0;
  uint32_t        color2 = 0;
  uint16_t        posy = 6;
  system_power_t  power_now;

  power_now    = (M5.Axp.isACIN()) ? kPOWER_EXTERNAL : kPOWER_INTERNAL;
  if(power_now == sysState.power) return;
  if(power_now == kPOWER_EXTERNAL) {
    color1     = 0x6c6c6c;
    color2     = 0xff9c00;
    posy       = 68;
  }
  else {
    color2     = 0x6c6c6c;
    color1     = 0xff9c00;
    posy       = 3;
  }
  disp_power_buff.fillRect(0,  3, 77, 37, TFT_BLACK);
  disp_power_buff.fillRect(0, 68, 77, 37, TFT_BLACK);

  disp_power_buff.drawColorBitmap( 0,    0, 77,37, (uint8_t*)image_Internal, color1,   0x000000);
  disp_power_buff.drawColorBitmap( 0,   65, 77,37, (uint8_t*)image_External, color2,   0x000000);
  disp_power_buff.drawColorBitmap(54, posy, 20,31, (uint8_t*)image_PowerC,   0xae2828, 0x000000);
  disp_power_buff.drawColorBitmap( 0,   39, 77,24, (uint8_t*)image_power,    0xff9c00, 0x000000);
  disp_power_buff.pushSprite(240,3);
  sysState.power = power_now;
}


void setup_3d() {
  VERBOSE("setup_3d()\n");
  disp_3d_buff.createSprite(85, 56);
  disp_3d_buff.fillRect(0, 0, 85, 56, disp_3d_buff.color565(0x33, 0x20, 0x00));
  for(int n = 0; n < 12; n++) {
    rect_source[n].start_point.x = rect[n].start_point.x * 12;
    rect_source[n].start_point.y = rect[n].start_point.y * 12;
    rect_source[n].start_point.z = rect[n].start_point.z * 12;
    rect_source[n].end_point.x   = rect[n].end_point.x   * 12;
    rect_source[n].end_point.y   = rect[n].end_point.y   * 12;
    rect_source[n].end_point.z   = rect[n].end_point.z   * 12;
  }
}


void polar_to_cartesian(double Angle, double radius, Point_2d_t *point, int16_t offsetX = 42, int16_t offsetY = 28) {
  VERBOSE("polar_to_cartesian\n");
  point->x = radius  * cos( Angle ) + offsetX;
  point->y = offsetY - radius * sin( Angle );
}


void mpu6886_test() {
  VERBOSE("mpu6886_test()\n");
  float     accX       = 0;
  float     accY       = 0;
  float     accZ       = 0;
  double    theta      = 0;
  double    last_theta = 0;
  double    phi        = 0;
  double    last_phi   = 0;
  double    alpha      = 0.2;
  Line_3d_t x          = { .start_point = {0, 0, 0}, .end_point = {0, 0,  0} };
  Line_3d_t y          = { .start_point = {0, 0, 0}, .end_point = {0, 0,  0} };
  Line_3d_t z          = { .start_point = {0, 0, 0}, .end_point = {0, 0, 30} };
  Line_3d_t rect_dis;
  Line3D    line3d;

  line3d.setZeroOffset(42, 28);
  M5.IMU.getAccelData(&accX, &accY, &accZ);

  if ((accX < 1) && (accX > -1)) {
    theta = asin(-accX) * 57.295;
  }
  if (accZ != 0) {
    phi = atan(accY / accZ) * 57.295;
  }

  theta = alpha * theta + (1 - alpha) * last_theta;
  phi   = alpha * phi   + (1 - alpha) * last_phi;

  disp_3d_buff.fillRect(0, 0, 85, 56, disp_3d_buff.color565(0x33, 0x20, 0x00));
  z.end_point.x = 0;
  z.end_point.y = 0;
  z.end_point.z = 24;
  line3d.RotatePoint(&z.end_point, theta, phi, 0);
  line3d.RotatePoint(&z.end_point, &x.end_point, -90, 0, 0);
  line3d.RotatePoint(&z.end_point, &y.end_point, 0, 90, 0);

  line3d.printLine3D(&disp_3d_buff, &x, TFT_GREEN);
  line3d.printLine3D(&disp_3d_buff, &y, TFT_GREEN);
  line3d.printLine3D(&disp_3d_buff, &z, TFT_GREEN);

  uint16_t linecolor = disp_3d_buff.color565(0xff, 0x9c, 0x00);

  for (int n = 0; n < 12; n++) {
    line3d.RotatePoint(&rect_source[n].start_point, &rect_dis.start_point, theta, phi, (double)0);
    line3d.RotatePoint(&rect_source[n].end_point, &rect_dis.end_point, theta, phi, (double)0);
    line3d.printLine3D(&disp_3d_buff, &rect_dis, linecolor);
  }
  disp_3d_buff.pushSprite(74, 130);
  last_theta = theta;
  last_phi = phi;
}


void clock_setup() {
  VERBOSE("clock_setup()\n");
  disp_clock_buff.createSprite(164,101);
  disp_clock_buff.drawJpg(clockImage,18401,0,0,163,101);
}


void clock_flush() {
  VERBOSE("clock_flush()\n");
  uint16_t pos_x[6] = { 4, 28, 66, 90, 127, 144 };
  disp_clock_buff.fillRect(  0, 20, 162, 50, disp_clock_buff.color565(0x1a, 0x11, 0x00));
  disp_clock_buff.fillRect(  0, 25,   2, 45, disp_clock_buff.color565(0xff, 0x9c, 0x00));
  disp_clock_buff.fillRect( 56, 35,   5,  5, disp_clock_buff.color565(0xff, 0x9c, 0x00));
  disp_clock_buff.fillRect( 56, 54,   5,  5, disp_clock_buff.color565(0xff, 0x9c, 0x00));
  disp_clock_buff.fillRect(118, 40,   5,  5, disp_clock_buff.color565(0xff, 0x9c, 0x00));
  disp_clock_buff.fillRect(118, 57,   5,  5, disp_clock_buff.color565(0xff, 0x9c, 0x00));
  M5.Rtc.GetTime(&sysState.Rtctime);
  disp_clock_buff.drawColorBitmap(pos_x[0], 26, 24, 42, DigNumber     [sysState.Rtctime.Hours   / 10 ], 0xff9c00, 0x1a1100);
  disp_clock_buff.drawColorBitmap(pos_x[1], 26, 24, 42, DigNumber     [sysState.Rtctime.Hours   % 10 ], 0xff9c00, 0x1a1100);
  disp_clock_buff.drawColorBitmap(pos_x[2], 26, 24, 42, DigNumber     [sysState.Rtctime.Minutes / 10 ], 0xff9c00, 0x1a1100);
  disp_clock_buff.drawColorBitmap(pos_x[3], 26, 24, 42, DigNumber     [sysState.Rtctime.Minutes % 10 ],  0xff9c00, 0x1a1100);
  disp_clock_buff.drawColorBitmap(pos_x[4], 33, 18, 35, DigNumber_35px[sysState.Rtctime.Seconds / 10 ], 0xff9c00, 0x1a1100);
  disp_clock_buff.drawColorBitmap(pos_x[5], 33, 18, 35, DigNumber_35px[sysState.Rtctime.Seconds % 10 ], 0xff9c00, 0x1a1100);
  sysState.clockCount ++;
  sysState.clockCount %= 6;
  disp_clock_buff.drawColorBitmap(0, 0, 164, 17, rectptrBuff[sysState.clockCount], 0xff9c00, 0x000000);
  disp_clock_buff.pushSprite(72,4);
}


static void i2s_micro_fft_task(void *arg) {
  VERBOSE("i2s_micro_fft_task()\n");
  uint8_t*      microRawData = (uint8_t*)calloc(2048, sizeof(uint8_t));
  uint8_t       state        = MODE_MIC;
  uint8_t       FFTDataBuff[128];
  uint8_t       FFTValueBuff[24];
  size_t        bytesread;
  int16_t*      buffptr;
  double        data = 0;
  float         adc_data;
  uint16_t      ydata;
  uint32_t      subData;
  i2sQueueMsg_t QueueMsg;

  while(true) {
    if(xQueueReceive(i2s_state_queue,&QueueMsg,(TickType_t)0) == pdTRUE) {
      DEBUG("Queue Now\n");
      if(QueueMsg.state == MODE_MIC) {
        init_i2s_speaker_or_mic(MODE_MIC);
        state = MODE_MIC;
      }
      else {
        DEBUG("Speaker\n");
        DEBUG("Length: %d\n", QueueMsg.audioSize);
        init_i2s_speaker_or_mic(MODE_SPK);
        size_t written = 0;
        i2s_write(SPEAKER_I2S_NUMBER,(unsigned char*)QueueMsg.audioPtr,QueueMsg.audioSize,&written,portMAX_DELAY);
        state = MODE_SPK;
      }
    }
    else if( state == MODE_MIC ) {
      fft_config_t *real_fft_plan = fft_init(1024, FFT_REAL, FFT_FORWARD, NULL, NULL);
      i2s_read(I2S_NUM_0, (char *)microRawData, 2048, &bytesread, (100 / portTICK_RATE_MS));
      buffptr = ( int16_t*)microRawData;

      for ( int count_n = 0; count_n < real_fft_plan->size; count_n++) {
        adc_data = (float)map(buffptr[count_n], INT16_MIN, INT16_MAX, -2000, 2000);
        real_fft_plan->input[count_n] = adc_data;
      }
      fft_execute(real_fft_plan);

      for(int count_n = 1; count_n < real_fft_plan->size / 4; count_n++) {
        data = sqrt(real_fft_plan->output[2 * count_n] * real_fft_plan->output[2 * count_n] + real_fft_plan->output[2 * count_n + 1] * real_fft_plan->output[2 * count_n + 1]);
        if ((count_n - 1) < 128)  {
          data  = ( data > 2000 ) ? 2000 : data;
          ydata = map(data, 0, 2000, 0, 255);
          FFTDataBuff[128 - count_n] = ydata;
        }
      }

      for(int count = 0; count < 24; count++) {
        subData = 0;
        for( int count_i = 0; count_i < 5; count_i++ )  {
          subData += FFTDataBuff[count * 5 + count_i ];
        }
        subData /= 5;
        FFTValueBuff[count] = map(subData,0,255,0,8);
      }
      xQueueSend( fft_value_queue, (void * )&FFTValueBuff, 0 );
      fft_destroy(real_fft_plan);
      DEBUG("mmp\n");
    }
    else {
      delay(10);
    }
  }
}


void microphone_setup() {
  VERBOSE("microphone_setup()\n");
  fft_value_queue = xQueueCreate(5, 24 * sizeof(uint8_t));
  if(fft_value_queue == 0) return;
  i2s_state_queue = xQueueCreate(5, sizeof(i2sQueueMsg_t));
  if(i2s_state_queue == 0) return;
  init_i2s_speaker_or_mic(MODE_MIC);
  xTaskCreatePinnedToCore(i2s_micro_fft_task, "microPhoneTask", 4096, NULL, 3, NULL, 0);
  disp_fft_buff.createSprite(143, 54);
}


void microphone_fft() {
  VERBOSE("microphone_fft()\n");
  uint8_t FFTValueBuff[24];
  xQueueReceive(fft_value_queue, (void *)&FFTValueBuff, portMAX_DELAY);
  disp_fft_buff.fillRect(0, 0, 143, 54, disp_fft_buff.color565(0x33, 0x20, 0x00));
  uint32_t colorY = disp_fft_buff.color565(0xff, 0x9c, 0x00);
  uint32_t colorG = disp_fft_buff.color565(0x66, 0xff, 0x00);
  uint32_t colorRect;
  for(int x = 0; x < 24; x++) {
    for(int y = 0; y < 9; y++) {
      if(y < FFTValueBuff[23 - x]) {
        colorRect = colorY;
      }
      else if(y == FFTValueBuff[23 - x]) {
        colorRect = colorG;
      }
      else {
        continue;
      }
      disp_fft_buff.fillRect(x * 6, 54 - y * 6 - 5, 5, 5, colorRect);
    }
  }
  disp_fft_buff.pushSprite(170, 130);
}


void bat_power_setup() {
  VERBOSE("bat_power_setup()\n");
  disp_bat_buff.createSprite(64, 82);
  disp_bat_buff.drawJpg(batPowerImage, 13769, 0, 0, 64, 82);
  disp_bat_buff.pushSprite(3, 61);
}


void bat_power_flush() {
  VERBOSE("bat_power_flush()\n");
  uint32_t colorY = disp_fft_buff.color565(0xff, 0x9c, 0x00);
  uint32_t color  = (M5.Axp.isACIN()) ? 0x66ff00 : 0x6c6c6c;

  disp_bat_buff.drawColorBitmap(3, 37, 60, 10, Number_7x10px[14], color, 0x000000);
  disp_bat_buff.fillRect(10, 27, 2,  2, colorY);
  disp_bat_buff.fillRect(55,  4, 6, 10, colorY);

  float batVoltage    = M5.Axp.GetBatVoltage();
  float batPercentage = (batVoltage < 3.2) ? 0 : (batVoltage - 3.2) * 100;
  int   rectwidth     = 27 - 27 * (int)batPercentage / 100;
  uint8_t batVoltage1 = (uint16_t)batVoltage % 10;
  uint8_t batVoltage2 = (uint16_t)(batVoltage * 10) % 10;

  disp_bat_buff.drawColorBitmap( 3, 20, 7, 10, Number_7x10px[batVoltage1], 0xff9c00, 0x000000);
  disp_bat_buff.drawColorBitmap(13, 20, 7, 10, Number_7x10px[batVoltage2], 0xff9c00, 0x000000);
  disp_bat_buff.drawColorBitmap(20, 20, 7, 10, Number_7x10px[10],          0xff9c00,  0x000000);
  disp_bat_buff.drawRect(29, 17, 31, 15, colorY);

  if( M5.Axp.isACIN()) {
    sysState.batCount ++;
    sysState.batCount %= 7;
  }

  disp_bat_buff.drawColorBitmap(31, 19, 27, 11, batRect[sysState.batCount], 0xff9c00, 0x000000);
  disp_bat_buff.fillRect(58 - rectwidth, 19, rectwidth, 11, TFT_BLACK);

  if(sysState.batVoltageWriteCount > 50) {
    sysState.batVoltageWriteCount = 0;
    sysState.batVoltageBuff[sysState.batVoltageWriteptr] = ( batPercentage / 20 ) + 1;
    sysState.batVoltageWriteptr++;
    sysState.batVoltageReadptr++;

    sysState.batVoltageWriteptr %= 15;
    sysState.batVoltageReadptr  %= 15;

    disp_bat_buff.fillRect(6, 50, 55, 29, disp_bat_buff.color565(0x33, 0x20, 0x00));

    for(int i = 0; i < 11; i++) {
      int lim = sysState.batVoltageBuff[(sysState.batVoltageReadptr + i) % 15];
      for(int y = 0; y < lim; y++) {
        disp_bat_buff.fillRect(6 + i * 5, 74 - y * 5, 4, 4, colorY);
      }
    }
  }
  sysState.batVoltageWriteCount++;
  disp_bat_buff.pushSprite(3, 61);
}


void touch_setup() {
  VERBOSE("touch_setup()\n");
  uint32_t colorGREY = disp_touch_buff.color565(0x30,0x30,0x30);
  disp_touch_buff.createSprite(64, 40);
  disp_touch_buff.drawJpg(touchImage ,12262, 0, 0, 64, 40);

  disp_touch_buff.drawColorBitmap(12, 15, 7, 10, (uint8_t*)Number_7x10px[11], 0xff9c00, 0x000000);
  disp_touch_buff.drawColorBitmap(19, 15, 3, 10, (uint8_t*)Number_7x10px[13], 0xff9c00, 0x000000);
  disp_touch_buff.drawColorBitmap(12, 26, 7, 10, (uint8_t*)Number_7x10px[12], 0xff9c00, 0x000000);
  disp_touch_buff.drawColorBitmap(19, 26, 3, 10, (uint8_t*)Number_7x10px[13], 0xff9c00, 0x000000);
  for(int i = 0; i < 3; i++) {
    int number = (0 / (int)(pow( 10 , ( 2 - i )))) % 10;
    disp_touch_buff.drawColorBitmap(22 + i * 7, 15, 7, 10, (uint8_t*)Number_7x10px[number], 0xff9c00, 0x000000);
  }
  for(int i = 0; i < 3; i++) {
    int number = (0 / (int)(pow( 10 , (2 - i )))) % 10;
    disp_touch_buff.drawColorBitmap(22 + i * 7, 26, 7, 10, (uint8_t*)Number_7x10px[number], 0xff9c00, 0x000000);
  }
  disp_touch_buff.fillRect( 5, 15,  5, 21, TFT_BLACK);
  disp_touch_buff.fillRect(46,  4, 15,  8, colorGREY);
  disp_touch_buff.fillRect(46, 16, 15,  8, colorGREY);
  disp_touch_buff.fillRect(46, 28, 15,  8, colorGREY);
  disp_touch_buff.pushSprite(3, 146);
}


void touch_flush() {
  VERBOSE("touch_flush()\n");
  TouchPoint_t  pos           = M5.Touch.getPressPoint();
  bool          touchStateNow = (pos.x == -1) ? false : true;
  uint32_t      color;
  i2sQueueMsg_t msg;

  msg.state     = MODE_SPK;
  msg.audioPtr  = (void*)bibiSig;
  msg.audioSize = 8820;

  if(touchStateNow) {
    if(sysState.touchState == false) {
      xQueueSend(i2s_state_queue, &msg, (TickType_t)100);
      msg.state = MODE_MIC;
      xQueueSend(i2s_state_queue, &msg, (TickType_t)100);
    }

    uint32_t colorY    = disp_touch_buff.color565(0xff, 0x9c, 0x00);
    uint32_t colorG    = disp_touch_buff.color565(0x66, 0xff, 0x00);
    uint32_t colorGREY = disp_touch_buff.color565(0x30, 0x30, 0x30);
    disp_touch_buff.fillRect(5, 15, 5, 21, colorG);

    disp_touch_buff.drawColorBitmap(12, 15, 7, 10, (uint8_t*)Number_7x10px[11], 0xff9c00, 0x000000);
    disp_touch_buff.drawColorBitmap(19, 15, 3, 10, (uint8_t*)Number_7x10px[13], 0xff9c00, 0x000000);
    disp_touch_buff.drawColorBitmap(12, 26, 7, 10, (uint8_t*)Number_7x10px[12], 0xff9c00, 0x000000);
    disp_touch_buff.drawColorBitmap(19, 26, 3, 10, (uint8_t*)Number_7x10px[13], 0xff9c00, 0x000000);
    for(int i = 0; i < 3; i++) {
      int number = (pos.x / (int)( pow( 10 , ( 2 - i )))) % 10;
      disp_touch_buff.drawColorBitmap(22 + i * 7, 15, 7, 10, (uint8_t*)Number_7x10px[number], 0xff9c00, 0x000000);
    }
    for( int i = 0; i < 3; i++ ) {
      int number = (pos.y / (int)( pow( 10 , (2 - i )))) % 10;
      disp_touch_buff.drawColorBitmap(22 + i * 7, 26, 7, 10, (uint8_t*)Number_7x10px[number], 0xff9c00, 0x000000);
    }

    color = (touch_btn_0.inHotZone(pos)) ? colorY : colorGREY;
    disp_touch_buff.fillRect(46 ,4, 15, 8, color);

    color = (touch_btn_1.inHotZone(pos)) ? colorY : colorGREY;
    disp_touch_buff.fillRect(46, 16, 15, 8, color);

    color = (touch_btn_2.inHotZone(pos)) ? colorY : colorGREY;
    disp_touch_buff.fillRect(46, 28, 15, 8, color);

    if(sysState.touchState == false) {
      for(int i = 0; i < 6; i++) {
        sysState.App1Zone[i]->inHotZoneDoFun(pos);
      }
    }
  }
  else {
    uint32_t colorGREY = disp_touch_buff.color565(0x30,0x30,0x30);
    disp_touch_buff.fillRect( 5, 15,  5, 21, TFT_BLACK);
    disp_touch_buff.fillRect(46,  4, 15,  8, colorGREY);
    disp_touch_buff.fillRect(46, 16, 15,  8, colorGREY);
    disp_touch_buff.fillRect(46, 28, 15,  8, colorGREY);
  }
  disp_touch_buff.pushSprite(3,146);
  sysState.touchState = touchStateNow;
}


void sd_card_setup() {
  VERBOSE("sd_card_setup()\n");
  TFT_eSprite disp_sd_card_buff =  TFT_eSprite(&M5.Lcd);
  disp_sd_card_buff.createSprite(64, 54);
  disp_sd_card_buff.drawJpg(sdCardImage, 14835, 0, 0, 64, 54);
  sdcard_type_t Type = SD.cardType();
  if(Type == CARD_UNKNOWN || Type == CARD_NONE) {
    sysState.SDCardState = false;
    disp_sd_card_buff.fillRect(55,4,6,10,TFT_BLACK);
  }
  else {
    sysState.SDCardState = true;
    uint32_t colorY         = disp_sd_card_buff.color565(0xff,0x9c,0x00);
    uint64_t sdcardSize     = SD.cardSize() * 10 /1024 /1024/ 1024 ;
    uint64_t sdcardFreeSize = (SD.cardSize() - SD.usedBytes()) * 10 / 1024 / 1024 / 1024 ;

    disp_sd_card_buff.fillRect(55, 4, 6, 10,colorY);
    disp_sd_card_buff.drawColorBitmap( 3, 19, 7, 10, (uint8_t*)Number_7x10px[sdcardFreeSize /100 % 10], 0xff9c00, 0x000000);
    disp_sd_card_buff.drawColorBitmap(10, 19, 7, 10, (uint8_t*)Number_7x10px[sdcardFreeSize /10 % 10 ], 0xff9c00, 0x000000);
    disp_sd_card_buff.drawColorBitmap(20, 19, 7, 10, (uint8_t*)Number_7x10px[sdcardFreeSize % 10 ],     0xff9c00, 0x000000);
    disp_sd_card_buff.drawColorBitmap(33, 19, 7, 10, (uint8_t*)Number_7x10px[sdcardSize / 100 % 10],    0xff9c00, 0x000000);
    disp_sd_card_buff.drawColorBitmap(40, 19, 7, 10, (uint8_t*)Number_7x10px[sdcardSize / 10 % 10],     0xff9c00, 0x000000);
    disp_sd_card_buff.drawColorBitmap(50, 19, 7, 10, (uint8_t*)Number_7x10px[sdcardSize % 10],          0xff9c00, 0x000000);
    Serial.printf("SDCard Type = %d \r\n", Type);
    Serial.printf("SDCard Size = %d \r\n", (int)(SD.cardSize() / 1024 / 1024));
  }
  disp_sd_card_buff.pushSprite(3,4);
}


void sd_card_flush() {
  VERBOSE("sd_card_flush()\n");
  sdcard_type_t Type = SD.cardType();
  sysState.SDCardscaneCount++;
  if(sysState.SDCardscaneCount < 50) return;
  sysState.SDCardscaneCount = 0;

  if(Type == CARD_UNKNOWN || Type == CARD_NONE) {
    sysState.SDCardState = false;
    disp_sd_card_buff.fillRect(55, 4, 6, 10, TFT_BLACK);
  }
  else {
    sysState.SDCardState = true;
    uint32_t colorY = disp_sd_card_buff.color565(0xff, 0x9c, 0x00);

    uint64_t sdcardSize     = SD.cardSize() * 10 / 1024 / 1024 / 1024 ;
    uint64_t sdcardFreeSize = (SD.cardSize() - SD.usedBytes()) * 10 / 1024 / 1024 / 1024 ;

    disp_sd_card_buff.fillRect(55,4,6,10,colorY);
    disp_sd_card_buff.drawColorBitmap( 3, 19, 7, 10, (uint8_t*)Number_7x10px[sdcardFreeSize / 100 % 10], 0xff9c00, 0x000000);
    disp_sd_card_buff.drawColorBitmap(10, 19, 7, 10, (uint8_t*)Number_7x10px[sdcardFreeSize / 10 % 10],  0xff9c00, 0x000000);
    disp_sd_card_buff.drawColorBitmap(20, 19, 7, 10, (uint8_t*)Number_7x10px[sdcardFreeSize % 10],       0xff9c00, 0x000000);
    disp_sd_card_buff.drawColorBitmap(33, 19, 7, 10, (uint8_t*)Number_7x10px[sdcardSize / 100 % 10],     0xff9c00, 0x000000);
    disp_sd_card_buff.drawColorBitmap(40, 19, 7, 10, (uint8_t*)Number_7x10px[sdcardSize / 10 % 10],      0xff9c00, 0x000000);
    disp_sd_card_buff.drawColorBitmap(50, 19, 7, 10, (uint8_t*)Number_7x10px[sdcardSize % 10],           0xff9c00, 0x000000);
    Serial.printf("SDCard Type = %d\n", Type);
    Serial.printf("SDCard Size = %d\n", (int)(SD.cardSize() / 1024 / 1024));
  }
  disp_sd_card_buff.pushSprite(3, 4);
}


void draw_jpeg_image_from_sd_card(fs::FS &fs, const char *dirname) {
  VERBOSE("draw_jpeg_image_from_sd_card(fs, %s)\n", dirname);
  TouchPoint_t pos = M5.Touch.getPressPoint();
  uint16_t *buff   = (uint16_t*)ps_calloc(10880, sizeof(uint16_t));
  int Count        = 200;
  int menuAmoCount = 0;
  int menuPosyNow  = 0;
  int menuState    = 1; // 0 -> off, 1 -> runing, 2 -> open, 3 -> exiting
  bool flushJPEG   = false;

  HotZone_t _touchBtn0( 10, 206, 120, 280);
  HotZone_t _touchBtn1(130, 206, 200, 280);
  HotZone_t _touchBtn2(230, 206, 310, 280);

  disp_buff.fillRect(0, 0, 320, 240, TFT_BLACK);
  disp_buff.pushSprite(0, 0);

  menu_buff.createSprite(320, 34);
  menu_buff.drawJpg(imageMenu, 14900, 0, 0, 320, 34);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (true) {
    if (Count >= 200) {
      if (!file.isDirectory()) {
        disp_buff.drawJpgFile(fs, file.name(), 0, 0, 320, 240);
        disp_buff.getSprite2Buff(buff, 0, 206, 320, 34);
        flushJPEG = true;
      }
      file = root.openNextFile();
      if (!file) {
        root = fs.open(dirname);
        file = root.openNextFile();
      }
      Count = 0;
    }
    delay(5);
    Count++;
    pos = M5.Touch.getPressPoint();

    if(pos.x != -1) {
      switch (menuState)
      {
        case 0:
          menuState   = 1;
          menuPosyNow = 0;
          break;
        case 2:
          menuAmoCount = 0;
          if (_touchBtn2.inHotZone(pos)) goto endImage;
          break;
      }
    }

    switch(menuState) {
      case 1:
        menuPosyNow    += 4;
        if(menuPosyNow >= 34) {
          menuPosyNow   = 34;
          menuState     = 2;
          menuAmoCount  = 0;
          flushJPEG     = true;
        }
        break;
      case 2:
        menuAmoCount++;
        if(menuAmoCount > 500) {
          menuAmoCount  = 0;
          menuPosyNow   = 34;
          menuState     = 3;
        }
        break;
      case 3:
        menuPosyNow    -= 4;
        if(menuPosyNow <= 0) {
          menuPosyNow   = 0;
          menuState     = 0;
          menuAmoCount  = 0;
          flushJPEG     = true;
        }
        break;
    }

    if ((menuState == 1) || (menuState == 3) || (flushJPEG == true)) {
      if (flushJPEG == true)
        flushJPEG = false;
      if (menuState == 3)
        disp_buff.pushImage(0, 206, 320, 34, buff);
      disp_buff.pushInSprite(&menu_buff, 0, 0, 320, 34, 0, 241 - menuPosyNow);
      disp_buff.pushSprite(0, 0);
    }
  }
endImage:
  disp_buff.drawJpg(coreMainImage, 87169, 0, 0, 320, 240, 0, 0);
  disp_buff.pushSprite(0, 0);
  menu_buff.deleteSprite();
  sysState.power = kPOWER_MAX;
  free(buff);
}


void app_image() {
  VERBOSE("app_image()\n");
  if(sysState.SDCardState == false) return;
  draw_jpeg_image_from_sd_card(SD, "/img");
}


void app_sleep() {
  VERBOSE("app_sleep()\n");
  delay(500);
  M5.Axp.SetSleep();
}


void app_wifi() {
  VERBOSE("app_wifi()\n");
  bool first   = true;
  int  count   = 0;
  int  count_n = 0;

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  disp_buff.drawJpg(wifiScanImage, 28123, 0, 0, 320, 240);
  disp_buff.pushSprite(0, 0);

  disp_buff.setFreeFont(&EVA_20px);
  disp_buff.setTextSize(1);
  disp_buff.setTextColor(disp_buff.color565(0xff, 0xa0, 0), disp_buff.color565(0, 0, 0));
  disp_buff.setTextDatum(TL_DATUM);

  TouchPoint_t pos      = M5.Touch.getPressPoint();
  uint16_t color_yellow = disp_buff.color565(0xff, 0xa0, 0x0);
  uint16_t color_grey   = disp_buff.color565(0x43, 0x43, 0x43);
  HotZone escBtn(234, 210, 280, 320);

  WiFi.scanNetworks(true);
  uint16_t colorWifiBar[10] = { TFT_RED, TFT_RED,
                                color_yellow, color_yellow, color_yellow,
                                TFT_GREEN, TFT_GREEN, TFT_GREEN, TFT_GREEN, TFT_GREEN };
  delay(100);
  while(true) {
    int WifiNumber = WiFi.scanComplete();
    if(WifiNumber == - 1) {
      if(first) {
        if(count >= 10) {
          count = 0;
          disp_buff.fillRect( 0,34,320,173,TFT_BLACK);
          disp_buff.drawColorBitmap(122, 98, 76, 44, TouchFishBuff[count_n], 0xff9c00, 0x000000);
          disp_buff.pushSprite(0,0);
          count_n++;
          if(count_n >= 6) count_n = 0;
        }
        count++;
      }
    }
    else if (WifiNumber == 0) {
      Serial.println("no networks found");
      M5.Lcd.drawString("No networks found", 15, 22);
    }
    else {
      Serial.print(WifiNumber);
      Serial.println(" networks found");
      disp_buff.fillRect( 0,34,320,173,TFT_BLACK);
      if( WifiNumber >= 8 ) WifiNumber = 7;
      for (int i = 0; i < WifiNumber; i++) {
        disp_buff.fillRect( 18,34+i*24 + 2,7,20,disp_buff.color565(0xff,0xa0,0));
        String SSIDStr = WiFi.SSID(i);
        if(SSIDStr.length() > 11) {
          SSIDStr = SSIDStr.substring(0,8);
          SSIDStr += "...";
        }
        disp_buff.setTextDatum(TL_DATUM);
        disp_buff.drawString(SSIDStr, 35, 34  + i * 24 + 2);
      }
      if(first) {
        for (int i = 0; i < WifiNumber; i++) {
            for(int n = 9; n >= 0; n-- ) {
              disp_buff.fillRect(298 - 12 * n, 34 + i * 24 + 2, 8, 20, color_grey);
            }
        }
        disp_buff.pushSprite(0,0);
      }
      for(int n = 9; n >= 0; n--) {
        for (int i = 0; i < WifiNumber; i++)  {
          int32_t rssi      = (WiFi.RSSI(i) < -100 ) ? -100 : WiFi.RSSI(i);
          rssi              = map(rssi, -100, -20, -100, 0);
          uint16_t colorNow = color_yellow;

          if(rssi < ( n * -10)) {
            colorNow = color_grey;
          }
          else {
            colorNow = colorWifiBar[9 - n];
          }
          disp_buff.fillRect(298-12*n,34+i*24+2,8,20,colorNow);
        }
        if(first) {
          disp_buff.pushSprite(0, 0);
          delay(5);
        }
      }
      WiFi.scanDelete();
      WiFi.scanNetworks(true);
      if(!first) {
        disp_buff.pushSprite(0,0);
      }
      first = false;
    }
    delay(10);

    pos = M5.Touch.getPressPoint();
    if(pos.x != -1) {
      if(escBtn.inHotZone(pos)) break;
    }
  }
  disp_buff.drawJpg(coreMainImage, 87169, 0, 0, 320, 240, 0, 0);
  disp_buff.pushSprite(0,0);
  sysState.power = kPOWER_MAX;
}


void app_timer() {
  VERBOSE("app_timer()\n");
  disp_buff.drawJpg(timerAppImage, 59165, 0, 0, 320, 240, 0, 0);
  disp_buff.pushSprite(0,0);

  TouchPoint_t  pos           = M5.Touch.getPressPoint();
  bool          stopped       = true;
  bool          zeroflag      = false;
  bool          pressed       = false;
  uint16_t      posx[6]       = { 4, 28, 66, 90, 127, 144 };
  uint16_t      count_bibi    = 0;
  TFT_eSprite   Timerbuff     = TFT_eSprite(&M5.Lcd);
  TFT_eSprite   TimerRectbuff = TFT_eSprite(&M5.Lcd);
  unsigned long timeNow       = micros();
  int64_t       timeset       = 65000;
  int64_t       timeSetting   = timeset;
  int           min           = timeSetting / 60000 % 60;
  int           sec           = timeSetting / 1000 % 60;
  int           mill          = timeSetting % 1000;
  uint32_t      colorNum      = 0xff9c00;
  uint16_t      color16Yellow = Timerbuff.color565(0xff, 0x9c,   0);
  uint16_t      color16Red    = Timerbuff.color565(255,    0,    0);
  uint16_t      color16green  = Timerbuff.color565(  0,  255,    0);
  uint16_t      color_grey    = disp_buff.color565( 0x43, 0x43, 0x43);
  uint16_t      color16       = color16Yellow;
  i2sQueueMsg_t msg;
  uint16_t      colorBar[12]  = { color16green,  color16green,  color16green,  color16green,  color16green,  color16green,
                                  color16Yellow, color16Yellow, color16Yellow, color16Yellow,
                                  color16Red,    color16Red };
  HotZone       StopBtn(240,36,320,114);
  HotZone       CleanBtn(240,118,320,198);
  HotZone       SettingBtn(185,144,249,240);
  HotZone       EscBtn(240,200,320,280);

  Timerbuff.createSprite(176, 56);
  TimerRectbuff.createSprite(40, 164);
  TimerRectbuff.fillRect(0, 0, 40, 164, TFT_BLACK);

  msg.state     = MODE_SPK;
  msg.audioPtr  = (void*)bibiSig;
  msg.audioSize = 8820;

  Timerbuff.fillRect( 56, 13, 5, 5, Timerbuff.color565(0xff, 0x9c, 0x00));
  Timerbuff.fillRect( 56, 32, 5, 5, Timerbuff.color565(0xff, 0x9c, 0x00));
  Timerbuff.fillRect(118, 18, 5, 5, Timerbuff.color565(0xff, 0x9c, 0x00));
  Timerbuff.fillRect(118, 35, 5, 5, Timerbuff.color565(0xff, 0x9c, 0x00));
  TimerRectbuff.fillRect(0, 0, 40, 164, TFT_BLACK);
  for(int i = 0; i < 12; i++) {
    TimerRectbuff.fillRect(0, 154 - i * 14, 30, 10, colorBar[11 - i]);
  }
  TimerRectbuff.pushSprite(7, 36);

  while(true) {
    if(!stopped) {
      timeSetting  = timeSetting - (millis() - timeNow);
      if(timeSetting <= 0)  {
        timeSetting   = 0;
        count_bibi    = 0;
        zeroflag      = true;
        stopped       = true;
      }
    }
    if(zeroflag) {
      delay(10);
      count_bibi ++;
      if(count_bibi     > 20) {
          count_bibi    = 0;
          msg.state     = MODE_SPK;
          msg.audioPtr  = (void*)bibiSig;
          msg.audioSize = 8820;
          xQueueSend(i2s_state_queue,&msg,(TickType_t)portMAX_DELAY);
      }
    }

    int timerect = map(timeSetting, 0, timeset, 1, 12);
    TimerRectbuff.fillRect(0, 0, 40, 164, TFT_BLACK);
    for(int i = 0; i < 12; i++) {
      if( i < timerect )
        TimerRectbuff.fillRect(0, 154 - i * 14, 30, 10, colorBar[11 - i]);
      else
        TimerRectbuff.fillRect(0, 154 - i * 14, 30, 10, color_grey);
    }
    TimerRectbuff.pushSprite(7, 36);

    timeNow  = millis();
    min      = timeSetting / 60000 % 60;
    sec      = timeSetting / 1000 % 60;
    mill     = timeSetting / 10 % 100;
    colorNum = (min == 0) ? 0xff0000 : 0xff9c00;
    color16  = (min == 0) ? color16Red : color16Yellow;

    Timerbuff.fillRect( 56, 13, 5, 5, color16);
    Timerbuff.fillRect( 56, 32, 5, 5, color16);
    Timerbuff.fillRect(118, 18, 5, 5, color16);
    Timerbuff.fillRect(118, 35, 5, 5, color16);

    Timerbuff.drawColorBitmap(posx[0],  4, 24, 42, DigNumber[      min  / 10], colorNum, 0x000000);
    Timerbuff.drawColorBitmap(posx[1],  4, 24, 42, DigNumber[      min  % 10], colorNum, 0x000000);
    Timerbuff.drawColorBitmap(posx[2],  4, 24, 42, DigNumber[      sec  / 10], colorNum, 0x000000);
    Timerbuff.drawColorBitmap(posx[3],  4, 24, 42, DigNumber[      sec  % 10], colorNum, 0x000000);
    Timerbuff.drawColorBitmap(posx[4], 14, 18, 35, DigNumber_35px[ mill / 10], colorNum, 0x000000);
    Timerbuff.drawColorBitmap(posx[5], 14, 18, 35, DigNumber_35px[ mill % 10], colorNum, 0x000000);
    Timerbuff.pushSprite(54, 92);

    pos = M5.Touch.getPressPoint();
    if((pos.x != -1) && (pressed == false)) {
      pressed         = true;
      msg.state       = MODE_SPK;
      msg.audioPtr    = (void*)bibiSig;
      msg.audioSize   = 8820;

      xQueueSend(i2s_state_queue, &msg, (TickType_t)portMAX_DELAY);

      if(StopBtn.inHotZone(pos)) {
        stopped = (stopped == true) ? false : true;
      }
      if(zeroflag) {
          zeroflag    = false;
          timeSetting = timeset;
      }
      if(CleanBtn.inHotZone(pos)) {
        timeSetting = timeset;
        stopped     = true;

        TimerRectbuff.fillRect(0,0,40,164,TFT_BLACK);
        for(int i = 0; i< 12; i++) {
            TimerRectbuff.fillRect(0, 154 - i * 14, 30, 10, colorBar[11 - i]);
        }
        TimerRectbuff.pushSprite(7, 36);
      }
      if(EscBtn.inHotZone(pos)) {
        break;
      }
    }
    else if(pos.x == -1) {
      pressed = false;
    }
  }
  msg.state     = MODE_MIC;
  msg.audioPtr  = (uint8_t*)previewR;
  msg.audioSize = 120264;

  xQueueSend(i2s_state_queue,&msg,(TickType_t)portMAX_DELAY);

  Timerbuff.deleteSprite();
  disp_buff.drawJpg(coreMainImage, 87169, 0, 0, 320, 240, 0, 0);
  disp_buff.pushSprite(0, 0);
  sysState.power = kPOWER_MAX;
}


void tft_test() {
  VERBOSE("tft_test()\n");
  M5.Lcd.fillRect(0, 0, 320, 240, TFT_RED);
  delay(1000);
  M5.Lcd.fillRect(0, 0, 320, 240, TFT_GREEN);
  delay(1000);
  M5.Lcd.fillRect(0, 0, 320, 240, TFT_BLUE);
  delay(1000);
  disp_buff.pushSprite(0, 0);
}


void app_setting() {
  VERBOSE("app_setting()\n");
  i2sQueueMsg_t msg;
  HotZone       IOBtn   (  7,  99, 104, 154);
  HotZone       MotorBtn(113,  99, 210, 154);
  HotZone       TFTBtn  (  7, 161, 104, 216);
  HotZone       SoundBtn(113, 161, 210, 216);
  HotZone       EscBtn  (240, 200, 320, 280);
  TouchPoint_t  pos           = M5.Touch.getPressPoint();
  bool          pressed       = false;
  uint8_t       IOState       = 0;
  uint8_t       MotorState    = 1;
  uint8_t       ioBuff[6]     = {14, 13, 32, 33, 19, 27};
  uint16_t      adcReadCount  = 0;
  uint8_t       iostatebuff[4][6] = {
      {1, 1, 1, 1, 1, 1},
      {0, 1, 0, 1, 0, 1},
      {1, 0, 1, 0, 1, 0},
      {0, 0, 0, 0, 0, 0},
  };

  disp_buff.drawJpg(settingAppImage, 50683, 0, 0, 320, 240); //settingAppImage
  msg.state     = MODE_SPK;
  msg.audioPtr  = (void*)bibiSig;
  msg.audioSize = 8820;
  pinMode(14, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(33, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(27, OUTPUT);

  disp_buff.fillRect(193, 103, 14, 47, TFT_RED);

  if( sysState.soundFlag ) {
    disp_buff.fillRect(193, 165, 14, 47, TFT_GREEN);
  }
  else {
      disp_buff.fillRect(193, 165, 14, 47, TFT_RED);
  }
  disp_buff.pushSprite(0, 0);

  while(true) {
    pos = M5.Touch.getPressPoint();
    if((pos.x != -1 )&&( pressed == false)) {
        pressed       = true;
        msg.state     = MODE_SPK;
        msg.audioPtr  = (void*)bibiSig;
        msg.audioSize = 8820;
        xQueueSend(i2s_state_queue, &msg, (TickType_t)portMAX_DELAY);

        if(IOBtn.inHotZone(pos)) {
            for(int i = 0; i < 6; i++) {
              digitalWrite(ioBuff[i], iostatebuff[IOState][i]);
            }
            IOState++;
            IOState = (IOState >= 4) ? 0 : IOState;
            Serial.printf("Set IOState %d \n",IOState);
        }

        if(MotorBtn.inHotZone(pos)) {
          M5.Axp.SetLDOEnable(3, MotorState);
          if(MotorState == 1) {
            disp_buff.fillRect(193, 103, 14, 47, TFT_GREEN);
          }
          else {
            disp_buff.fillRect(193, 103, 14, 47, TFT_RED);
          }
          disp_buff.pushSprite(0, 0);
          MotorState = (MotorState == 0) ? 1 : 0;
          Serial.printf("Set Motor %d \n",MotorState);
        }

        if(SoundBtn.inHotZone(pos)) {
          sysState.soundFlag = ( sysState.soundFlag == true ) ? false : true;
          M5.Axp.SetSpkEnable(sysState.soundFlag);

          if(sysState.soundFlag) {
            disp_buff.fillRect(193, 165, 14, 47, TFT_GREEN);
          }
          else {
            disp_buff.fillRect(193, 165, 14, 47, TFT_RED);
          }
          disp_buff.pushSprite(0, 0);
        }
        if(TFTBtn.inHotZone(pos)) tft_test();
        if(EscBtn.inHotZone(pos)) break;
    }
    else if(pos.x == -1) {
      pressed = false;
    }

    if( adcReadCount > 50 ) {
      adcReadCount      = 0;
      uint16_t pin35ADC = analogRead(35);
      double pin35vol   = (double)pin35ADC *  3.3 / 4096;
      uint16_t pin36ADC = analogRead(36);
      double pin36vol   = (double)pin36ADC *  3.3 / 4096;

      if((pin35vol > 1.8 ) && (pin35vol < 2.2)) {
        disp_buff.fillRect(86, 103, 14, 21, TFT_GREEN);
      }
      else {
        disp_buff.fillRect(86, 103, 14, 21, TFT_RED);
      }
      if((pin36vol > 0.8) && (pin36vol < 1.2)) {
        disp_buff.fillRect(86, 129, 14, 21, TFT_GREEN);
      }
      else {
        disp_buff.fillRect(86, 129, 14, 21, TFT_RED);
      }
      disp_buff.pushSprite(0, 0);
      Serial.printf("ADC:%.2f,%.2f\n",pin35vol,pin36vol);
  }
  adcReadCount ++;
  delay(10);
  }
  msg.state = MODE_MIC;

  xQueueSend(i2s_state_queue,&msg,(TickType_t)portMAX_DELAY);

  disp_buff.drawJpg(coreMainImage, 87169, 0, 0, 320, 240, 0, 0);
  disp_buff.pushSprite(0, 0);
  sysState.power = kPOWER_MAX;
}


void mpu6886_page() {
  VERBOSE(" mpu6886_page()\n");
  HotZone_t     _touchBtn2(230, 206, 310, 280);
  TouchPoint_t  pos           = M5.Touch.getPressPoint();
  float         accX          = 0;
  float         accY          = 0;
  float         accZ          = 0;
  double        theta         = 0;
  double        last_theta    = 0;
  double        phi           = 0;
  double        last_phi      = 0;
  double        alpha         = 0.2;
  int           menuAmoCount  = 0;
  int           menuPosyNow   = 0;
  int           menuState     = 1;
  Line_3d_t     x             = { .start_point = {0, 0, 0}, .end_point = {0, 0,  0} };
  Line_3d_t     y             = { .start_point = {0, 0, 0}, .end_point = {0, 0,  0} };
  Line_3d_t     z             = { .start_point = {0, 0, 0}, .end_point = {0, 0, 30} };
  Line_3d_t     rect_dis;
  Line3D        line3d;

  menu_buff.createSprite(320, 34);
  menu_buff.drawJpg(imageMenu, 14900, 0, 0, 320, 34);
  for (int n = 0; n < 12; n++) {
    rect_source[n].start_point.x = rect[n].start_point.x * 50;
    rect_source[n].start_point.y = rect[n].start_point.y * 50;
    rect_source[n].start_point.z = rect[n].start_point.z * 50;
    rect_source[n].end_point.x   = rect[n].end_point.x   * 50;
    rect_source[n].end_point.y   = rect[n].end_point.y   * 50;
    rect_source[n].end_point.z   = rect[n].end_point.z   * 50;
  }

  line3d.setZeroOffset(160, 110);

  while(true) {
    M5.IMU.getAccelData(&accX, &accY, &accZ);
    if((accX < 1) && (accX > -1)) {
        theta = asin(-accX) * 57.295;
    }
    if (accZ != 0) {
        phi = atan(accY / accZ) * 57.295;
    }

    theta = alpha * theta + (1 - alpha) * last_theta;
    phi   = alpha * phi   + (1 - alpha) * last_phi;

    disp_buff.fillRect(0,0,320,240,disp_3d_buff.color565(0x0,0x0,0x0));

    z.end_point.x = 0;
    z.end_point.y = 0;
    z.end_point.z = 24;
    line3d.RotatePoint(&z.end_point, theta, phi, 0);
    line3d.RotatePoint(&z.end_point, &x.end_point, -90, 0, 0);
    line3d.RotatePoint(&z.end_point, &y.end_point, 0, 90, 0);
    line3d.printLine3D(&disp_buff, &x, TFT_GREEN);
    line3d.printLine3D(&disp_buff, &y, TFT_GREEN);
    line3d.printLine3D(&disp_buff, &z, TFT_GREEN);

    uint16_t linecolor = disp_buff.color565(0xff,0x9c,0x00);
    for (int n = 0; n < 12; n++) {
      line3d.RotatePoint(&rect_source[n].start_point, &rect_dis.start_point, theta, phi, (double)0);
      line3d.RotatePoint(&rect_source[n].end_point,   &rect_dis.end_point,   theta, phi, (double)0);
      line3d.printLine3D(&disp_buff, &rect_dis, linecolor);
    }
    last_theta = theta;
    last_phi   = phi;
    pos        = M5.Touch.getPressPoint();
    if(pos.x != -1) {
      switch(menuState) {
        case 0: menuState    = 1;
                menuPosyNow  = 0;
                break;
        case 2: menuAmoCount = 0;
                if( _touchBtn2.inHotZone(pos)) goto endImage;
                break;
      }
    }

    switch(menuState) {
      case 1:
        menuPosyNow     += 4;
        if(menuPosyNow  >= 34) {
            menuPosyNow  = 34;
            menuState    = 2;
            menuAmoCount = 0;
        }
        break;
      case 2:
        menuAmoCount++;
        if(menuAmoCount  > 500) {
            menuAmoCount = 0;
            menuPosyNow  = 34;
            menuState    = 3;
        }
        break;
      case 3:
        menuPosyNow     -= 4;
        if(menuPosyNow  <= 0) {
            menuPosyNow  = 0;
            menuState    = 0;
            menuAmoCount = 0;
        }
        break;
    }
    disp_buff.fillRect(0, 206, 320, 34, TFT_BLACK);
    disp_buff.pushInSprite(&menu_buff, 0, 0, 320, 34, 0, 241 - menuPosyNow);
    disp_buff.pushSprite(0, 0);
  }
  endImage:

  for (int n = 0; n < 12; n++) {
    rect_source[n].start_point.x = rect[n].start_point.x * 12;
    rect_source[n].start_point.y = rect[n].start_point.y * 12;
    rect_source[n].start_point.z = rect[n].start_point.z * 12;
    rect_source[n].end_point.x   = rect[n].end_point.x   * 12;
    rect_source[n].end_point.y   = rect[n].end_point.y   * 12;
    rect_source[n].end_point.z   = rect[n].end_point.z   * 12;
  }
  disp_buff.drawJpg(coreMainImage, 87169, 0, 0, 320, 240, 0, 0);
  disp_buff.pushSprite(0 ,0);
  sysState.power = kPOWER_MAX;
}


void set_check_state(int number, bool state, bool flush) {
  VERBOSE("set_check_state(%d, %s, %s)\n", number, state ? "true": "false", flush ? "true": "false");
  int       posx = 0;
  int       posy = 0;
  uint8_t*  ptr1 = nullptr;
  uint8_t*  ptr2 = nullptr;

  if(number >= 5) {
    posx = 166;
    posy = 154 - (number - 5) * 33;
    ptr1 = (uint8_t*)image_Sysinit_0000s_0000_L1;
    ptr2 = (uint8_t*)image_Sysinit_0000s_0001_L2;
  }
  else {
    posx = 44;
    posy = 22 + number * 33;
    ptr1 = (uint8_t*)image_Sysinit_0001s_0000_R1;
    ptr2 = (uint8_t*)image_Sysinit_0001s_0001_R2;
  }
  int32_t color = (state) ? 0x35ffae : 0xff0000;

  disp_buff.drawColorBitmapAlpha(posx, posy,     53, 33, (uint8_t*)ptr2, color,    0x000000);
  disp_buff.drawColorBitmapAlpha(posx, posy + 1, 53, 33, (uint8_t*)ptr1, 0xff9a00, color);

  if( flush == false ) return;
  disp_buff.pushSprite(0, 0);
}


void sys_init_check() {
  VERBOSE("sys_init_check()\n");
  disp_buff.drawJpg(coverImage, 21301, 0, 0, 320, 240, 0, 0);
  disp_buff.pushSprite(0, 0);
}


void setup() {
  sys.begin("FactoryTest");
  disp_buff.createSprite(320, 240);
  disp_cover_scroll_buff.createSprite(320, 60);

  add_i2c_device("Axp192",0x34);
  add_i2c_device("CST Touch",0x38);
  add_i2c_device("IMU6886",0x68);
  add_i2c_device("BM8563",0x51);

  M5.Axp.SetLcdVoltage(2800);

  sys_init_check();
  SD.begin();

  sysState.App1Zone[0] = new HotZone( 20, 191,  64, 235, &app_image);
  sysState.App1Zone[1] = new HotZone( 79, 191, 123, 235, &app_wifi);
  sysState.App1Zone[2] = new HotZone(138, 191, 182, 235, &app_timer);
  sysState.App1Zone[3] = new HotZone(197, 191, 241, 235, &app_sleep);
  sysState.App1Zone[4] = new HotZone(256, 191, 300, 235, &app_setting);
  sysState.App1Zone[5] = new HotZone( 72, 130, 161, 186, &mpu6886_page);

  M5.Axp.SetLcdVoltage(3300);

  M5.Axp.SetBusPowerMode(0);
  M5.Axp.SetCHGCurrent(AXP192::kCHG_190mA);

  M5.Axp.SetSpkEnable(sysState.soundFlag);
  init_i2s_speaker_or_mic(MODE_SPK);
  xTaskCreatePinnedToCore(i2s_task, "i2s_task", 4096, NULL, 3, NULL, 0);

  M5.Axp.SetLDOEnable(3, true);
  cover_scroll_text("Motor Test", M5.Lcd.color565(SUCCESS_COLOR));
  delay(100);
  M5.Axp.SetLDOEnable(3, false);

  M5.Axp.SetLed(1);
  cover_scroll_text("LCD Test", M5.Lcd.color565(SUCCESS_COLOR));
  delay(100);
  M5.Axp.SetLed(0);

  check_i2c_addr();
  check_psram();
  check_imu_init();
  check_sd_card();

  disp_cover_scroll_buff.deleteSprite();

  M5.Axp.SetLDOVoltage(3, 3300);
  M5.Axp.SetLed(1);

  disp_buff.drawJpg(coreMainImage, 87169, 0, 0, 320, 240, 0, 0);
  disp_buff.pushSprite(0, 0);

  setup_3d();
  clock_setup();
  power_setup();
  microphone_setup();
  bat_power_setup();
  touch_setup();
  sd_card_setup();
  choose_power();
}


void loop() {
  if(timecount >= 2) {
    timecount = 0;
    clock_flush();
    bat_power_flush();
  }
  timecount++;
  sd_card_flush();
  choose_power();
  mpu6886_test();
  microphone_fft();
  touch_flush();
  delay(5);
}
