#include <Preferences.h>
#include <M5ez.h>
#include "ezScreenShot.h"


#define			SNAP_DELAY		10000
#define			ACCEL_DELAY		500

bool			ezScreenShot::_on;
unsigned long	ezScreenShot::_last_snap;
unsigned long	ezScreenShot::_last_accel;
M5_Orientation	ezScreenShot::_last_orentation;
M5_Orientation	ezScreenShot::_trigger_orentation;


bool ezScreenShot::entry(uint8_t command, void* data) {
    switch(command) {
        case FEATURE_MSG_PING:
			return true;
        case FEATURE_MSG_START:
            begin();
            return true;
		case FEATURE_MSG_QUERY_ENABLED:
			return on();
    }
    return false;
}


void ezScreenShot::begin() {
	_last_orentation = unknown_up;
	_last_snap 		 = 0;
	_read_flash();
	ez.settings.menuObj.addItem("ScreenShot settings", ezScreenShot::menu);
	ez.addEvent(ezScreenShot::loop);
}


void ezScreenShot::menu() {
	bool start_state = _on;
	ezMenu menu("ScreenShot Settings");
	menu.txtSmall();
	menu.buttons("up#Back#select##down#");
	menu.addItem("on|Enable ScreenShot\t" + (String)(_on ? "on" : "off"));
	menu.addItem("trigger|Trigger\t" + get_orientation_name(_trigger_orentation) + " up", _select_trigger_menu);
	while(int selection = menu.runOnce()) {
		switch(selection) {
			case 1:
				_on = !_on;
				menu.setCaption(1, "Enable ScreenShot\t" + (String)(_on ? "on" : "off"));
				break;
			case 2:
				menu.setCaption(2, "Trigger\t" + get_orientation_name(_trigger_orentation) + " up");
				break;
		}
	}
	if (_on != start_state) {
		_write_flash();
	}
}


void ezScreenShot::_select_trigger_menu() {
	ezMenu menu("ScreenShot Trigger");
	menu.txtSmall();
	menu.buttons("up#Back#select##down#");
	menu.addItem("2|Back Up");
	menu.addItem("4|Top Up");
	menu.addItem("6|Bottom Up");
	menu.addItem("3|Left Up");
	menu.addItem("5|Right Up");
	menu.addItem("1|Face Up");
	if(menu.runOnce()) {
		M5_Orientation old_orientation = _trigger_orentation;
		_trigger_orentation = (M5_Orientation)atoi(menu.pickName().c_str());
		if(_trigger_orentation != old_orientation) _write_flash();
	}
}


// Save a screenshot to the file specified.
// Based on https://stackoverflow.com/questions/16724214/writing-images-with-an-arduino
// and https://web.archive.org/web/20080912171714/http://www.fortunecity.com/skyscraper/windows/364/bmpffrmt.html
//
void ezScreenShot::snap() {
	// Allocate a one line data buffer
	unsigned char* data = (unsigned char*)malloc(320 * 3);
	if(!data) {
	return;
	}
	char  			buffer[32];
	int   			file_size = 54 + 230400; // headers (54 bytes) + pixel data = (row_size * 240)
	RTC_DateTypeDef	date;
	RTC_TimeTypeDef	time;

	M5.Rtc.GetData(&date);
	M5.Rtc.GetTime(&time);
	snprintf(buffer, 32, "/img/%4d%02d%02d%02d%02d%02d.bmp", date.Year, date.Month, date.Date, time.Hours, time.Minutes, time.Seconds);
	File file = SD.open(buffer, "w");
	if(!file) return;

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
	// RGB -> BGR
	for(int i = 0; i < 320; i++) {
		uint8_t temp = data[i * 3];
		data[i * 3]  = data[(i * 3) + 2];
		data[(i * 3) + 2] = temp;
	}
	file.write(data, 320 * 3);
	}
	file.close();
	free(data);
	_last_snap = millis();
}


uint16_t ezScreenShot::loop() {
	float	dummy, z;
	if(_on && _last_snap + SNAP_DELAY < millis()) {
		if(_trigger_orentation == get_orientation()) {
			Serial.println("saving screenshot");
			snap();
		}
	}
	return _on ? 100 : 1000;
}


bool ezScreenShot::on() {
	return _on;
}


void ezScreenShot::_read_flash() {
	Preferences prefs;
	prefs.begin("M5ez", true);	// read-only
	_on 				= prefs.getBool("scrshot_on", false);
	_trigger_orentation = (M5_Orientation)prefs.getChar("scrshot_trigger", (char)back_up);
	prefs.end();
}


void ezScreenShot::_write_flash() {
	Preferences prefs;
	prefs.begin("M5ez", false);	// read-write
	prefs.putBool("scrshot_on", _on);
	prefs.putChar("scrshot_trigger", _trigger_orentation);
	prefs.end();
}


M5_Orientation ezScreenShot::get_orientation() {
  if (millis() - _last_accel < ACCEL_DELAY) return _last_orentation;
  float 	  ax, ay, az;
  const float threshold = 0.85;
  _last_accel 			= millis();
  _last_orentation 		= unknown_up;
  M5.IMU.getAccelData(&ax, &ay, &az);
  if      (az >  threshold) _last_orentation = face_up;
  else if (az < -threshold) _last_orentation = back_up;
  else if (ay >  threshold) _last_orentation = top_up;
  else if (ay < -threshold) _last_orentation = bottom_up;
  else if (ax >  threshold) _last_orentation = right_up;
  else if (ax < -threshold) _last_orentation = left_up;
  return _last_orentation;
}


String ezScreenShot::get_orientation_name(M5_Orientation orientation) {
	switch(orientation) {
		case unknown_up:	return "unknown";
		case face_up:		return "face";
		case back_up:		return "back";
		case left_up:		return "left";
		case top_up:		return "top";
		case right_up:		return "right";
		case bottom_up:		return "bottom";
	}
}
