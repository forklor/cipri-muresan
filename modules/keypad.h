#ifndef KEYPAD_H_
#define KEYPAD_H_

#include <Keypad.h>

void _keypad_setup(void (*f)(Key *, int));
void _keypad_loop(long milliseconds);

void keypad_add_keyhold(Key key);
bool keypad_is_keyhold(Key key);
void keypad_remove_keyhold(Key key);

#endif //KEYPAD_H_