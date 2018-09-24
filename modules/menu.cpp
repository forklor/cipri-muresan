#include <Arduino.h>
#include "menu.h"
#include "display.h"
#include "timer.h"

#define UPDATE_DISPLAY_TIME_MS 1000

#include <MenuBackend.h>

void updateDisplay();

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
	Serial.print(F("Menu change "));
	Serial.print(changed.from.getName());
	Serial.print(F(" "));
	Serial.println(changed.to.getName());
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

void updateDisplay() {

	if(menu.getCurrent().getShortkey() == MENU_TIMER || menu.getCurrent().getShortkey() == MENU_MANUAL) {
		display_lcd.clear();
		display_lcd.setCursor(0, 0);

		if(!timer_is_paused()) {
			if(timer_is_running()) {
				char *display_val = timer_get_display_time(timer_get_current_time());
				display_lcd.print(display_val);
				free(display_val);
				display_lcd.setCursor(8, 0);
				display_val = timer_get_display_time(timer_get_stop_time());
				display_lcd.print(display_val);
				free(display_val);
			} else {
				char *display_val = timer_get_display_time(timer_get_run_time());
				display_lcd.print(display_val);
				free(display_val);
				display_lcd.setCursor(8, 0);
				display_val = timer_get_display_time(timer_get_current_time());
				display_lcd.print(display_val);
				free(display_val);
			}
		} else {
			char *display_val = timer_get_display_time(timer_get_run_time());
			display_lcd.print(display_val);
			free(display_val);

			display_lcd.setCursor(8, 0);
			
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

	selectedMenu = MENU_ROOT;
	lastStepMs = millis();

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

char menu_get_current() {
	return selectedMenu;
}

char menu_get_display() {
	return menu.getCurrent().getShortkey();
}

void _menu_loop(long milliseconds) {

	if(display_has_message()) return;
	if(milliseconds - lastStepMs >= UPDATE_DISPLAY_TIME_MS) {
		updateDisplay();
		lastStepMs = milliseconds;
	}
}