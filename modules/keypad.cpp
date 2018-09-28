#include "keypad.h"

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns

char keys[ROWS][COLS] = {
	/*{'1','2','3', 'A'},
	{'4','5','6', 'B'},
	{'7','8','9', 'C'},
	{'*','0','#', 'D'} */

	{'D', '#', '0', '*'},
	{'C', '9', '8', '7'},
	{'B', '6', '5', '4'},
	{'A', '3', '2', '1'},
};

	


//byte rowPins[ROWS] = {2, 3, 4, 5}; //connect to the row pinouts of the keypad
//byte colPins[COLS] = {6, 7, 8, 9}; //connect to the column pinouts of the keypad

byte rowPins[ROWS] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 4, 3, 2}; //connect to the column pinouts of the keypad



Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void (*_k_listener)(Key *keys, int keysLen);

bool holdKeys[16];

void keypad_add_keyhold(Key key) {
	holdKeys[key.kcode] = true;
}

bool keypad_is_keyhold(Key key) {
	return holdKeys[key.kcode];
}

void keypad_remove_keyhold(Key key) {
	holdKeys[key.kcode] = false;
}

void _keypad_setup(void (*f)(Key *, int)){
	keypad.setHoldTime(5000);
	_k_listener = f;
	for(int i = 0; i< 16; i++) {
		holdKeys[i] = false;
	}
}

void _keypad_loop(long milliseconds) {

	int changedKeysLen = 0;
	if (keypad.getKeys()) {
		bool hasChanges = false;
		Key changedKeys[10] = {};
		for (int i=0; i<LIST_MAX; i++) {
			if (keypad.key[i].kstate != IDLE) {
				changedKeys[changedKeysLen] = keypad.key[i];
				changedKeysLen += 1;
				
				if(!hasChanges && keypad.key[i].stateChanged) hasChanges = true;
			}
		}
		if(hasChanges && changedKeysLen > 0) {
			_k_listener(changedKeys, changedKeysLen);
		}
	}
}
