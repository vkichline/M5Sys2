#include <Preferences.h>
#include "Home.h"


// To keep from falling into the rollback trap while testing,
// disable rollback by deleting NVS data.
//
void disable_rollback() {
  Preferences prefs;
  prefs.begin("sd-menu", false);  // read/write
  prefs.remove("menusize");
  prefs.remove("digest");
  prefs.end();
}


// Display the sysinfo from SysInfo lib
//
void system_info_menu() {
  VERBOSE("sysinfo_control()\n");
  SysInfo sysinfo;
  sysinfo.display_info(); // This loops forever or loads Home
}


void shutdown() { M5.Axp.DeepSleep();      }


void setup() {
  #include <themes/default.h>
  #include <themes/dark.h>
  ez.remove("ezBattery");
  sys.begin("Home");
}


void loop() {
  ezMenu menu("Home Menu");
  menu.txtSmall();
  menu.addItem("Load a Program",   loader);
  menu.addItem("Disable Rollback", disable_rollback);
  menu.addItem("Settings",         ez.settings.menu);
  menu.addItem("System Info",      system_info_menu);
  menu.addItem("Shutdown",         shutdown);
  menu.run();
}
