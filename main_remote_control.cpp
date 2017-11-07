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

long timerValue;

void setTimerValueUp() {
	timerValue += 1000;
	display_lcd.setCursor(0, 0);
	display_lcd.print(timer_get_display_time(timerValue));
}

void setTimerValueDown() {
	timerValue -= 1000;
	display_lcd.setCursor(0, 0);
	display_lcd.print(timer_get_display_time(timerValue));
}

void startSettingTimeValue(long value, char *displayName) {
	
	timerValue = value;

	display_lcd.clear();
	display_lcd.setCursor(0, 0);
	display_lcd.print(timer_get_display_time(timerValue));
	display_lcd.setCursor(0, 1);
	display_lcd.print(displayName);
}

void ok_pressed() {

	char currentMenu = menu_get_current();
	if(currentMenu == MENU_ROOT) {

		char displayMenu = menu_get_display();
		if(displayMenu == MENU_MANUAL) {
			display_show_message("SWITCHED", "TO MANUAL MODE", 1500);
			if(!timer_is_paused()) {
				timer_stop();
			}
		} else if (displayMenu == MENU_TIMER) {
			display_show_message("SWITCHED", "TO TIMER MODE", 1500);
			if(timer_is_paused()) {
				timer_start();
			}
		}
	} else if(currentMenu == MENU_MANUAL_SET_RUN_TIME) {
		char displayMenu = menu_get_display();
		if(displayMenu == MENU_MANUAL_SET_RUN_TIME) {
			timer_set_run_time(timerValue);
			menu_down();
			startSettingTimeValue(timer_get_stop_time(), "MOTOR STOP TIME");
		} else {
			timer_set_stop_time(timerValue);
			menu_down();
			startSettingTimeValue(timer_get_run_time(), "MOTOR RUN TIME");
		}
	}
}

void up_pressed() {

	char current = menu_get_current();
	if(current == MENU_ROOT) {
		menu_up();
	}

	Serial.println(current);

	if(current == MENU_MANUAL_SET_RUN_TIME) {
		setTimerValueUp();
	}
}

void down_pressed() {

	char current = menu_get_current();
	if(current == MENU_ROOT) {
		menu_down();
	}

	if(current == MENU_MANUAL_SET_RUN_TIME) {
		setTimerValueDown();
	}
}

void start_stop_all_pressed() {

	char currentMenu = menu_get_current();
	if(currentMenu == MENU_MANUAL_SET_RUN_TIME) {
		menu_use(MENU_ROOT);
		menu_down();
		// Move to auto timer mode if timer is running
		if(!timer_is_paused()) {
			menu_down()
		}

		return;
	}

	int all_addresses[6] = {
		WIRELESS_MODULE_1,
		WIRELESS_MODULE_2,
		WIRELESS_MODULE_3,
		WIRELESS_MODULE_4,
		WIRELESS_MODULE_5,
		WIRELESS_MODULE_6 
	};

	wirelessMessage msg;
	bool validCommand = false;
	char displayMenu = menu_get_display();


	if(displayMenu == MENU_MANUAL) {

		if(timer_is_running()) {
			display_show_message("STOPPING", "ALL MOTORS", 1500);
			msg.type = MESSAGE_STOP;
			timer_stop();
			timer_set_state(false);
			validCommand = true;	
		} else {
			display_show_message("STARTING", "ALL MOTORS", 1500);
			msg.type = MESSAGE_START;
			timer_stop();
			timer_set_state(true);
			validCommand = true;
		}
		
	} else if(displayMenu == MENU_TIMER) {

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
	char currentMenu = menu_get_current();
	switch(key) {
		case '1':
			if(currentMenu == MENU_ROOT) {
				msg.type = MESSAGE_TOGGLE_START_STOP;
				wireless_send_message(WIRELESS_MODULE_1, msg);
				break;
			}
		case '4':
			if(currentMenu == MENU_ROOT) {
				msg.type = MESSAGE_CHANGE_DIRECTION;
				wireless_send_message(WIRELESS_MODULE_1, msg);
				break;
			}
		case '7':
			if(currentMenu == MENU_ROOT) {
				msg.type = MESSAGE_SET_PARAMS;
				msg.parameters = {
					50,
					5,
					3,
					15
				};
				wireless_send_message(WIRELESS_MODULE_1, msg);
				break;
			}
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
	menu_left();
	menu_select();
	startSettingTimeValue(timer_get_run_time(), "MOTOR RUN TIME");
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