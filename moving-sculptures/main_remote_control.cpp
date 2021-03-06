#include "main_remote_control.h"

#ifdef REMOTE_CONTROL_CONTROLLER

#define MEMORY_CHECK_VALUE 33

#include <Arduino.h>
#include <EEPROM.h>

#include "modules/wireless.h"
#include "modules/keypad.h"
#include "modules/display.h"
#include "modules/menu.h"
#include "modules/timer.h"

#ifdef DEBUG
#include "modules/MemoryFree.h"
#endif
#include "print.h"

#define UPDATE_MOTOR_STATUS_MS 1200
#define UPDATE_BATTERY_LEVEL_MS 5000

struct timerParameters {
	long runTime;
	long stopTime;
};

wirelessMessageCommand statusMsg;



motorStatus currentMotorStatus;
long lastUpdateStatusTime;
long lastUpdateBatteryTime;
motorParameters settingParameters;
int selectedMotorNumber;
bool updatingMotorStatus = false;
bool sentStopAll = false;
bool sentStartAll = false;


void timerStateChanged(bool running) {

	wirelessMessageCommand msg;
	msg.type = running ? MESSAGE_START : MESSAGE_STOP;
	wireless_send_message_all(msg);
	// Update last state to be sent with message_status command
	if(msg.type == MESSAGE_START) {
		sentStartAll = true;
		sentStopAll = false;
	} else if(msg.type == MESSAGE_STOP) {
		sentStopAll = true;
		sentStartAll = false;
	}
	lastUpdateBatteryTime = millis();
}

void displayMotorStatus() {
	updatingMotorStatus = true;
	display_lcd.clear();
	display_lcd.setCursor(0, 0);
	display_lcd.print("M STATUS ");
	display_lcd.setCursor(8, 0);
	display_lcd.print(selectedMotorNumber);
	display_lcd.setCursor(10, 0);
	display_lcd.print("S=");
	display_lcd.setCursor(12, 0);
	display_lcd.print(currentMotorStatus.speed);
	display_lcd.setCursor(0, 1);
	display_lcd.print("B=");
	display_lcd.setCursor(2, 1);
	display_lcd.print(((float)currentMotorStatus.battery/55.35));
	display_lcd.setCursor(9, 1);
	display_lcd.print("C=");
	display_lcd.setCursor(11, 1);
	display_lcd.print(currentMotorStatus.cs2);
	lastUpdateStatusTime = millis();
}

long timerValue;

void setTimerValueUp() {
	timerValue += 1000;
	display_lcd.setCursor(0, 0);
	char *display_val = timer_get_display_time(timerValue);
	display_lcd.print(display_val);
	free(display_val);
}

void setTimerValueDown() {
	if(timerValue - 1000 > 0) timerValue -= 1000;
	display_lcd.setCursor(0, 0);
	char *display_val = timer_get_display_time(timerValue);
	display_lcd.print(display_val);
	free(display_val);
}

void setTimerValueUp(long diff) {
	timerValue += diff;
	display_lcd.setCursor(0, 0);
	char *display_val = timer_get_display_time(timerValue);
	display_lcd.print(display_val);
	free(display_val);
}

void setTimerValueDown(long diff) {
	if(timerValue - diff > 0) timerValue -= diff;
	display_lcd.setCursor(0, 0);
	char *display_val = timer_get_display_time(timerValue);
	display_lcd.print(display_val);
	free(display_val);
}


void startSettingTimeValue(long value, const char *displayName) {
	timerValue = value;

	display_lcd.clear();
	display_lcd.setCursor(0, 0);
	char *display_val = timer_get_display_time(timerValue);
	display_lcd.print(display_val);
	free(display_val);
	display_lcd.setCursor(0, 1);
	display_lcd.print(displayName);
}

int decimalVal;
void setDecimalValueUp() {
	decimalVal += 1;
	display_lcd.setCursor(0, 0);
	display_lcd.print("                ");
	display_lcd.setCursor(0, 0);
	display_lcd.print(decimalVal);
}

