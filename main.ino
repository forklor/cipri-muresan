#include "modules/motor.h"

void setup() {

	Serial.begin(9600);
	_motor_setup();
	motor_set_parameters({
		100,
		5,
		2,
		15
	});
	motor_start();
}

void loop() {
	
	unsigned long milliseconds = millis();

	_motor_loop(milliseconds);

	if (Serial.available() > 0) {
		char receivedChar = Serial.read();
		switch(receivedChar) {
			case 's':
				motor_toggle_start_stop();
				break;
			case 'x':
				motor_stop();
				break;
			case 'b':
				motor_start();
				break;
			case 'd':
				motor_switch_direction();
				break;
			default:
				Serial.print("Unknown command ");
				Serial.print(receivedChar);
				Serial.print("\n");
		}
	}
}