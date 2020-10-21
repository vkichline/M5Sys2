#include <M5Sys2.h>
#include <WiFiScanner.h>


WiFiScanner scanner;


void splash() {
  VERBOSE("splash()\n");
  M5.Lcd.clear();
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.drawCentreString("Scanning", 160, 80, 4);
}


void setup() {
  sys.begin("WiFiScanner");
  scanner.begin();
}


void loop() {
  VERBOSE("loop()\n");
  splash();
  ezMenu  m("WiFi Scanner");
  m.txtSmall();
  m.buttons("up # back | Home # select ## down # Scan");

  scanner.scan(m);
  while(true) {
    if(0 == m.runOnce()) sys.load_home_program();
    String selection = m.pickButton();
    DEBUG("selection = %s\n", selection);
    if(0 == selection.compareTo("Scan")) {
      splash();
      scanner.scan(m);
    }
    else if(0 == selection.compareTo("select")) {
      scanner.display_node(m.pickName().c_str());
    }
  }
}
