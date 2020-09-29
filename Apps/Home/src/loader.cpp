#include <SD.h>
#include "Home.h"

void loader() {
  File root = SD.open("/app");
  if(!root) {
    ez.msgBox("Critical Error", "SD Card not found.");
    return;
  }
  ezMenu menu("Load Program");
  menu.txtSmall();
  menu.buttons(STD_SUBMENU_BUTTONS);
  menu.setSortFunction(ezMenu::sort_asc_name_ci);

  // Fill the menu with executable binaries:
  while(true) {
    File f = root.openNextFile();
    if(!f) break;
    if(!f.isDirectory()) {
      String name(f.name());
      if(name.startsWith("/app/")) name.remove(0, 5);   // Remove the leading text
      // Invisible (resource fork) files can be eliminated with MacZap, but if not, skip them!
      if(!name.startsWith("._")) {
        if(name.endsWith(".bin")) {
          name.remove(name.length() - 4);               // remove '.bin'
          if(0 != name.compareTo("menu")) {             // don't show the launcher (which is running)
            String str = f.name() + String("|") + name; // Item is full name | display name
            menu.addItem(str);
          }
        }
      }
    }
    f.close();
  }
  root.close();
  switch(menu.runOnce()) {
    case 0: return;
    default: {
      sys.launch(menu.pickName().c_str());
    }
  }
}
