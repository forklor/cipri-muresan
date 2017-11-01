#include <Keypad.h>

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns

char keys[ROWS][COLS] = {
	{'1','2','3', 'A'},
	{'4','5','6', 'B'},
	{'7','8','9', 'C'},
	{'*','0','#', 'D'}
};

byte rowPins[ROWS] = {2, 3, 4, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 7, 8, 9}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void (*_k_listener)(char pressed);

void _keypad_setup(void (*f)(char)) {
	_k_listener = f;
}

void _keypad_loop(long milliseconds) {
	char key = keypad.getKey();

	if (key != NO_KEY) {
		_k_listener(key);
	}
}
