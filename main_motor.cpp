#include "main_motor.h"

#ifdef MOTOR_CONTROLLER

#include <Arduino.h>

#include "modules/motor.h"
#include "modules/wireless.h"

wirelessMessage m_wirelessMessageReceived(wirelessMessage message) {
	Serial.print(F("received message "));
	Serial.println(message.type);

	switch (message.type) {
		case MESSAGE_TOGGLE_START_STOP:
			motor_toggle_start_stop();
			break;
		case MESSAGE_SET_PARAMS:
			Serial.print(F("speed:"));
			Serial.print(message.parameters.maxSpeed);
			Serial.print(F("acc:"));
			Serial.print(message.parameters.acceleration);
			Serial.print(F("decc:"));	
			Serial.println(message.parameters.decelerationPercentage);
			motor_set_parameters(message.parameters);
			break;
		case MESSAGE_GET_PARAMS:
			message.parameters = motor_get_parameters();
			break;
		case MESSAGE_CHANGE_DIRECTION: 
			motor_switch_direction();
			break;
		case MESSAGE_START:
			motor_start();
			break;
		case MESSAGE_STOP:
			motor_stop();
			break;
		case MESSAGE_MOTOR_STATUS:
			message.status = motor_get_status();
			break;
		default:
			Serial.println(F("Unknown message type"));
	}

	return message;
}

void _setup() {

	Serial.begin(9600);
	Serial.println(F("Motor program running"));
	_wireless_setup(9, 10, MOTOR_MODULE_NUMBER);
	wireless_listen(WIRELESS_REMOTE, m_wirelessMessageReceived);

	_motor_setup();
	motor_set_parameters({
		100, 			// Speed
		20, 			// Accelereration step
		50, 			// Decceleration percentage
		15, 			// CS Threshold
		20000  			// Change dir time
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
				Serial.print(F("Unknown command "));
				Serial.println(receivedChar);
		}
	}
}

#endif //MOTOR_CONTROLLER