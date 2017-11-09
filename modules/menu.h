#ifndef MENU_H_
#define MENU_H_

//extern MenuBackend menu;

#define MENU_ROOT '\0'
#define MENU_MANUAL '1'
#define MENU_TIMER '2'

#define MENU_MANUAL_SET_RUN_TIME '3'
#define MENU_MANUAL_SET_STOP_TIME '4'

#define MENU_SET_MOTOR_SPEED '5'
#define MENU_SET_MOTOR_ACCELERATION '6'
#define MENU_SET_MOTOR_DECELERATION '7'
#define MENU_SET_MOTOR_CS '8'
#define MENU_SET_MOTOR_CHANGE_TIME '9'
#define MENU_GET_MOTOR_STATUS 'A'


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