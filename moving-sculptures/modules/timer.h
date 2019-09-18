#ifndef TIMER_H_
#define TIMER_H_
#include <Arduino.h>

long timer_get_current_time();

long timer_get_run_time();
long timer_get_stop_time();

void timer_set_run_time(long time);
void timer_set_stop_time(long time);

char *timer_get_display_time(long time);

void timer_stop();
void timer_start();

void timer_set_state(bool running);
bool timer_is_running();
bool timer_is_paused();

void _timer_setup(void (*f)(bool));
void _timer_loop(long milliseconds);

#endif //TIMER_H_