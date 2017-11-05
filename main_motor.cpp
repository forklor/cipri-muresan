#include "main_motor.h"

#ifdef MOTOR_CONTROLLER

#include <Arduino.h>

#include "modules/motor.h"
#include "modules/wireless.h"

void m_wirelessMessageReceived(wirelessMessage message) {
	Serial.print("received message ");
	Serial.print(message.type);
	Serial.print("\n");

	switch (message.type) {
		case MESSAGE_TOGGLE_START_STOP:
			motor_toggle_start_stop();
			break;
		case MESSAGE_SET_PARAMS:
			motor_set_parameters(message.parameters);
			break;
		case MESSAGE_CHANGE_DIRECTION: 
			motor_switch_direction();
			break;
		case MESSAGE_START:
			motor_start();
			break;
		case MESSAGE_STOP:
			motor_start();
			break;
		default:
			Serial.println("Unknown message type");
	}
}

void _setup() {

	Serial.begin(9600);
	Serial.println("Motor program running");
	_wireless_setup(9, 10, MOTOR_MODULE_NUMBER);
	wireless_listen(WIRELESS_REMOTE, m_wirelessMessageReceived);

	_motor_setup();
	motor_set_parameters({
		100,
		5,
		2,
		15
	});
}

void _loop() {
	
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

#endif //MOTOR_CONTROLLER