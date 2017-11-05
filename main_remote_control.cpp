#include "main_remote_control.h"

#ifdef REMOTE_CONTROL_CONTROLLER

#include <Arduino.h>

#include "modules/wireless.h"
#include "modules/keypad.h"
#include "modules/display.h"
#include "modules/menu.h"
#include "modules/timer.h"

// void rc_wirelessMessageReceived(wirelessMessage message) {
// 	Serial.print("received message");
// 	Serial.print(message.type);
// 	Serial.print("\n");
// }

void keypadPressed(char key) {

	int all_addresses[6] = {
		WIRELESS_MODULE_1,
		WIRELESS_MODULE_2,
		WIRELESS_MODULE_3,
		WIRELESS_MODULE_4,
		WIRELESS_MODULE_5,
		WIRELESS_MODULE_6 
	};

	Serial.println(key);
	wirelessMessage msg;
	switch(key) {
		case '1':
			msg.type = MESSAGE_TOGGLE_START_STOP;
			wireless_send_message(WIRELESS_MODULE_1, msg);
			break;
		case '4':
			msg.type = MESSAGE_CHANGE_DIRECTION;
			wireless_send_message(WIRELESS_MODULE_1, msg);
			break;
		case '7':
			msg.type = MESSAGE_SET_PARAMS;
			msg.parameters = {
				50,
				5,
				3,
				15
			};
			wireless_send_message(WIRELESS_MODULE_1, msg);
			break;
		case 'A':
			msg.type = MESSAGE_TOGGLE_START_STOP;
			timer_start();
			wireless_send_message(all_addresses, 6, msg);
			break;
		case 'B':
			menu_up();
			// Serial.println("Sending change direction message");
			// msg.type = MESSAGE_CHANGE_DIRECTION;
			// wireless_send_message(WIRELESS_MODULE_1, msg);
			break;
		case 'C':
			menu_down();
			// Serial.println("Sending stop message");
			// msg.type = MESSAGE_STOP;
			// wireless_send_message(WIRELESS_MODULE_1, msg);
			break;
		case 'D':
			menu_select();
			// Serial.println("Sending start message");
			// msg.type = MESSAGE_START;
			// wireless_send_message(WIRELESS_MODULE_1, msg);
			// break;
		default:
			//do nothing
			break;
	}
}

void _setup() {

	Serial.begin(9600);
	Serial.println("Remote control program running");
	_wireless_setup(A0, A1, WIRELESS_REMOTE);
	_keypad_setup(keypadPressed);
	_display_setup();
	_menu_setup();
	_timer_setup();
}

void _loop() {
	long milliseconds = millis();
	_wireless_loop(milliseconds);
	_keypad_loop(milliseconds);
	_timer_loop(milliseconds);
	_menu_loop(milliseconds);
}

#endif //MOTOR_REMOTE_CONTROLLER