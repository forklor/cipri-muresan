#include "main_remote_control.h"

#ifdef REMOTE_CONTROL_CONTROLLER

#include <Arduino.h>

#include "modules/wireless.h"
#include "modules/keypad.h"
#include "modules/display.h"
#include "modules/menu.h"
#include "modules/timer.h"

void timerStateChanged(bool running) {
	int all_addresses[6] = {
		WIRELESS_MODULE_1,
		WIRELESS_MODULE_2,
		WIRELESS_MODULE_3,
		WIRELESS_MODULE_4,
		WIRELESS_MODULE_5,
		WIRELESS_MODULE_6 
	};

	wirelessMessage msg;
	msg.type = running ? MESSAGE_START : MESSAGE_STOP;
	wireless_send_message(all_addresses, 6, msg);
}

void ok_pressed() {

	if(menu_get_current() == MENU_ROOT) {

		char currentMenu = menu_get_display();
		if(currentMenu == MENU_MANUAL) {
			display_show_message("SWITCHED", "TO MANUAL MODE", 1500);
			if(!timer_is_paused()) {
				timer_stop();
			}
		} else if (currentMenu == MENU_TIMER) {
			display_show_message("SWITCHED", "TO TIMER MODE", 1500);
			if(timer_is_paused()) {
				timer_start();
			}
		}
	}
}

void up_pressed() {

	if(menu_get_current() == MENU_ROOT) {
		menu_up();
	}
}

void down_pressed() {

	if(menu_get_current() == MENU_ROOT) {
		menu_down();
	}
}

void start_stop_all_pressed() {

	int all_addresses[6] = {
		WIRELESS_MODULE_1,
		WIRELESS_MODULE_2,
		WIRELESS_MODULE_3,
		WIRELESS_MODULE_4,
		WIRELESS_MODULE_5,
		WIRELESS_MODULE_6 
	};

	char currentMenu = menu_get_display();
	wirelessMessage msg;
	bool validCommand = false;

	if(currentMenu == MENU_MANUAL) {

		if(timer_is_running()) {
			display_show_message("STOPPING", "ALL MOTORS", 1500);
			msg.type = MESSAGE_STOP;
			timer_stop();
			timer_set_state(false);
			validCommand = true;	
		} else {
			menu_left();
			display_show_message("STARTING", "ALL MOTORS", 1500);
			msg.type = MESSAGE_START;
			timer_stop();
			timer_set_state(true);
			validCommand = true;
		}
		
	} else if(currentMenu == MENU_TIMER) {

		if(timer_is_running()) {
			display_show_message("STOPPING", "ALL MOTORS TIMER", 1500);
			timer_set_state(false);
			if(timer_is_paused()) timer_start();
			msg.type = MESSAGE_START;
			validCommand = true;
		} else {
			display_show_message("STARTING", "ALL MOTORS TIMER", 1500);
			timer_set_state(true);
			if(timer_is_paused()) timer_start();
			msg.type = MESSAGE_START;
			validCommand = true;
		}
	}
	
	if(validCommand) wireless_send_message(all_addresses, 6, msg);
}


void keyReleased(char key) {
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
			start_stop_all_pressed();
			break;
		case 'B':
			up_pressed();
			// Serial.println("Sending change direction message");
			// msg.type = MESSAGE_CHANGE_DIRECTION;
			// wireless_send_message(WIRELESS_MODULE_1, msg);
			break;
		case 'C':
			down_pressed();
			// Serial.println("Sending stop message");
			// msg.type = MESSAGE_STOP;
			// wireless_send_message(WIRELESS_MODULE_1, msg);
			break;
		case 'D':
			ok_pressed();
			// Serial.println("Sending start message");
			// msg.type = MESSAGE_START;
			// wireless_send_message(WIRELESS_MODULE_1, msg);
			// break;
		default:
			//do nothing
			break;
	}
}

void start_program_timer() {
	Serial.println("Programming timer");
}

void start_program_motor(int motorNo) {
	Serial.print("Programming motor");
	Serial.println(motorNo);
}

void keypadListener(Key *keys, int keysLen) {

	if(keysLen == 1) {
		Key key = keys[0];
		if(key.kstate == RELEASED) {
			keyReleased(key.kchar);
		}
	} else if(keysLen == 2) {
		bool okHold = false;
		for(int i = 0; i<keysLen; i++) {
			if(keys[i].kchar == 'D' && keys[i].kstate == HOLD) {
				okHold = true;
				break;
			}
		}

		for(int i = 0; i<keysLen; i++) {
			if(keys[i].kstate == HOLD) {
				switch(keys[i].kchar) {
					case 'A':
						start_program_timer();
					break;

					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
						int motorNumber = keys[i].kchar - '0';
						start_program_motor(motorNumber);
					break;
				}
			}
		}
	}

	Serial.print("====");
	Serial.println(keysLen);
	for(int i = 0; i<keysLen; i++) {
		Serial.print(i);
		Serial.print("key ");
		Serial.print(keys[i].kchar);
		Serial.print(" ");
		char *msg = keys[i].kstate == PRESSED ? "PRESSED" : (keys[i].kstate == HOLD ? "HOLD" : (keys[i].kstate == RELEASED ? "RELEASED" : "IDLE"));
		Serial.println(msg);
	}
}

void _setup() {

	Serial.begin(9600);
	Serial.println("Remote control program running");
	_wireless_setup(A0, A1, WIRELESS_REMOTE);
	_keypad_setup(keypadListener);
	_display_setup();
	_menu_setup();
	_timer_setup(timerStateChanged);
}

void _loop() {
	long milliseconds = millis();
	_wireless_loop(milliseconds);
	_keypad_loop(milliseconds);
	_timer_loop(milliseconds);
	_menu_loop(milliseconds);
	_display_loop(milliseconds);
}

#endif //MOTOR_REMOTE_CONTROLLER