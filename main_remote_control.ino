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
	wirelessMessage msg;
	switch(key) {
		case 'A':
			Serial.println("Sending toggle start/stop message");
			msg.type = MESSAGE_TOGGLE_START_STOP;
			wireless_send_message(WIRELESS_MODULE_1, msg);
			break;
		case 'B':
			Serial.println("Sending change direction message");
			msg.type = MESSAGE_CHANGE_DIRECTION;
			wireless_send_message(WIRELESS_MODULE_1, msg);
			break;
		case 'C':
			Serial.println("Sending stop message");
			msg.type = MESSAGE_STOP;
			wireless_send_message(WIRELESS_MODULE_1, msg);
			break;
		case 'D':
			Serial.println("Sending start message");
			msg.type = MESSAGE_START;
			wireless_send_message(WIRELESS_MODULE_1, msg);
			break;
		default:
			//do nothing
			break;
	}
}

void setup() {

	Serial.begin(9600);
	Serial.println("Remote control program running");
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