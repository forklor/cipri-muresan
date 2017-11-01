#ifndef MOTOR_H_
#define MOTOR_H_

struct motorParameters {
	int maxSpeed;
	int acceleration;
	double decelerationProportion;
	int csThreshold;
};

void _motor_setup();
void _motor_loop(long milliseconds);

void motor_start();
void motor_stop();
void motor_stop_brake();
void motor_toggle_start_stop();
void motor_switch_direction();
void motor_set_parameters(motorParameters params);

#endif // MOTOR_H_