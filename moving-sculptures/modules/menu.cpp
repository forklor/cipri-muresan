#include <Arduino.h>
#include "menu.h"
#include "display.h"
#include "timer.h"

#include <MenuBackend.h>

#include "print.h"

void updateDisplay();

int bat1;
int bat2;
int bat3;
int bat4;
int bat5;
int bat6;

#define MOTOR_REACHABLE_MAX 5

bool motor1_running;
bool motor2_running;
bool motor3_running;
bool motor4_running;
bool motor5_running;
bool motor6_running;

bool motor1_disabled;
bool motor2_disabled;
bool motor3_disabled;
bool motor4_disabled;
bool motor5_disabled;
bool motor6_disabled;

byte motor1_reachable_value = 0;
byte motor2_reachable_value = 0;
byte motor3_reachable_value = 0;
byte motor4_reachable_value = 0;
byte motor5_reachable_value = 0;
byte motor6_reachable_value = 0;

bool motor1_reachable;
bool motor2_reachable;
bool motor3_reachable;
bool motor4_reachable;
bool motor5_reachable;
bool motor6_reachable;


int UPDATE_DISPLAY_TIME_MS;

char selectedMenu;

void menuUseEventListener(MenuUseEvent used) {
	s_print(F("Menu use "));
	s_print(used.item.getName());
	s_println(used.item.getShortkey());
	selectedMenu = used.item.getShortkey();
	updateDisplay();
}

long lastStepMs;

void menuChangeEventListener(MenuChangeEvent changed) {

	updateDisplay();
	// s_print(F("Menu change "));
	// s_print(changed.from.getName());
	// s_print(F(" "));
	// s_println(changed.to.getName());
}

MenuBackend menu = MenuBackend(menuUseEventListener, menuChangeEventListener);

MenuItem mainTimer = MenuItem("AutoTimer", MENU_TIMER);
MenuItem mainManual = MenuItem("Manual", MENU_MANUAL);

MenuItem manualRunTime = MenuItem("R", MENU_MANUAL_SET_RUN_TIME);
MenuItem manualStopTime = MenuItem("S", MENU_MANUAL_SET_STOP_TIME);

MenuItem setMotorSpeed = MenuItem("MSP", MENU_SET_MOTOR_SPEED);
MenuItem setMotorAccelertion = MenuItem("MA", MENU_SET_MOTOR_ACCELERATION);
MenuItem setMotorDeceleration = MenuItem("MD", MENU_SET_MOTOR_DECELERATION);
MenuItem setMotorCs = MenuItem("MCS", MENU_SET_MOTOR_CS);
MenuItem setMotorTime = MenuItem("MT", MENU_SET_MOTOR_CHANGE_TIME);
MenuItem getMotorStatus = MenuItem("MS", MENU_GET_MOTOR_STATUS);

void displayBatteryStatus(int value, int motorNo, bool running, bool disabled, bool reachable) {

	int x = 12 + motorNo % 4 + ((int)motorNo / 4);
	int y = motorNo / 4;
	float voltage = value;

	float level0 = 650, 
		level1 = 658,
		level2 = 666,
		level3 = 674,
		level4 = 682,
		level5 = 690;

	if(running) {
		level0 = 535;
		level1 = 559;
		level2 = 584;
		level3 = 609;
		level4 = 634;
		level5 = 659;
	}


	//s_print("motor=");
	//s_print(motorNo);
	//s_print(" voltage=");
	//s_println(voltage);

	if(!reachable) {
		byte batlevel[8] = {
			B10001,
			B01010,
			B01010,
			B00100,
			B00100,
			B01010,
			B01010,
			B10001,
		};
		display_lcd.createChar(motorNo - 1, batlevel);
		display_lcd.setCursor(x, y);
		display_lcd.write(byte(motorNo - 1));
	} else if(disabled) {
		byte batlevel[8] = {
			B01110,
			B11111,
			B10101,
			B10001,
			B11011,
			B11011,
			B11111,
			B11111,
		};
		display_lcd.createChar(motorNo - 1, batlevel);
		display_lcd.setCursor(x, y);
		display_lcd.write(byte(motorNo - 1));
	} else if(voltage > level5) {
		byte batlevel[8] = {
			B01110,
			B11111,
			B11111,
			B11111,
			B11111,
			B11111,
			B11111,
			B11111,
		};
		display_lcd.createChar(motorNo - 1, batlevel);
		display_lcd.setCursor(x, y);
		display_lcd.write(byte(motorNo - 1));
	 }else if(voltage <= level5 && voltage > level4) {
		byte batlevel[8] = {
			B01110,
			B10001,
			B11111,
			B11111,
			B11111,
			B11111,
			B11111,
			B11111,
		};
		display_lcd.createChar(motorNo - 1, batlevel);
		display_lcd.setCursor(x, y);
		display_lcd.write(byte(motorNo - 1));
	} else if(voltage <= level4 && voltage > level3) {
		byte batlevel[8] = {
			B01110,
			B10001,
			B10001,
			B11111,
			B11111,
			B11111,
			B11111,
			B11111,
		};
		display_lcd.createChar(motorNo - 1, batlevel);
		display_lcd.setCursor(x, y);
		display_lcd.write(byte(motorNo - 1));
	} else if(voltage <= level3 && voltage > level2) {
		byte batlevel[8] = {
			B01110,
			B10001,
			B10001,
			B10001,
			B11111,
			B11111,
			B11111,
			B11111,
		};
		display_lcd.createChar(motorNo - 1, batlevel);
		display_lcd.setCursor(x, y);
		display_lcd.write(byte(motorNo - 1));
	} else if(voltage <= level2 && voltage > level1) {
		byte batlevel[8] = {
			B01110,
			B10001,
			B10001,
			B10001,
			B10001,
			B11111,
			B11111,
			B11111,
		};
		display_lcd.createChar(motorNo - 1, batlevel);
		display_lcd.setCursor(x, y);
		display_lcd.write(byte(motorNo - 1));
  	} else if(voltage <= level1 && voltage > level0) {
		byte batlevel[8] = {
			B01110,
			B10001,
			B10001,
			B10001,
			B10001,
			B00001,
			B11111,
			B11111,
		};
		display_lcd.createChar(motorNo - 1, batlevel);
		display_lcd.setCursor(x, y);
		display_lcd.write(byte(motorNo - 1));
  	} else if(voltage <= level0) {
		byte batlevel[8] = {
			B01110,
			B10001,
			B10001,
			B10001,
			B10001,
			B10001,
			B10001,
			B11111,
		};
		display_lcd.createChar(motorNo - 1, batlevel);
		display_lcd.setCursor(x, y);
		display_lcd.write(byte(motorNo - 1));
	}
}

