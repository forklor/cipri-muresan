#ifndef MAIN_ENTRY_FILE
#define MAIN_ENTRY_FILE

#include "modules/wireless.h"
#include "modules/keypad.h"
#include "modules/display.h"

// void rc_wirelessMessageReceived(wirelessMessage message) {
// 	Serial.print("received message");
// 	Serial.print(message.type);
// 	Serial.print("\n");
// }

void keypadPressed(char key) {
	display_lcd.home();
	display_lcd.print(key);

	Serial.println(key);

	if(key == 'A') {
		Serial.println("Sending message");
		wirelessMessage msg = {
			MESSAGE_TOGGLE_START_STOP,
			{0, 0, 0, 0}
		};
		wireless_send_message(WIRELESS_MODULE_1, msg);
	}
}

void setup() {

	Serial.begin(9600);
	Serial.println("Module 1 program running");
	_wireless_setup(A0, A1, WIRELESS_REMOTE);
	_keypad_setup(keypadPressed);
	_display_setup();
	//wireless_listen(WIRELESS_REMOTE, m_wirelessMessageReceived);
}

void loop() {
	long milliseconds = millis();
	_wireless_loop(milliseconds);
	_keypad_loop(milliseconds);
}

#endif //MAIN_ENTRY_FILE