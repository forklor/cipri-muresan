#ifndef WIRELES_H_
#define WIRELES_H_

#include "wireless_message.h"

void _wireless_setup(int pinA, int pinB);
void _wireless_loop(long milliseconds);

void wireless_listen(void (*f)(wirelessMessage));

#endif //WIRELES_H_