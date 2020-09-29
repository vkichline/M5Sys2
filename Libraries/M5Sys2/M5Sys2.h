#pragma once

#include <Preferences.h>
#include <M5Core2.h>
#include <M5ez.h>
#include <Logger.h>

// M5Sys2 is built to load cooperative programs from the SD card for M5Stack Core2 + M5ez (transitional version)
// The expected organization of the SD Card is:
//
//  /app          This directory contains loadable .bin files
//  /img          This directory contains .bmp, .jpg and .png files
//  /snd          This directory contains sound samples
//  /log          This directory contains M5Sys2 log files
//  /dat          For general purpose: make a sub-dir per app
//  /version.json System version information
//
//  /app/Home.bin is the system app, loaded by load_home_program()
//
// Loading the M5Sys2 header file creates a `sys` object.
// Calling `sys.begin()` does all the M5 and ez initialization needed.
//
// Written by Van Kichline
// in the year of the plague

#define STD_SUBMENU_BUTTONS   "up # Back # select ## down #"  // very frequently used menu button string
#define M5SYS2_PREFS_NAME      "M5Sys2"                       // Name of the NVS partition for settings

class M5Sys2 {
  public:
    M5Sys2();
    void        begin(const char* appName);
    void        load_home_program();
    void        launch(const char* app_name);
    const char* get_app_name();
    bool        start_spiffs(bool formatOnFailure = false);
    Logger      log;
  private:
    void        _check_for_reload();
    String      _app_name;
    long        _time_offset;   // millis() + _time_offset = time in milliseconds
};

extern  M5Sys2  sys;
