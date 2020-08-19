#ifndef MOTOR_H_
#define MOTOR_H_

struct motorParameters {
	int maxSpeed;
	int acceleration;
	double decelerationPercentage;
	int csThreshold;
	long changeDirTime;
};

struct motorStatus {
	int speed;
	byte direction;
	byte cs1;
	byte cs2;
	int battery;
	bool disabled;
};

void _motor_setup();
void _motor_loop(long milliseconds);

void motor_start();
void motor_stop();
void motor_stop_brake();
void motor_toggle_start_stop();
void motor_switch_direction();
void motor_set_parameters(motorParameters params);
motorParameters motor_get_parameters();
motorStatus motor_get_status();

bool motor_is_disabled();

#endif // MOTOR_H_