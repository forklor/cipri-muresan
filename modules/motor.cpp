#include "Arduino.h"

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

#include <math.h>
#include "motor.h"

#define BRAKE 0
#define CW    1
#define CCW   2
#define CS_THRESHOLD 15	 // Definition of safety current (Check: "1.3 Monster Shield Example").

#define MOTOR_A1_PIN 7
#define MOTOR_B1_PIN 8

#define PWM_MOTOR_1 6

#define CURRENT_SEN_1 A2
#define CURRENT_SEN_2 A3

#define EN_PIN_1 A0
#define EN_PIN_2 A1

#define MOTOR_1 0

#define INTERRUPT_PIN 2

#define SPEED_CHANGE_STEP_MS 50 // how often to update motor speed
#define DIRECTION_CHANGE_STOP_TIME_MS 1400 // how much to wait after speed gets to 0 before changing direction

motorParameters settings;

int _currentSpeed;
volatile int _currentDirection;
int _targetSpeed;
volatile int _targetDirection;

long _lastLoopMillis;
long _directionChangeStopMillis;
long _directionChangeMillis;

bool _running;

//Function that controls the variables: motor(0 or 1), direction (cw or ccw) and pwm (between 0 and 255);
void motorGo( uint8_t direct, uint8_t pwm) {

	if(direct == CW) {
		digitalWrite(MOTOR_A1_PIN, LOW);
		digitalWrite(MOTOR_B1_PIN, HIGH);
		//digitalWrite(MOTOR_A2_PIN, LOW);
		//digitalWrite(MOTOR_B2_PIN, HIGH);
	} else if(direct == CCW) {
		digitalWrite(MOTOR_A1_PIN, HIGH);
		digitalWrite(MOTOR_B1_PIN, LOW);
		//digitalWrite(MOTOR_A2_PIN, HIGH);
		//digitalWrite(MOTOR_B2_PIN, LOW);
	} else {
		digitalWrite(MOTOR_A1_PIN, LOW);
		digitalWrite(MOTOR_B1_PIN, LOW);
		//digitalWrite(MOTOR_A2_PIN, LOW);
		//digitalWrite(MOTOR_B2_PIN, LOW);
	}

	analogWrite(PWM_MOTOR_1, pwm);
	//analogWrite(PWM_MOTOR_2, pwm);
}

void interrupt_listener() {
	_targetDirection = _currentDirection == CW ? CCW : CW;
}

void _motor_setup() {

	pinMode(MOTOR_A1_PIN, OUTPUT);
	pinMode(MOTOR_B1_PIN, OUTPUT);

	pinMode(PWM_MOTOR_1, OUTPUT);

	pinMode(INTERRUPT_PIN, INPUT_PULLUP);

	//pinMode(MOTOR_A2_PIN, OUTPUT);
	//pinMode(MOTOR_B2_PIN, OUTPUT);

	//pinMode(PWM_MOTOR_2, OUTPUT);

	pinMode(CURRENT_SEN_1, INPUT);
	pinMode(CURRENT_SEN_2, INPUT);
	pinMode(EN_PIN_1, OUTPUT);
	pinMode(EN_PIN_2, OUTPUT);

	digitalWrite(EN_PIN_1, HIGH);
	digitalWrite(EN_PIN_2, HIGH);

	_lastLoopMillis = millis();

	_running = false;
	_currentSpeed = _targetSpeed = 0;
	_currentDirection = _targetDirection = CW;

	attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), interrupt_listener, CHANGE);
}

void motor_start() {
	_running = true;
	_targetSpeed = settings.maxSpeed;
	if(_targetDirection != _currentDirection) {
		_targetDirection = _currentDirection;
	}
	if(_targetDirection == BRAKE) {
		_targetDirection = _currentDirection = CW;
	}
	_directionChangeMillis = millis();
}

void motor_stop() {
	_running = false;
	_targetSpeed = 0;
	_targetDirection = _currentDirection;// = BRAKE;
}

void motor_stop_brake() {
	_targetSpeed = _currentSpeed = 0;
	//_targetDirection = _currentDirection = BRAKE;
	motorGo(_currentDirection, 0);
}

void motor_toggle_start_stop() {
	_targetSpeed = _targetSpeed > 0 ? 0 : settings.maxSpeed;
	_targetDirection = _currentDirection;
	if(_targetSpeed > 0) _directionChangeMillis = millis();
	if(_targetSpeed == 0) {
		_targetDirection = _currentDirection = BRAKE;
	} else if(_currentDirection == BRAKE) {
		_targetDirection = _currentDirection = CW;
	}
}

void motor_switch_direction() {
	_targetDirection = _currentDirection == CW ? CCW : CW;
}

void motor_set_parameters(motorParameters params) {
	settings = params;
	if(_running) _targetSpeed = settings.maxSpeed;
}

motorParameters motor_get_parameters() {
	return settings;
}

motorStatus motor_get_status() {
	motorStatus status = {
		_currentSpeed,
		_currentDirection,
		analogRead(CURRENT_SEN_1),
		analogRead(CURRENT_SEN_2)
	};

	return status;
}

void _motor_loop(long milliseconds) {

	if(milliseconds - _lastLoopMillis < SPEED_CHANGE_STEP_MS) return;

	_lastLoopMillis = milliseconds;
	bool changed = false;
	if(_currentDirection == _targetDirection) {
		if(_currentSpeed != _targetSpeed) {
			int diff = _targetSpeed > _currentSpeed ? settings.acceleration : -(settings.acceleration * (1 / (settings.decelerationPercentage / 100)));
			_currentSpeed += diff;
			changed = true;
		} else if(_targetSpeed > 0 && milliseconds - _directionChangeMillis >= settings.changeDirTime) {
			Serial.println(F("Changing direction, time passed"));
			motor_switch_direction();
		}
	} else {

		if(_currentSpeed == 0) {
			// When reaching speed 0, it takes a while for the motor to fully stop,
			// To prevent any sudden movement, wait some time before changing speed and direction
			if(!_directionChangeStopMillis) {
				_directionChangeStopMillis = milliseconds;
			}
			if(milliseconds - _directionChangeStopMillis >= DIRECTION_CHANGE_STOP_TIME_MS) {
				_directionChangeStopMillis = 0;
				_currentDirection = _targetDirection;
				changed = true;
				_directionChangeMillis = milliseconds;
			}
		} else {
			_currentDirection = BRAKE;
			_currentSpeed -= (settings.acceleration * settings.decelerationPercentage);
			changed = true;
		}
	}

	_currentSpeed = MIN(255, MAX(0, _currentSpeed));

	if(changed) {
		Serial.print(_currentDirection);
		Serial.print(F("->"));
		Serial.println(_currentSpeed);

		motorGo(_currentDirection, _currentSpeed);
	}

	// Serial.print("cs1:");
	// Serial.print(analogRead(CURRENT_SEN_1));
	// Serial.print(" cs2:");
	// Serial.print(analogRead(CURRENT_SEN_2));
	// Serial.print(" en1:");
	// Serial.print(analogRead(EN_PIN_1));
	// Serial.print(" en2:");
	// Serial.print(analogRead(EN_PIN_2));
	// Serial.print("\n");


	if (_currentSpeed == _targetSpeed && _currentSpeed > 0 && analogRead(CURRENT_SEN_1) < CS_THRESHOLD) {
		//Serial.println("CS_THRESHOLD reached");
	}
}