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

void _display_setup() {
	display_lcd.begin(16, 2);
	display_lcd.setCursor(0, 0);
	display_lcd.print(" ");
}

void _display_loop(long milliseconds) {

}