void setDecimalValueDown() {

	if(decimalVal > 0) decimalVal -= 1;
	display_lcd.setCursor(0, 0);
	display_lcd.print("                ");
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

void saveTimerParameters(long runTime, long stopTime) {
	int check_memory = MEMORY_CHECK_VALUE;
	timerParameters savedTimerParams = {
		runTime, // run time
		stopTime // stop time
	};
	EEPROM.put(0, check_memory);
	EEPROM.put(sizeof(int), savedTimerParams);
}

void updateBatteryIndicator(int value,  int motorNo, bool running, bool disabled, bool reachable) {

	menu_set_battery_level(value, motorNo, running, disabled, reachable);
}

void updateInputMotorParameters() {
	if(menu_get_current() == MENU_GET_MOTOR_STATUS) {
		char displayMenu = menu_get_display();
		char displayName[16] = "";
		switch(displayMenu) {
			case MENU_SET_MOTOR_SPEED:
				sprintf(displayName, "SPEED %d", selectedMotorNumber);
				startSettingDecimalValue(settingParameters.maxSpeed, displayName);
				break;
			case MENU_SET_MOTOR_ACCELERATION:
				sprintf(displayName, "ACCELERATION %d", selectedMotorNumber);
				startSettingDecimalValue(settingParameters.acceleration, displayName);
				break;

			case MENU_SET_MOTOR_DECELERATION:
				sprintf(displayName, "DECELER. %d (%%)", selectedMotorNumber);
				startSettingDecimalValue(settingParameters.decelerationPercentage, displayName);
				break;

			case MENU_SET_MOTOR_CS:
				sprintf(displayName, "CURRENT %d", selectedMotorNumber);
				startSettingDecimalValue(settingParameters.csThreshold, displayName);
				break;

			case MENU_SET_MOTOR_CHANGE_TIME:
				sprintf(displayName, "CHANGE TIME %d", selectedMotorNumber);
				startSettingTimeValue(settingParameters.changeDirTime, displayName);
				break;
			case MENU_GET_MOTOR_STATUS:
				displayMotorStatus();
				break;
		}
	}
}

void rc_wirelessMessageSendFailed(int address) {
	updateBatteryIndicator(0, address, false, false, false);
}

void rc_wirelessMessageAckReceived(wirelessMessageResponse message) {

	if((message.type == MESSAGE_GET_PARAMS || message.type == MESSAGE_MOTOR_STATUS) &&
		menu_get_current() == MENU_GET_MOTOR_STATUS &&
		selectedMotorNumber == message.motorModuleNumber) {

		settingParameters = message.parameters;
		updateInputMotorParameters();
	}

	if(message.type == MESSAGE_MOTOR_STATUS
		&& updatingMotorStatus
		&& selectedMotorNumber == message.motorModuleNumber
		&& menu_get_current() == MENU_GET_MOTOR_STATUS) {

		currentMotorStatus = message.status;
		displayMotorStatus();
	}

	if(
		message.type == MESSAGE_MOTOR_STATUS && 
		(menu_get_display() == MENU_TIMER ||
		menu_get_display() == MENU_MANUAL)
		) {


		s_print("motor: ");
		s_print(message.motorModuleNumber);
		s_print(" battery: ");
		s_print(message.status.battery);
		s_print(" speed: ");
		s_print(message.status.speed);
		s_print(" disabled: ");
		s_println(message.status.disabled);

		updateBatteryIndicator(message.status.battery, message.motorModuleNumber, message.status.speed > 0, message.status.disabled, true);
	}
}

void ok_pressed() {

	char currentMenu = menu_get_current();
	if(currentMenu == MENU_ROOT) {

		char displayMenu = menu_get_display();
		if(displayMenu == MENU_MANUAL && !timer_is_paused()) {
			display_show_message("SWITCHED", "TO MANUAL MODE", 1500);
			if(!timer_is_paused()) {
				timer_stop();
			}
		} else if (displayMenu == MENU_TIMER && timer_is_paused()) {
			display_show_message("SWITCHED", "TO TIMER MODE", 1500);
			if(timer_is_paused()) {
				timer_start();
			}
		}
	} else if(currentMenu == MENU_MANUAL_SET_RUN_TIME) {
		char displayMenu = menu_get_display();
		if(displayMenu == MENU_MANUAL_SET_RUN_TIME) {
			if(timerValue != timer_get_run_time()) {
				timer_set_run_time(timerValue);
				saveTimerParameters(timer_get_run_time(), timer_get_stop_time());
			}
			menu_down();
			startSettingTimeValue(timer_get_stop_time(), "MOTOR STOP TIME");
		} else {
			if(timerValue != timer_get_stop_time()) {
				timer_set_stop_time(timerValue);
				saveTimerParameters(timer_get_run_time(), timer_get_stop_time());
			}
			menu_down();
			startSettingTimeValue(timer_get_run_time(), "MOTOR RUN TIME");
		}
	} else if(currentMenu == MENU_GET_MOTOR_STATUS) {
		char displayMenu = menu_get_display();
		// Save previously set item
		bool updateParams = true;
		switch(displayMenu) {
			case MENU_SET_MOTOR_SPEED:
				if(decimalVal == settingParameters.maxSpeed) updateParams = false;
				settingParameters.maxSpeed = decimalVal;
				break;
			case MENU_SET_MOTOR_ACCELERATION:
				if(decimalVal == settingParameters.acceleration) updateParams = false;
				settingParameters.acceleration = decimalVal;
				break;
			case MENU_SET_MOTOR_DECELERATION:
				if(decimalVal == settingParameters.decelerationPercentage) updateParams = false;
				settingParameters.decelerationPercentage = decimalVal;
				break;
			case MENU_SET_MOTOR_CS:
				if(decimalVal == settingParameters.csThreshold) updateParams = false;
				settingParameters.csThreshold = decimalVal;
				break;
			case MENU_SET_MOTOR_CHANGE_TIME:
				if(decimalVal == settingParameters.changeDirTime) updateParams = false;
				settingParameters.changeDirTime = timerValue;
				break;
			default:
				updateParams = false;
				break;
		}

		if(updateParams) {
			wirelessMessageCommand msg;
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

	if(current == MENU_GET_MOTOR_STATUS) {
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

	if(current == MENU_GET_MOTOR_STATUS) {
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
	if(currentMenu == MENU_MANUAL_SET_RUN_TIME || currentMenu == MENU_GET_MOTOR_STATUS) {
		updatingMotorStatus = false;
		menu_use(MENU_ROOT);
		menu_down();
		// Move to auto timer mode if timer is running
		if(!timer_is_paused()) {
			menu_down();
		}

		return;
	}

	wirelessMessageCommand msg;
	bool validCommand = false;
	char displayMenu = menu_get_display();

	if(displayMenu == MENU_MANUAL) {
		if(timer_is_running()) {
			display_show_message("STOPPING", "ALL MOTORS", 1500);
			msg.type = MESSAGE_STOP;
			timer_stop();
			timer_set_state(false);
			sentStopAll = true;
			validCommand = true;
		} else {
			display_show_message("STARTING", "ALL MOTORS", 1500);
			msg.type = MESSAGE_START;
			timer_stop();
			timer_set_state(true);
			sentStartAll = true;
			validCommand = true;
		}

	} else if(displayMenu == MENU_TIMER) {
		if(timer_is_running()) {
			display_show_message("STOPPING", "ALL MOTORS TIMER", 1500);
			timer_set_state(false);
			if(timer_is_paused()) timer_start();
			msg.type = MESSAGE_STOP;
			sentStopAll = true;
			validCommand = true;
		} else {
			display_show_message("STARTING", "ALL MOTORS TIMER", 1500);
			timer_set_state(true);
			if(timer_is_paused()) timer_start();
			msg.type = MESSAGE_START;
			sentStartAll = true;
			validCommand = true;
		}
	}

	if(validCommand) {
		wireless_send_message_all(msg);
		lastUpdateBatteryTime = millis();
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

	if(menu_get_current() != MENU_ROOT) {
		// First, exit to main menu
		start_stop_all_pressed();
	}

	menu_right();
	menu_select();
	selectedMotorNumber = motorNo;
	wirelessMessageCommand msg;
	msg.type = MESSAGE_GET_PARAMS;
	wireless_send_message(motorNo, msg);
	updateInputMotorParameters();

}

bool isMenuAboutMotorInfo(char displayMenu) {
	return (displayMenu == MENU_SET_MOTOR_CHANGE_TIME ||
			displayMenu == MENU_SET_MOTOR_CHANGE_TIME ||
			displayMenu == MENU_MANUAL_SET_STOP_TIME ||
			displayMenu == MENU_GET_MOTOR_STATUS);
}

bool isMenuAboutCurrentMotorInfo(char displayMenu, int motorNo) {

	return ((displayMenu == MENU_SET_MOTOR_CHANGE_TIME ||
			displayMenu == MENU_SET_MOTOR_CHANGE_TIME ||
			displayMenu == MENU_MANUAL_SET_STOP_TIME ||
			displayMenu == MENU_GET_MOTOR_STATUS) && 
			selectedMotorNumber == motorNo);
}

void keyReleased(char key) {

	sentStartAll = false;
	sentStopAll = false;

	wirelessMessageCommand msg;
	char currentMenu = menu_get_current();
	char displayMenu = menu_get_display();
	switch(key) {
		case '1':
			if(currentMenu == MENU_ROOT || isMenuAboutCurrentMotorInfo(displayMenu, WIRELESS_MODULE_1)) {
				msg.type = MESSAGE_TOGGLE_START_STOP;
				wireless_send_message(WIRELESS_MODULE_1, msg);
			} else if(isMenuAboutMotorInfo(displayMenu)) {
				start_program_motor(WIRELESS_MODULE_1);
			}
			break;
		case '4':
			// if(displayMenu == MENU_SET_MOTOR_CHANGE_TIME || 
			// 	displayMenu == MENU_MANUAL_SET_RUN_TIME ||
			// 	displayMenu == MENU_MANUAL_SET_STOP_TIME) {
			// 	setTimerValueUp(60000);
			// }
			if(currentMenu == MENU_ROOT) {
				msg.type = MESSAGE_CHANGE_DIRECTION;
				wireless_send_message(WIRELESS_MODULE_1, msg);
			}
			break;
		case '2':
			if(currentMenu == MENU_ROOT || isMenuAboutCurrentMotorInfo(displayMenu, WIRELESS_MODULE_2)) {
				msg.type = MESSAGE_TOGGLE_START_STOP;
				wireless_send_message(WIRELESS_MODULE_2, msg);
			} else if(isMenuAboutMotorInfo(displayMenu)) {
				start_program_motor(WIRELESS_MODULE_2);
			}
			break;
		case '5':
			if(currentMenu == MENU_ROOT) {
				msg.type = MESSAGE_CHANGE_DIRECTION;
				wireless_send_message(WIRELESS_MODULE_2, msg);
			}
			break;
		case '3':
			if(currentMenu == MENU_ROOT || isMenuAboutCurrentMotorInfo(displayMenu, WIRELESS_MODULE_3)) {
				msg.type = MESSAGE_TOGGLE_START_STOP;
				wireless_send_message(WIRELESS_MODULE_3, msg);
			} else if(isMenuAboutMotorInfo(displayMenu)) {
				start_program_motor(WIRELESS_MODULE_3);
			}
			break;
		case '6':
			if(currentMenu == MENU_ROOT) {
				msg.type = MESSAGE_CHANGE_DIRECTION;
				wireless_send_message(WIRELESS_MODULE_3, msg);
			}
			break;
		case '*':
			if(currentMenu == MENU_ROOT || isMenuAboutCurrentMotorInfo(displayMenu, WIRELESS_MODULE_4)) {
				msg.type = MESSAGE_TOGGLE_START_STOP;
				wireless_send_message(WIRELESS_MODULE_4, msg);
			} else if(isMenuAboutMotorInfo(displayMenu)) {
				start_program_motor(WIRELESS_MODULE_4);
			}
			break;
		case '7':
			// if(displayMenu == MENU_SET_MOTOR_CHANGE_TIME || 
			// 	displayMenu == MENU_MANUAL_SET_RUN_TIME ||
			// 	displayMenu == MENU_MANUAL_SET_STOP_TIME) {
			// 	setTimerValueDown(60000);
			// }
			if(currentMenu == MENU_ROOT) {
				msg.type = MESSAGE_CHANGE_DIRECTION;
				wireless_send_message(WIRELESS_MODULE_4, msg);
			}
			break;
		case '0':
			if(currentMenu == MENU_ROOT || isMenuAboutCurrentMotorInfo(displayMenu, WIRELESS_MODULE_5)) {
				msg.type = MESSAGE_TOGGLE_START_STOP;
				wireless_send_message(WIRELESS_MODULE_5, msg);
			} else if(isMenuAboutMotorInfo(displayMenu)) {
				start_program_motor(WIRELESS_MODULE_5);
			}
			break;
		case '8':
			if(currentMenu == MENU_ROOT) {
				msg.type = MESSAGE_CHANGE_DIRECTION;
				wireless_send_message(WIRELESS_MODULE_5, msg);
			}
			break;
		case '#':
			if(currentMenu == MENU_ROOT || isMenuAboutCurrentMotorInfo(displayMenu, WIRELESS_MODULE_6)) {
				msg.type = MESSAGE_TOGGLE_START_STOP;
				wireless_send_message(WIRELESS_MODULE_6, msg);
			} else if(isMenuAboutMotorInfo(displayMenu)) {
				start_program_motor(WIRELESS_MODULE_6);
			}
			break;
		case '9':
			if(currentMenu == MENU_ROOT) {

				msg.type = MESSAGE_CHANGE_DIRECTION;
				wireless_send_message(WIRELESS_MODULE_6, msg);
			}
			break;
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
			break;
		default:
			//do nothing
			break;
	}
}

void keypadListener(Key *keys, int keysLen) {

	if(keysLen == 1) {
		Key key = keys[0];
		if(key.kstate == RELEASED) {
			if(!keypad_is_keyhold(key)) {
				keyReleased(key.kchar);
			}
			keypad_remove_keyhold(key);
		}
		// OK pressed for 5 seconds
		if(key.kstate == HOLD && key.kchar == 'D') {
			keypad_add_keyhold(key);
			start_program_timer();
		}
	} else if(keysLen == 2) {
		bool okHold = false;
		for(int i = 0; i<keysLen; i++) {
			if(keys[i].kchar == 'D' && keys[i].kstate == HOLD) {
				keypad_add_keyhold(keys[i]);
				okHold = true;
				break;
			}
		}

		for(int i = 0; i<keysLen; i++) {
			if(keys[i].kstate == HOLD) {
				keypad_add_keyhold(keys[i]);
				switch(keys[i].kchar) {
					case '1':
					case '2':
					case '3':
						if(okHold) {
							int motorNumber = keys[i].kchar - '0';
							start_program_motor(motorNumber);
						}
						break;
					case '*':
						if(okHold) {
							start_program_motor(4);	
						}
						break;
					case '0':
						if(okHold) {
							start_program_motor(5);	
						}
					break;
					case '#':
						if(okHold) {
							start_program_motor(6);	
						}
					break;
				}
			}
		}
	}
}

void get_all_motor_status() {
	if(!wireless_send_is_busy()) {

		// Use parameters in message struct to send current state of start / stop all 
		// Motor uses this to stop if not stopped already or to start if motor isn't already in that state
		statusMsg.parameters.maxSpeed = sentStartAll ? 1 : 0;
		statusMsg.parameters.acceleration = sentStopAll ? 1 : 0;
		wireless_send_message_all(statusMsg);
	}
}

void _setup() {

#ifdef DEBUG
	Serial.begin(9600);
#endif
	s_println(F("Remote control program running"));
	_wireless_setup_remote(A0, A1);
	wireless_listen_ack(rc_wirelessMessageAckReceived);
	wireless_send_message_all_fail_listener(rc_wirelessMessageSendFailed);
	_keypad_setup(keypadListener);
	_display_setup();
	_menu_setup();

	statusMsg.type = MESSAGE_MOTOR_STATUS;

	lastUpdateBatteryTime = 0;
	timerParameters savedTimerParams;
	int check_saved_memory;
	EEPROM.get(0, check_saved_memory);
	if(check_saved_memory == MEMORY_CHECK_VALUE) {
		s_println(F("Found parameters saved in EEPROM"));
		EEPROM.get(sizeof(int), savedTimerParams);
	} else {
		s_println(F("Didn't find parameters saved in EEPROM, initializing..."));
		saveTimerParameters(30000L, 690000L);
	}

	_timer_setup(timerStateChanged);
	timer_set_run_time(savedTimerParams.runTime);
	timer_set_stop_time(savedTimerParams.stopTime);
	get_all_motor_status();
}


long displayFreeMemoryMs;

void _loop() {
	long milliseconds = millis();
	_wireless_loop(milliseconds);
	_keypad_loop(milliseconds);
	_timer_loop(milliseconds);
	_menu_loop(milliseconds);
	_display_loop(milliseconds);

	if(updatingMotorStatus &&
		milliseconds - lastUpdateStatusTime >= UPDATE_MOTOR_STATUS_MS) {

		wireless_send_message(selectedMotorNumber, statusMsg);
		lastUpdateStatusTime = milliseconds;
	}

	if((menu_get_display() == MENU_MANUAL || menu_get_display() == MENU_TIMER) && 
		milliseconds - lastUpdateBatteryTime >= UPDATE_BATTERY_LEVEL_MS) {
		get_all_motor_status();
		lastUpdateBatteryTime = milliseconds;
	}

#ifdef DEBUG
	if(milliseconds - displayFreeMemoryMs >= 1000) {
		displayFreeMemoryMs = milliseconds;
		s_print("freeMemory()=");
		s_println(freeMemory());
	}
#endif
}

#endif //MOTOR_REMOTE_CONTROLLER