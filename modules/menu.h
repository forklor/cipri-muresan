#ifndef MENU_H_
#define MENU_H_

//extern MenuBackend menu;

#define MENU_ROOT '0'
#define MENU_MANUAL '1'
#define MENU_TIMER '4'

#define MENU_MANUAL_SET_RUN_TIME '7'
#define MENU_MANUAL_SET_STOP_TIME '8'


void menu_up();
void menu_down();
void menu_right();
void menu_left();
void menu_select();
void menu_use(char menu);

char menu_get_current();
char menu_get_display();

void _menu_setup();
void _menu_loop(long milliseconds);

#endif //MENU_H_