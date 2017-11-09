#ifndef MOTOR_H_
#define MOTOR_H_

struct motorParameters {
	int maxSpeed;
	int acceleration;
	double decelerationProportion;
	int csThreshold;
	long changeDirTime;
};

struct motorStatus {
	int speed;
	int direction;
	int cs1;
	int cs2;
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

#endif // MOTOR_H_