void updateDisplay() {

	if(menu.getCurrent().getShortkey() == MENU_TIMER || menu.getCurrent().getShortkey() == MENU_MANUAL) {
		display_lcd.clear();
		display_lcd.setCursor(0, 0);

		if(!timer_is_paused()) {
			UPDATE_DISPLAY_TIME_MS = 1000;
			if(timer_is_running()) {
				char *display_val = timer_get_display_time(timer_get_current_time());
				display_lcd.print(display_val);
				free(display_val);
				display_lcd.setCursor(7, 0);
				display_val = timer_get_display_time(timer_get_stop_time());
				display_lcd.print(display_val);
				free(display_val);
			} else {
				char *display_val = timer_get_display_time(timer_get_run_time());
				display_lcd.print(display_val);
				free(display_val);
				display_lcd.setCursor(7, 0);
				display_val = timer_get_display_time(timer_get_current_time());
				display_lcd.print(display_val);
				free(display_val);
			}
		} else {
			UPDATE_DISPLAY_TIME_MS = 30000;
			char *display_val = timer_get_display_time(timer_get_run_time());
			display_lcd.print(display_val);
			free(display_val);

			display_lcd.setCursor(7, 0);
			
			display_val = timer_get_display_time(timer_get_stop_time());
			display_lcd.print(display_val);
			free(display_val);
		}

		display_lcd.setCursor(0, 1);
		char *displayMenuName = (char *)menu.getCurrent().getName();
		char *display = (char *)malloc((strlen(displayMenuName) + 4) * sizeof(char));
		sprintf(display, "%s %s", displayMenuName, timer_is_running() ? "ON" : "OFF");
		display_lcd.print(display);
		free(display);

		displayBatteryStatus(bat1, 1, motor1_running, motor1_disabled, motor1_reachable);
		displayBatteryStatus(bat2, 2, motor2_running, motor2_disabled, motor2_reachable);
		displayBatteryStatus(bat3, 3, motor3_running, motor3_disabled, motor3_reachable);
		displayBatteryStatus(bat4, 4, motor4_running, motor4_disabled, motor4_reachable);
		displayBatteryStatus(bat5, 5, motor5_running, motor5_disabled, motor5_reachable);
		displayBatteryStatus(bat6, 6, motor6_running, motor6_disabled, motor6_reachable);
	}
}

void menu_up() {
	s_print("menu move up ");
	s_println(menu.getCurrent().getName());
	menu.moveUp();
	updateDisplay();
}

void menu_down() {
	s_print("menu move down ");
	s_println(menu.getCurrent().getName());
	menu.moveDown();
	updateDisplay();
}

void menu_right() {
	s_print("menu move right ");
	s_println(menu.getCurrent().getName());
	menu.moveRight();
	updateDisplay();
}

void menu_left() {
	s_print("menu move left ");
	s_println(menu.getCurrent().getName());
	menu.moveLeft();
	updateDisplay();
}

void menu_select() {
	menu.use();
}

void menu_use(char menuName) {
	menu.use(menuName);
}

