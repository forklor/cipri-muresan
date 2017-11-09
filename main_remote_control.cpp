#include "main_remote_control.h"

#ifdef REMOTE_CONTROL_CONTROLLER

#include <Arduino.h>

#include "modules/wireless.h"
#include "modules/keypad.h"
#include "modules/display.h"
#include "modules/menu.h"
#include "modules/timer.h"

#define UPDATE_MOTOR_STATUS_MS 800

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

motorStatus currentMotorStatus;
bool updatingMotorStatus;
long lastUpdateStatusTime;
motorParameters settingParameters;
int selectedMotorNumber;

void displayMotorStatus() {
	updatingMotorStatus = true;
	display_lcd.clear();
	display_lcd.setCursor(0, 0);
	display_lcd.print("M STATUS ");
	display_lcd.setCursor(9, 0);
	display_lcd.print(selectedMotorNumber);
	display_lcd.setCursor(10, 0);
	display_lcd.print("S:");
	display_lcd.setCursor(12, 0);
	display_lcd.print(currentMotorStatus.speed);
	display_lcd.setCursor(0, 1);
	display_lcd.print("D:");
	display_lcd.setCursor(2, 1);
	display_lcd.print(currentMotorStatus.direction);
	display_lcd.setCursor(3, 1);
	display_lcd.print("C:");
	display_lcd.setCursor(5, 1);
	display_lcd.print(currentMotorStatus.cs1);
	display_lcd.setCursor(9, 1);
	display_lcd.print("C:");
	display_lcd.setCursor(11, 1);
	display_lcd.print(currentMotorStatus.cs2);
	lastUpdateStatusTime = millis();
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


int decimalVal;
void setDecimalValueUp() {
	timerValue += 1;
	display_lcd.setCursor(0, 0);
	display_lcd.print(decimalVal);
}

void setDecimalValueDown() {
	decimalVal -= 1;
	display_lcd.setCursor(0, 0);
	display_lcd.print(decimalVal);
}

void startSettingDecimalValue(long value, char *displayName) {

	decimalVal = value;
	display_lcd.clear();
	display_lcd.setCursor(0, 0);
	display_lcd.print(decimalVal);
	display_lcd.setCursor(0, 1);
	display_lcd.print(displayName);
}

void updateInputMotorParameters() {

	if(menu_get_current() == MENU_SET_MOTOR_SPEED) {
		char displayMenu = menu_get_display();
		switch(displayMenu) {
			case MENU_SET_MOTOR_SPEED:
				startSettingDecimalValue(settingParameters.maxSpeed, "SPEED MOTOR ");
				break;

			case MENU_SET_MOTOR_ACCELERATION:
				startSettingDecimalValue(settingParameters.acceleration, "ACCELERATION MOTOR ");
				break;

			case MENU_SET_MOTOR_DECELERATION:
				startSettingDecimalValue(settingParameters.decelerationPercentage, "DECELERATION MOTOR ");
				break;

			case MENU_SET_MOTOR_CS:
				startSettingDecimalValue(settingParameters.csThreshold, "CURRENT MOTOR ");
				break;

			case MENU_SET_MOTOR_CHANGE_TIME:
				startSettingTimeValue(settingParameters.changeDirTime, "TIME MOTOR ");
				break;
			case MENU_GET_MOTOR_STATUS:
				displayMotorStatus();
				break;
		}
	}
}

void rc_wirelessMessageAckReceived(wirelessMessage message) {
	Serial.print("received ack message ");
	Serial.println(message.type);
	if(message.type == MESSAGE_GET_PARAMS && menu_get_current() == MENU_SET_MOTOR_SPEED) {
		settingParameters = message.parameters;
		updateInputMotorParameters();
		Serial.print("s: ");
		Serial.print(message.parameters.maxSpeed);
		Serial.print(" a: ");
		Serial.print(message.parameters.acceleration);
		Serial.print(" d: ");
		Serial.print(message.parameters.decelerationPercentage);
		Serial.print(" cs: ");
		Serial.print(message.parameters.csThreshold);
		Serial.print(" t: ");
		Serial.println(message.parameters.changeDirTime);
	}

	if(message.type == MESSAGE_MOTOR_STATUS
		&& updatingMotorStatus
		&& menu_get_current() == MENU_SET_MOTOR_SPEED
		&& menu_get_display() == MENU_GET_MOTOR_STATUS) {
		currentMotorStatus = message.status;
		displayMotorStatus();
		Serial.print("s: ");
		Serial.print(message.status.speed);
		Serial.print(" d: ");
		Serial.print(message.status.direction);
		Serial.print(" cs1: ");
		Serial.print(message.status.cs1);
		Serial.print(" cs2: ");
		Serial.println(message.status.cs2);
	}
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
	} else if(currentMenu == MENU_SET_MOTOR_SPEED) {
		char displayMenu = menu_get_display();
		// Save previously set item
		bool updateParams = true;
		switch(displayMenu) {
			case MENU_SET_MOTOR_SPEED:
				settingParameters.maxSpeed = decimalVal;
				break;
			case MENU_SET_MOTOR_ACCELERATION:
				settingParameters.acceleration = decimalVal;
				break;
			case MENU_SET_MOTOR_DECELERATION:
				settingParameters.decelerationPercentage = decimalVal;
				break;
			case MENU_SET_MOTOR_CS:
				settingParameters.csThreshold = decimalVal;
				break;
			case MENU_SET_MOTOR_CHANGE_TIME:
				settingParameters.changeDirTime = timerValue;
				break;
			default:
				updateParams = false;
				break;
		}

		if(updateParams) {
			wirelessMessage msg;
			msg.type = MESSAGE_SET_PARAMS;
			msg.parameters = settingParameters;
			wireless_send_message(selectedMotorNumber, msg);
		}

		menu_down();
		displayMenu = menu_get_display();
		if(displayMenu == MENU_GET_MOTOR_STATUS) {
			currentMotorStatus = { 0, 0, 0, 0 };
			updatingMotorStatus = true;
		} else {
			updatingMotorStatus = false;
		}
		updateInputMotorParameters();
	}
}

