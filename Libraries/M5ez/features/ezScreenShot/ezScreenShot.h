#pragma once

// Orientation describes which way is up wrt the M5.
// Normally, sitting on its botton, face_up would be the orientation.
// Top and bottom refer to the top and bottom sides.
//
enum M5_Orientation { unknown_up, face_up, back_up, left_up, top_up, right_up, bottom_up };

class ezScreenShot {
	public:
		static bool     		entry(uint8_t command, void* data);
		static void     		begin();
		static void     		menu();
		static void     		snap();
		static uint16_t 		loop();
		static bool     		on();
		static M5_Orientation	get_orientation();
		static String           get_orientation_name(M5_Orientation orientation);
	private:
		static bool     		_on;
		static unsigned long	_last_snap;
		static unsigned long    _last_accel;
		static M5_Orientation	_last_orentation;
		static M5_Orientation	_trigger_orentation;
		static void     		_read_flash();
		static void     		_write_flash();
		static void             _select_trigger_menu();
};
