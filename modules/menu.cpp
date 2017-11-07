#include <Arduino.h>
#include "menu.h"
#include "display.h"
#include "timer.h"

#define UPDATE_DISPLAY_TIME_MS 500

#include <MenuBackend.h>

void updateDisplay();

char selectedMenu;

void menuUseEventListener(MenuUseEvent used) {
	Serial.print("Menu use ");
	Serial.println(used.item.getName());
	selectedMenu = used.item.getShortkey();
}

long lastStepMs;

void menuChangeEventListener(MenuChangeEvent changed) {

	updateDisplay();
	Serial.print("Menu change ");
	Serial.print(changed.from.getName());
	Serial.print(" ");
	Serial.println(changed.to.getName());
}

MenuBackend menu = MenuBackend(menuUseEventListener, menuChangeEventListener);

MenuItem mainTimer = MenuItem("Auto Timer", MENU_TIMER);
MenuItem mainManual = MenuItem("Manual", MENU_MANUAL);

MenuItem manualRunTime = MenuItem("Set Run Time", MENU_MANUAL_SET_RUN_TIME);
MenuItem manualStopTime = MenuItem("Set Stop Time", MENU_MANUAL_SET_STOP_TIME);

void updateDisplay() {

	if(menu.getCurrent().getShortkey() == MENU_TIMER || menu.getCurrent().getShortkey() == MENU_MANUAL) {
		display_lcd.clear();
		display_lcd.setCursor(0, 0);

		if(!timer_is_paused()) {
			if(timer_is_running()) {
				display_lcd.print(timer_get_display_time(timer_get_current_time()));
				display_lcd.setCursor(8, 0);
				display_lcd.print(timer_get_display_time(timer_get_stop_time()));
			} else {
				display_lcd.print(timer_get_display_time(timer_get_run_time()));
				display_lcd.setCursor(8, 0);
				display_lcd.print(timer_get_display_time(timer_get_current_time()));
			}
		} else {
			display_lcd.print(timer_get_display_time(timer_get_run_time()));
			display_lcd.setCursor(8, 0);
			display_lcd.print(timer_get_display_time(timer_get_stop_time()));
		}

		display_lcd.setCursor(0, 1);
		String state = timer_is_running() ? " ON" : " OFF";
		display_lcd.print(menu.getCurrent().getName() + state);
	}
}


void menu_up() {
	Serial.println("menu move up");
	menu.moveUp();
}

void menu_down() {
	Serial.println("menu move down");
	menu.moveDown();
}

void menu_right() {
	Serial.println("menu move right");
	menu.moveRight();
}

void menu_left() {
	Serial.println("menu move left");
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

	menu.moveDown();
}

char menu_get_current() {
	return selectedMenu;
}

char menu_get_display() {
	Serial.println(menu.getCurrent().getName());
	return menu.getCurrent().getShortkey();
}

void _menu_loop(long milliseconds) {

	if(display_has_message()) return;
	if(milliseconds - lastStepMs >= UPDATE_DISPLAY_TIME_MS) {
		updateDisplay();
		lastStepMs = milliseconds;
	}
}