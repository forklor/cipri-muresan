#ifndef KEYPAD_H_
#define KEYPAD_H_

#include <Keypad.h>

void _keypad_setup(void (*f)(Key *, int));
void _keypad_loop(long milliseconds);

#endif //KEYPAD_H_