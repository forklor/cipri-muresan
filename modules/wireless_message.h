#ifndef WIRELESS_MESSAGE_H_
#define WIRELESS_MESSAGE_H_

#include "motor.h"

#define MESSAGE_SET_PARAMS 1
#define MESSAGE_GET_PARAMS 2
#define MESSAGE_TOGGLE_START_STOP 3
#define MESSAGE_CHANGE_DIRECTION 4
#define MESSAGE_START 5
#define MESSAGE_STOP 6
#define MESSAGE_MOTOR_STATUS 7

struct wirelessMessage {
	int type;
	int motorModuleNumber;
	motorParameters parameters;
	motorStatus status;
};

#endif // WIRELESS_MESSAGE_H_