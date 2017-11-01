#ifndef MAIN_ENTRY_FILE
#define MAIN_ENTRY_FILE

#include "modules/motor.h"
#include "modules/wireless.h"
#include "modules/wireless_message.h"

void wirelessMessageReceived(wirelessMessage message) {
	Serial.print("received message");
	Serial.print(message.type);
	Serial.print("\n");
}

void setup() {

	Serial.begin(9600);
	_motor_setup();
	motor_set_parameters({
		100,
		5,
		2,
		15
	});
	
	_wireless_setup(9, 10);
	wireless_listen(wirelessMessageReceived);
}

void loop() {
	
	unsigned long milliseconds = millis();

	_motor_loop(milliseconds);
	_wireless_loop(milliseconds);

	if (Serial.available() > 0) {
		char receivedChar = Serial.read();
		switch(receivedChar) {
			case 's':
				motor_toggle_start_stop();
				break;
			case 'x':
				motor_stop();
				break;
			case 'c':
				motor_stop_brake();
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

#endif //MAIN_ENTRY_FILE