#include "Arduino.h"

#include <math.h>
#include "motor.h"

#define BRAKE 0
#define CW    1
#define CCW   2
#define CS_THRESHOLD 15	 // Definition of safety current (Check: "1.3 Monster Shield Example").

#define MOTOR_A1_PIN 7
#define MOTOR_B1_PIN 8

#define PWM_MOTOR_1 5

#define CURRENT_SEN_1 A2

#define EN_PIN_1 A0

#define MOTOR_1 0

#define SPEED_CHANGE_STEP_MS 50 // how often to update motor speed

motorParameters settings;

int _currentSpeed;
int _currentDirection;
int _targetSpeed;
int _targetDirection;

long _lastLoopMillis;

//Function that controls the variables: motor(0 or 1), direction (cw or ccw) and pwm (between 0 and 255);
void motorGo(uint8_t motor, uint8_t direct, uint8_t pwm) {
	if(motor == MOTOR_1) {
		if(direct == CW) {
			digitalWrite(MOTOR_A1_PIN, LOW);
			digitalWrite(MOTOR_B1_PIN, HIGH);
		} else if(direct == CCW) {
			digitalWrite(MOTOR_A1_PIN, HIGH);
			digitalWrite(MOTOR_B1_PIN, LOW);
		} else {
			digitalWrite(MOTOR_A1_PIN, LOW);
			digitalWrite(MOTOR_B1_PIN, LOW);
		}

		analogWrite(PWM_MOTOR_1, pwm);
	}
}

void _motor_setup() {
	
	pinMode(MOTOR_A1_PIN, OUTPUT);
	pinMode(MOTOR_B1_PIN, OUTPUT);
	pinMode(PWM_MOTOR_1, OUTPUT);
	pinMode(CURRENT_SEN_1, INPUT);
	pinMode(EN_PIN_1, OUTPUT);

	// Initiates the serial to do the monitoring
	Serial.println("Begin motor control");

	_lastLoopMillis = millis();

	_currentSpeed = _targetSpeed = 0;
	_currentDirection = _targetDirection = CW;
}

void motor_start() {
	_targetSpeed = settings.maxSpeed;
}

void motor_stop() {
	_targetSpeed = 0;
}

void motor_toggle_start_stop() {
	_targetSpeed = _targetSpeed > 0 ? 0 : settings.maxSpeed;
}

void motor_switch_direction() {
	_targetDirection = _currentDirection == CW ? CCW : CW;
}

void motor_set_parameters(motorParameters params) {
	settings = params;
	_targetSpeed = settings.maxSpeed;
}

void _motor_loop(long milliseconds) {

	if(milliseconds - _lastLoopMillis < SPEED_CHANGE_STEP_MS) return;

	_lastLoopMillis = milliseconds;
	bool changed = false;
	if(_currentDirection == _targetDirection) {
		if(_currentSpeed != _targetSpeed) {
			int diff = _targetSpeed > _currentSpeed ? settings.acceleration : -(settings.acceleration * settings.decelerationProportion);
			_currentSpeed += diff;
			changed = true;
		}
	} else {
		if(_currentSpeed == 0) {
			_currentDirection = _targetDirection;
			changed = true;
		} else {
			_currentSpeed -= (settings.acceleration * settings.decelerationProportion);
			changed = true;
		}
	}

	if(changed) {
		Serial.print(_currentDirection);
		Serial.print("->");
		Serial.print(_currentSpeed);
		Serial.print("\n");
		motorGo(MOTOR_1, _currentSpeed == 0 ? BRAKE : _currentDirection, _currentSpeed);
	}

	if (analogRead(CURRENT_SEN_1) < CS_THRESHOLD) {
		//Serial.println("CS_THRESHOLD reached");
	}
}