#pragma once

class ezScreenShot {
	public:
		static bool     		entry(uint8_t command, void* data);
		static void     		begin();
		static void     		menu();
		static void     		snap();
		static uint16_t 		loop();
		static bool     		on();
	private:
		static bool     		_on;
		static unsigned long	_last_snap;
		static void     		_read_flash();
		static void     		_write_flash();
};
