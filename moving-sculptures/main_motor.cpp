#include "main_motor.h"

#ifdef MOTOR_CONTROLLER

#define MEMORY_CHECK_VALUE 37

#include <Arduino.h>
#include <EEPROM.h>

#include "modules/motor.h"
#include "modules/wireless.h"
#include "modules/MemoryFree.h"

void saveMotorParameters(motorParameters params) {
	int check_memory = MEMORY_CHECK_VALUE;
	EEPROM.put(0, check_memory);
	EEPROM.put(sizeof(int), params);
}

wirelessMessageAck m_wirelessMessageReceived(wirelessMessageCommand message) {
	Serial.print(F("received message "));
	Serial.println(message.type);

	wirelessMessageAck ack;
	ack.motorModuleNumber = MOTOR_MODULE_NUMBER;

	switch (message.type) {
		case MESSAGE_TOGGLE_START_STOP:
			if(!motor_is_disabled()) motor_toggle_start_stop();
			break;
		case MESSAGE_GET_PARAMS:
			message.parameters = motor_get_parameters();
			break;
		case MESSAGE_CHANGE_DIRECTION: 
			if(!motor_is_disabled()) motor_switch_direction();
			break;
		case MESSAGE_START:
			if(!motor_is_disabled()) motor_start();
			break;
		case MESSAGE_STOP:
			if(!motor_is_disabled()) motor_stop();
			break;
		default:
			Serial.println(F("Unknown message type"));
	}

	return ack;
}


wirelessMessageResponse m_wirelessMessageWithResponseReceived(wirelessMessageCommand message) {
	Serial.print(F("received message "));
	Serial.println(message.type);

	wirelessMessageResponse resp;
	resp.motorModuleNumber = MOTOR_MODULE_NUMBER;
	resp.type = message.type;

	switch (message.type) {
		case MESSAGE_GET_PARAMS:
			resp.parameters = motor_get_parameters();
		case MESSAGE_MOTOR_STATUS:
			resp.status = motor_get_status();
			break;
		default:
			Serial.println(F("Unknown message type"));
	}

	return resp;
}


void _setup() {

	Serial.begin(9600);
	Serial.println(F("Motor program running"));
	_wireless_setup(9, 10, MOTOR_MODULE_NUMBER);
	wireless_listen(MOTOR_MODULE_NUMBER, m_wirelessMessageReceived, m_wirelessMessageWithResponseReceived);

	int check_memory;
	motorParameters savedParameters;
	EEPROM.get(0, check_memory);
	if(check_memory == MEMORY_CHECK_VALUE) {
		Serial.println(F("Found parameters saved in EEPROM"));
		EEPROM.get(sizeof(int), savedParameters);
	} else {
		Serial.println(F("Didn't find parameters saved in EEPROM, initializing..."));
		saveMotorParameters({
			100, 			// Speed
			20, 			// Accelereration step
			50, 			// Decceleration percentage
			500, 			// CS Threshold
			20000  			// Change dir time
		});
	}
	_motor_setup();
	motor_set_parameters(savedParameters);
}

long displayFreeMemoryMs;

void _loop() {
	
	unsigned long milliseconds = millis();

	_motor_loop(milliseconds);
	_wireless_loop(milliseconds);

	// if(milliseconds - displayFreeMemoryMs >= 1000) {
	// 	displayFreeMemoryMs = milliseconds;
	// 	Serial.print("battery()=");
	// 	Serial.println(analogRead(4));
	// }

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