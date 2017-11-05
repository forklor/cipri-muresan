#include "config.h"

#ifdef MOTOR_CONTROLLER
#include "main_motor.h"
#endif //MOTOR_CONTROLLER

#ifdef REMOTE_CONTROL_CONTROLLER
#include "main_remote_control.h"
#endif //REMOTE_CONTROL_CONTROLLER

void setup() {
	_setup();
}

void loop() {
	_loop();
}