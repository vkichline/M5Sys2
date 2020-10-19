#define ARDUINO_M5STACK_Core2
#define USE_DISPLAY
#include <M5Sys2.h>
#include <M5StackUpdater.h>

#define RELOAD_DELAY_SECONDS  60
#define HOME_PROGRAM          "/app/Home.bin"


// If _time_offset is zero, M5Sys2::begin() has not been called.
M5Sys2::M5Sys2() {
  _time_offset = 0;
}


// Start the system by initializing M5ez (and thereby M5Core2).
// get_app_name() becomes the one source of truth for the application's title.
// _time_offset, when added ti millis(), will give relatively accurate time in mS
// If the unit is upside down when reset, that's the gesture for loading the Home program.
// Start the logger.
//
void M5Sys2::begin(const char* appName) {
  RTC_TimeTypeDef   td;
  _app_name = appName;
  ez.begin();
  M5.Lcd.fillScreen(BLACK);

  // Set _time_offset to the difference between millis and time in milliseconds since midnight
  M5.Rtc.GetTime(&td);
  long m = millis();
  long c = td.Hours * 3600000 + td.Minutes * 60000 + td.Seconds * 1000;
  _time_offset = m - c;
  log.begin(appName, _time_offset);
  VERBOSE("_time_offset = %ld\n", _time_offset);
  ez.settings.menuObj.addItem("Logging", log.menu);

  M5.IMU.Init();
  _check_for_reload();
}


const char* M5Sys2::get_app_name() {
  return _app_name.c_str();
}


void M5Sys2::load_home_program() {
  updateFromFS(SD, HOME_PROGRAM);
  esp_restart();
}


void M5Sys2::launch(const char* app_name) {
  INFO("Loading application %s\n", app_name);
  updateFromFS(SD, app_name);
  esp_restart();
}


// Call immediately upon reboot.
// If the device was rebooted while upside-down, load the menu program.
// If the last reload was quite recent, skip menu load - this prevents
// loops as the device lays on its face.
//
void M5Sys2::_check_for_reload() {
  Preferences     prefs;
  RTC_TimeTypeDef t;
  float           dummy;
  float           z;
  uint32_t        last_time;
  uint32_t        time;

  M5.IMU.getAccelData(&dummy, &dummy, &z);
  DEBUG("Reload: %s\n", (z < -0.9) ? " true" : "false");
  // If the M5Stick Core2 is laying face down when reset:
  if(z < -0.9) {
    // DRY: If face down & rebooted, only do it once, not continually
    M5.Rtc.GetTime(&t);
    time = t.Hours * 3600 + t.Minutes * 60 + t.Seconds;
    prefs.begin("sd-menu", true);  // read-only
    last_time = prefs.getUInt("time", 0);
    prefs.end();
    // If it's been less than RELOAD_DELAY_SECONDS seconds, don't reload
    uint32_t  elapsed = time - last_time;
    if(RELOAD_DELAY_SECONDS > elapsed) {
      INFO("Last reboot only %d second%s ago; skipping reload.\n", elapsed, (1 == elapsed) ? "" : "s");
      return;
    }
    // Store the time we're restarting at
    prefs.begin("sd-menu", false);  // read/write
    prefs.putUInt("time", time);
    prefs.end();
    load_home_program();
  }
}


// Start the SPIFFS file system and return the status
//
bool M5Sys2::start_spiffs(bool formatOnFailure) {
  if(SPIFFS.begin(formatOnFailure)) {
    log.debug("SPIFFS file system started\n");
    return true;
  }
  else {
    log.error("SPIFFS File System startup failed.\n");
    return false;
  }
}


// Save a screenshot to the file specified.
// Based on https://stackoverflow.com/questions/16724214/writing-images-with-an-arduino
// and https://web.archive.org/web/20080912171714/http://www.fortunecity.com/skyscraper/windows/364/bmpffrmt.html
//
bool M5Sys2::screen_shot(const char* file_name) {
  // Allocate a one line data buffer
  unsigned char* data = (unsigned char*)malloc(320 * 3);
  if(!data) {
    ERROR("M5Sys2::Screen_shot: Failed to allocate screenshot buffer.");
    return false;
  }
  char  buffer[32];
  int   file_size = 54 + 230400; // headers (54 bytes) + pixel data = (row_size * 240)

  // Attempt to open the file
  if(nullptr == file_name) {
    RTC_DateTypeDef date;
    RTC_TimeTypeDef time;
    M5.Rtc.GetData(&date);
    M5.Rtc.GetTime(&time);
    snprintf(buffer, 32, "/img/%4d%02d%02d%02d%02d%02d.bmp", date.Year, date.Month, date.Date, time.Hours, time.Minutes, time.Seconds);
    file_name = buffer;
  }
  File file = SD.open(file_name, "w");
  if(!file) {
    ERROR("M5Sys2::Screen_shot: Failed to open file %s\n", file_name);
    return false;
  }
  INFO("saving screenshot \"%s\"\n", file_name);

  // Write the headers
  // create file headers (also taken from StackOverflow example)
  unsigned char bmpFileHeader[14] = {            // file header (always starts with BM!)
    'B','M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0 };
  unsigned char bmpInfoHeader[40] = {            // info about the file (size, etc)
    40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  bmpFileHeader[ 2] = (unsigned char)(file_size      );
  bmpFileHeader[ 3] = (unsigned char)(file_size >>  8);
  bmpFileHeader[ 4] = (unsigned char)(file_size >> 16);
  bmpFileHeader[ 5] = (unsigned char)(file_size >> 24);

  bmpInfoHeader[ 4] = (unsigned char)(      320      );
  bmpInfoHeader[ 5] = (unsigned char)(      320 >>  8);
  bmpInfoHeader[ 6] = (unsigned char)(      320 >> 16);
  bmpInfoHeader[ 7] = (unsigned char)(      320 >> 24);
  bmpInfoHeader[ 8] = (unsigned char)(      240      );
  bmpInfoHeader[ 9] = (unsigned char)(      240 >>  8);
  bmpInfoHeader[10] = (unsigned char)(      240 >> 16);
  bmpInfoHeader[11] = (unsigned char)(      240 >> 24);
  file.write(bmpFileHeader, sizeof(bmpFileHeader));   // write file header
  file.write(bmpInfoHeader, sizeof(bmpInfoHeader));   // " info header

  // Write the bitmap data
  for(int i = 239; i >= 0; i--) {
    M5.Lcd.readRectRGB(0, i, 320, 1, data);
    file.write(data, 320 * 3);
  }
  file.close();
  free(data);
  INFO("screenshot saved\n");
  return true;
}


// Arduino style; create the singleton; .h file will make it extern.

M5Sys2  sys;
