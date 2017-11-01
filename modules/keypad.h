#ifndef KEYPAD_H_
#define KEYPAD_H_

void _keypad_setup(void (*f)(char));
void _keypad_loop(long milliseconds);

#endif //KEYPAD_H_