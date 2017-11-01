#ifndef MAIN_ENTRY_FILE
#define MAIN_ENTRY_FILE

#include "modules/wireless.h"
#include "modules/wireless_message.h"
#include "modules/keypad.h"
#include "modules/display.h"

void keypadPressed(char key) {
	Serial.println(key);
	display_lcd.home();
	display_lcd.print("HELLO THERE");
}

void setup() {

	Serial.begin(9600);
	_keypad_setup(keypadPressed);
	_display_setup();
}

void loop() {
	_keypad_loop(millis());
}

#endif //MAIN_ENTRY_FILE