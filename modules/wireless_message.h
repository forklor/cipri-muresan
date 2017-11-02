#ifndef WIRELESS_MESSAGE_H_
#define WIRELESS_MESSAGE_H_

#include "motor.h"

#define MESSAGE_SET_PARAMS 1
#define MESSAGE_TOGGLE_START_STOP 2
#define MESSAGE_CHANGE_DIRECTION 3
#define MESSAGE_START 4
#define MESSAGE_STOP 5
#define MESSAGE_MOTOR_STATUS 6

struct wirelessMessage {
	int type;
	motorParameters parameters;
};

#endif // WIRELESS_MESSAGE_H_