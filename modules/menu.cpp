#include <Arduino.h>
#include "menu.h"
#include "display.h"
#include "timer.h"

#define UPDATE_DISPLAY_TIME_MS 500

#include <MenuBackend.h>

void updateDisplay();

/*
	This is an important function
	Here all use events are handled
	
	This is where you define a behaviour for a menu item
*/
void menuUseEventListener(MenuUseEvent used) {
	Serial.print("Menu use ");
	Serial.println(used.item.getName());
}

long lastStepMs;

/*
	This is an important function
	Here we get a notification whenever the user changes the menu
	That is, when the menu is navigated
*/
void menuChangeEventListener(MenuChangeEvent changed) {

	updateDisplay();
	Serial.print("Menu change ");
	Serial.print(changed.from.getName());
	Serial.print(" ");
	Serial.println(changed.to.getName());
}

MenuBackend menu = MenuBackend(menuUseEventListener, menuChangeEventListener);
//beneath is list of menu items needed to build the menu
MenuItem mainTimer = MenuItem("Auto Timer", '0');
MenuItem mainTimerOff = MenuItem("Auto Timer Stop", MENU_TIMER_OFF);
MenuItem mainTimerOn = MenuItem("Auto Timer Off", MENU_TIMER_ON);
MenuItem manualOff = MenuItem("Manual OFF", MENU_MANUAL_OFF);
MenuItem manualOn = MenuItem("Manual ON", MENU_MANUAL_ON);

void updateDisplay() {
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
	display_lcd.print(menu.getCurrent().getName());
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

void menu_exit() {
	menu.use('0');
}

void _menu_setup() {

	lastStepMs = millis();
	menu.getRoot().add(manualOff);
	manualOff.addAfter(mainTimer);
	manualOff.addRight(manualOn);
	manualOn.addRight(manualOff);
	mainTimer.addRight(mainTimerOn);
	mainTimerOn.addAfter(mainTimerOff);
	mainTimer.addAfter(manualOff);

	menu.moveDown();
}

char menu_get_current() {
	return menu.getCurrent().getShortkey();
}

void _menu_loop(long milliseconds) {

	if(display_has_message()) return;
	if(milliseconds - lastStepMs >= UPDATE_DISPLAY_TIME_MS) {
		updateDisplay();
		lastStepMs = milliseconds;
	}
}