#include "display.h"

LiquidCrystal_I2C display_lcd(0x3F, 16, 2);

void _display_setup() {

	display_lcd.begin(16, 2);
	display_lcd.backlight();
	display_lcd.setCursor(0, 0);
	display_lcd.print("HELLO THERE");
	delay ( 1000 );
}

void _display_loop(long milliseconds) {

}