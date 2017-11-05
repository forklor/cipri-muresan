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
MenuItem mainTimer = MenuItem("Main Timer", '0');
MenuItem mainTimerSetRunningTime = MenuItem("Main Timer Run Time");
MenuItem mainTimerSetStopTime = MenuItem("Main Timer Stop Time");
MenuItem manualTimer = MenuItem("Main Manual");
// MenuItem debug = MenuItem("Debug");
// MenuItem options = MenuItem("Options");
// 	MenuItem setDelay = MenuItem("Delay",'D');
// 		MenuItem d100 = MenuItem("100 ms");
// 		MenuItem d200 = MenuItem("200 ms");
// 		MenuItem d300 = MenuItem("300 ms");
// 		MenuItem d400 = MenuItem("400 ms");


void updateDisplay() {
	display_lcd.clear();
	display_lcd.setCursor(0, 0);

	if(timer_is_running()) {
		display_lcd.print(timer_get_display_time(timer_get_current_time()));
		display_lcd.setCursor(8, 0);
		display_lcd.print(timer_get_display_time(timer_get_stop_time()));
	} else {
		display_lcd.print(timer_get_display_time(timer_get_run_time()));
		display_lcd.setCursor(8, 0);
		display_lcd.print(timer_get_display_time(timer_get_current_time()));
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

void menu_select() {
	menu.use();
}

void menu_exit() {
	menu.use('0');
}

void _menu_setup() {

	lastStepMs = millis();
	menu.getRoot().add(mainTimer);
	mainTimer.addRight(mainTimerSetRunningTime);
	mainTimerSetRunningTime.addAfter(mainTimerSetStopTime);
	mainTimerSetStopTime.addAfter(mainTimerSetRunningTime);
	mainTimer.addAfter(manualTimer);
	manualTimer.addAfter(mainTimer);
}

void _menu_loop(long milliseconds) {
	if(milliseconds - lastStepMs >= UPDATE_DISPLAY_TIME_MS) {
		updateDisplay();
		lastStepMs = milliseconds;
	}
}