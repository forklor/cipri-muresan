#include "display.h"

#define I2C_ADDR  0x3F
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

LiquidCrystal_I2C	display_lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

bool hasMessage;
long messageStartTime;
long messageDisplayTime;

void _display_setup() {
	display_lcd.begin(16, 2);
	display_lcd.setCursor(0, 0);
	display_lcd.print(" ");
}

void _display_loop(long milliseconds) {
	if(hasMessage && milliseconds - messageStartTime >= messageDisplayTime) {
		hasMessage = false;
	}
}

void display_show_message(char *line1, char *line2, long timeMs) {
	display_lcd.clear();
	display_lcd.setCursor(0, 0);
	display_lcd.print(line1);

	display_lcd.setCursor(0, 1);
	display_lcd.print(line2);

	messageStartTime = millis();
	messageDisplayTime = timeMs;
	hasMessage = true;
}

bool display_has_message() {
	return hasMessage;
}