void up_pressed() {

	char current = menu_get_current();
	if(current == MENU_ROOT) {
		menu_up();
	}


	if(current == MENU_MANUAL_SET_RUN_TIME) {
		setTimerValueUp();
	}

	if(current == MENU_SET_MOTOR_SPEED) {
		switch(menu_get_display()) {
			case MENU_SET_MOTOR_SPEED:
			case MENU_SET_MOTOR_ACCELERATION:
			case MENU_SET_MOTOR_DECELERATION:
			case MENU_SET_MOTOR_CS:
				setDecimalValueUp();
				break;
			case MENU_SET_MOTOR_CHANGE_TIME:
				setTimerValueUp();
				break;
			default:
				break;
		}
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

	if(current == MENU_SET_MOTOR_SPEED) {
		switch(menu_get_display()) {
			case MENU_SET_MOTOR_SPEED:
			case MENU_SET_MOTOR_ACCELERATION:
			case MENU_SET_MOTOR_DECELERATION:
			case MENU_SET_MOTOR_CS:
				setDecimalValueDown();
				break;
			case MENU_SET_MOTOR_CHANGE_TIME:
				setTimerValueDown();
				break;
			default:
				break;
		}
	}
}

void start_stop_all_pressed() {

	char currentMenu = menu_get_current();
	if(currentMenu == MENU_MANUAL_SET_RUN_TIME || currentMenu == MENU_SET_MOTOR_SPEED) {
		updatingMotorStatus = false;
		menu_use(MENU_ROOT);
		menu_down();
		// Move to auto timer mode if timer is running
		if(!timer_is_paused()) {
			menu_down();
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

				msg.type = MESSAGE_MOTOR_STATUS;
				wireless_send_message(WIRELESS_MODULE_1, msg);
				break;
			}
		case 'A':
			start_stop_all_pressed();
			break;
		case 'B':
			up_pressed();
			break;
		case 'C':
			down_pressed();
			break;
		case 'D':
			ok_pressed();
		default:
			//do nothing
			break;
	}
}

void start_program_timer() {
	if(menu_get_current() == MENU_ROOT) {
		menu_left();
		menu_select();
		startSettingTimeValue(timer_get_run_time(), "MOTOR RUN TIME");
	}
}

void start_program_motor(int motorNo) {
	if(menu_get_current() == MENU_ROOT) {
		menu_right();
		menu_select();
		selectedMotorNumber = motorNo;
		wirelessMessage msg;
		msg.type = MESSAGE_GET_PARAMS;
		wireless_send_message(motorNo, msg);
	}
}

void keypadListener(Key *keys, int keysLen) {

	if(keysLen == 1) {
		Key key = keys[0];
		if(key.kstate == RELEASED) {
			keyReleased(key.kchar);
		}
		// OK pressed for 5 seconds
		if(key.kstate == HOLD && key.kchar == 'D') {
			start_program_timer();
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
	wireless_listen_ack(rc_wirelessMessageAckReceived);
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
	if(updatingMotorStatus && milliseconds - lastUpdateStatusTime >= UPDATE_MOTOR_STATUS_MS) {
		wirelessMessage msg;
		msg.type = MESSAGE_MOTOR_STATUS;
		wireless_send_message(selectedMotorNumber, msg);
	}
}

#endif //MOTOR_REMOTE_CONTROLLER