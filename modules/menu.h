#ifndef MENU_H_
#define MENU_H_

//extern MenuBackend menu;

#define MENU_MANUAL_OFF '1'
#define MENU_MANUAL_ON '2'
#define MENU_TIMER_ON '3'
#define MENU_TIMER_OFF '3'

void menu_up();
void menu_down();
void menu_right();
void menu_left();
void menu_select();
void menu_exit();

char menu_get_current();

void _menu_setup();
void _menu_loop(long milliseconds);

#endif //MENU_H_