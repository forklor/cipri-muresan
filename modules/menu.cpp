#include <Arduino.h>
#include "menu.h"
#include "display.h"
#include "timer.h"


#include <MenuBackend.h>

void updateDisplay();

int bat1;
int bat2;
int bat3;
int bat4;
int bat5;
int bat6;

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


int UPDATE_DISPLAY_TIME_MS;

char selectedMenu;

void menuUseEventListener(MenuUseEvent used) {
	Serial.print(F("Menu use "));
	Serial.print(used.item.getName());
	Serial.println(used.item.getShortkey());
	selectedMenu = used.item.getShortkey();
}

long lastStepMs;

void menuChangeEventListener(MenuChangeEvent changed) {

	updateDisplay();
	// Serial.print(F("Menu change "));
	// Serial.print(changed.from.getName());
	// Serial.print(F(" "));
	// Serial.println(changed.to.getName());
}

MenuBackend menu = MenuBackend(menuUseEventListener, menuChangeEventListener);

MenuItem mainTimer = MenuItem("Auto Timer", MENU_TIMER);
MenuItem mainManual = MenuItem("Manual", MENU_MANUAL);

MenuItem manualRunTime = MenuItem("R", MENU_MANUAL_SET_RUN_TIME);
MenuItem manualStopTime = MenuItem("S", MENU_MANUAL_SET_STOP_TIME);

MenuItem setMotorSpeed = MenuItem("MSP", MENU_SET_MOTOR_SPEED);
MenuItem setMotorAccelertion = MenuItem("MA", MENU_SET_MOTOR_ACCELERATION);
MenuItem setMotorDeceleration = MenuItem("MD", MENU_SET_MOTOR_DECELERATION);
MenuItem setMotorCs = MenuItem("MCS", MENU_SET_MOTOR_CS);
MenuItem setMotorTime = MenuItem("MT", MENU_SET_MOTOR_CHANGE_TIME);
MenuItem getMotorStatus = MenuItem("MS", MENU_GET_MOTOR_STATUS);

void displayBatteryStatus(int value, int motorNo, bool running, bool disabled) {

	int x = 12 + motorNo % 4 + ((int)motorNo / 4);
	int y = motorNo / 4;
	float voltage = ((float)value) / 1023 * 5;

	float level1_start, level1_end,
		level2_start, level2_end,
		level3_start, level3_end,
		level4_start, level4_end,
		level5_start, level5_end,
		level6_start, level6_end;

	if(running) {
		level1_start = 0; level1_end = 3.21;
		level2_start = 3.21; level2_end = 3.26;
		level3_start = 3.26; level3_end = 3.32;
		level4_start = 3.32; level4_end = 3.38;
		level5_start = 3.38; level5_end = 3.43;
		level6_start = 3.43; level6_end = 3.46;

	} else {
		level1_start = 0; level1_end = 3.21;
		level2_start = 3.21; level2_end = 3.26;
		level3_start = 3.26; level3_end = 3.32;
		level4_start = 3.32; level4_end = 3.38;
		level5_start = 3.38; level5_end = 3.43;
		level6_start = 3.43; level6_end = 3.46;
	}

	Serial.print("motor=");
	Serial.print(motorNo);
	Serial.print(" voltage=");
	Serial.println(voltage);

	if(disabled) {
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
	} else if(voltage <= level6_end && voltage > level6_start) {
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
	 }else if(voltage <= level5_end && voltage > level5_start) {
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
	} else if(voltage <= level4_end && voltage > level4_start) {
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
	} else if(voltage <= level3_end && voltage > level3_start) {
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
	} else if(voltage <= level2_end && voltage > level2_start) {
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
  	} else if(voltage < level1_end && voltage >= level1_start) {
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
		sprintf(display, "%s %s", displayMenuName, timer_is_running() ? " ON " : " OFF");
		display_lcd.print(display);
		free(display);

		displayBatteryStatus(bat1, 1, motor1_running, motor1_disabled);
		displayBatteryStatus(bat2, 2, motor2_running, motor2_disabled);
		displayBatteryStatus(bat3, 3, motor3_running, motor3_disabled);
		displayBatteryStatus(bat4, 4, motor4_running, motor4_disabled);
		displayBatteryStatus(bat5, 5, motor5_running, motor5_disabled);
		displayBatteryStatus(bat6, 6, motor6_running, motor6_disabled);
	}
}

void menu_up() {
	Serial.print("menu move up ");
	Serial.println(menu.getCurrent().getName());
	menu.moveUp();
}

void menu_down() {
	Serial.print("menu move down ");
	Serial.println(menu.getCurrent().getName());
	menu.moveDown();
}

void menu_right() {
	Serial.print("menu move right ");
	Serial.println(menu.getCurrent().getName());
	menu.moveRight();
}

void menu_left() {
	Serial.print("menu move left ");
	Serial.println(menu.getCurrent().getName());
	menu.moveLeft();
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

void menu_set_battery_level(int level, int motorNo, bool running, bool disabled) {
	switch (motorNo) {
		case 1:
			bat1 = level;
			motor1_running = running;
			motor1_disabled = disabled;
			break;
		case 2:
			bat2 = level;
			motor1_running = running;
			motor1_disabled = disabled;
			break;
		case 3:
			bat3 = level;
			motor2_running = running;
			motor2_disabled = disabled;
			break;
		case 4:
			bat4 = level;
			motor3_running = running;
			motor3_disabled = disabled;
			break;
		case 5:
			bat5 = level;
			motor3_running = running;
			motor3_disabled = disabled;
			break;
		case 6:
			bat6 = level;
			motor3_running = running;
			motor3_disabled = disabled;
			break;
		default:
			break;
	}
	displayBatteryStatus(level, motorNo, running, disabled);
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