void _menu_setup() {

	UPDATE_DISPLAY_TIME_MS = 1000;
	selectedMenu = MENU_ROOT;
	lastStepMs = millis();

	bat1 = 0;
	bat2 = 0;
	bat3 = 0;
	bat4 = 0;
	bat5 = 0;
	bat6 = 0;

	motor1_running = false;
	motor2_running = false;
	motor3_running = false;
	motor4_running = false;
	motor5_running = false;
	motor6_running = false;

	motor1_disabled = false;
	motor2_disabled = false;
	motor3_disabled = false;
	motor4_disabled = false;
	motor5_disabled = false;
	motor6_disabled = false;

	motor1_reachable = true;
	motor2_reachable = true;
	motor3_reachable = true;
	motor4_reachable = true;
	motor5_reachable = true;
	motor6_reachable = true;

	menu.getRoot().add(mainManual);
	mainManual.addAfter(mainTimer);
	mainTimer.addAfter(mainManual);

	mainManual.addLeft(manualRunTime);
	mainTimer.addLeft(manualRunTime);
	manualRunTime.addAfter(manualStopTime);
	manualStopTime.addAfter(manualRunTime);

	mainManual.addRight(getMotorStatus);
	mainTimer.addRight(getMotorStatus);

	getMotorStatus.addAfter(setMotorSpeed);
	setMotorSpeed.addAfter(setMotorAccelertion);
	setMotorAccelertion.addAfter(setMotorDeceleration);
	setMotorDeceleration.addAfter(setMotorCs);
	setMotorCs.addAfter(setMotorTime);
	setMotorTime.addAfter(getMotorStatus);
	
	menu.moveDown();
}

byte updateReachableValue(byte reachable_value, bool reachable) {

	if(reachable) {
		return MOTOR_REACHABLE_MAX;
	}

	if(!reachable && reachable_value > 0) {
		return reachable_value - 1;
	}

	return reachable_value;
}

void menu_set_battery_level(int level, int motorNo, bool running, bool disabled, bool reachable) {

	bool canUpdate = !display_has_message();
	switch (motorNo) {
		case 1:
			motor1_reachable_value = updateReachableValue(motor1_reachable_value, reachable);
			motor1_reachable = motor1_reachable_value > 0;
			if(reachable) {
				bat1 = level;
				motor1_running = running;
				motor1_disabled = disabled;
			}
			if(canUpdate) {
				displayBatteryStatus(bat1, 1, motor1_running, motor1_disabled, motor1_reachable);
			}
			break;
		case 2:
			if(reachable) {
				bat2 = level;
				motor2_running = running;
				motor2_disabled = disabled;
			}
			motor2_reachable_value = updateReachableValue(motor2_reachable_value, reachable);
			motor2_reachable = motor2_reachable_value > 0;
			if(canUpdate) {
				displayBatteryStatus(bat2, 2, motor2_running, motor2_disabled, motor2_reachable);
			}
			break;
		case 3:
			if(reachable) {
				bat3 = level;
				motor3_running = running;
				motor3_disabled = disabled;
			}
			motor3_reachable_value = updateReachableValue(motor3_reachable_value, reachable);
			motor3_reachable = motor3_reachable_value > 0;
			if(canUpdate) {
				displayBatteryStatus(bat3, 3, motor3_running, motor3_disabled, motor3_reachable);
			}

			break;
		case 4:
			if(reachable) {
				bat4 = level;
				motor4_running = running;
				motor4_disabled = disabled;
			}
			motor4_reachable_value = updateReachableValue(motor4_reachable_value, reachable);
			motor4_reachable = motor4_reachable_value > 0;
			if(canUpdate) {
				displayBatteryStatus(bat4, 4, motor4_running, motor4_disabled, motor4_reachable);
			}
			break;
		case 5:
			if(reachable) {
				bat5 = level;
				motor5_running = running;
				motor5_disabled = disabled;
			}
			motor5_reachable_value = updateReachableValue(motor5_reachable_value, reachable);
			motor5_reachable = motor5_reachable_value > 0;
			if(canUpdate) {
				displayBatteryStatus(bat5, 5, motor5_running, motor5_disabled, motor5_reachable);
			}
			break;
		case 6:
			if(reachable) {
				bat6 = level;
				motor6_running = running;
				motor6_disabled = disabled;
			}
			motor6_reachable_value = updateReachableValue(motor6_reachable_value, reachable);
			motor6_reachable  = motor6_reachable_value > 0;
			if(canUpdate) {
				displayBatteryStatus(bat6, 6, motor6_running, motor6_disabled, motor6_reachable);
			}
			break;
		default:
			break;
	}
}

char menu_get_current() {
	return selectedMenu;
}

char menu_get_display() {
	return menu.getCurrent().getShortkey();
}

void _menu_loop(long milliseconds) {

	if(display_has_message()) {
		UPDATE_DISPLAY_TIME_MS = 1000;
		return;
	}

	if(milliseconds - lastStepMs >= UPDATE_DISPLAY_TIME_MS) {
		updateDisplay();
		lastStepMs = milliseconds;
	}
}