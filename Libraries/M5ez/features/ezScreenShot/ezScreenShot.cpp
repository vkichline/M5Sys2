#include <Preferences.h>
#include <M5ez.h>
#include "ezScreenShot.h"


#define			SNAP_DELAY					10000

bool			ezScreenShot::_on;
unsigned long	ezScreenShot::_last_snap;


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
	_last_snap = 0;
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
		switch (menu.runOnce()) {
			case 1:
				_on = !_on;
				break;
			case 0:
				if (_on != start_state) {
					_write_flash();
				}
				return;
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
}


uint16_t ezScreenShot::loop() {
	float	dummy, z;
	if(_last_snap + SNAP_DELAY < millis()) {
		M5.IMU.getAccelData(&dummy, &dummy, &z);
		if(z < -0.9) {
			snap();
			_last_snap = millis();
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
	_on = prefs.getBool("scrshot_on", false);
	prefs.end();
}


void ezScreenShot::_write_flash() {
	Preferences prefs;
	prefs.begin("M5ez", false);	// read-write
	prefs.putBool("scrshot_on", _on);
	prefs.end();
}
