/**
 *  Script that uses Monster Moto Shield controller to change motor speed and direction following defined intervals (STATES_CYCLE)
 */
#include <math.h>

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

struct motorMotion {
	int drive;
	int speed;
	long millis;
};

struct motionState {
	int startSpeed;
	int endSpeed;
	int driveDirection;
	long timeMs;
};

// List of states for values
// {starSpeed, endSpeed, driveDirection, durationMs}
motionState STATES_CYCLE[8] = {
	{ 0, 255, CW, 5000 }, // increase speed CW from 0 to 255 in 5000 ms (5 seconds)
	{ 255, 255, CW, 5000 },	// stay at 255 speed CW for 5 seconds
	{ 255, 0, CW, 5000 }, //	decrease to speed CW 0 in 5 seconds
	{ 0, 0, BRAKE, 2000 },	// stay at speed 0 (motor brake) for 5 seconds
	{ 0, 255, CCW, 5000 }, // increase speed to 255 CCW (other direction) in 5 seconds
	{ 255, 255, CCW, 5000 }, // stay at top speed CCW for 5 seconds
	{ 255, 0, CCW, 1000 }, // decrease to speed 0 CCW in 1 second
	{ 0, 0, BRAKE, 10000 } // stay at speed 0 (motor brake) for 10 seconds
};

motorMotion currentMotion;
motionState currentState;

int stateCycleIndex = 0;
long startTimeCurrentState = 0;

int getIntervalValue(long currTime, long begin, long difference, long totalTime) {
	// return difference * currTime / totalTime + begin; // -> LINEAR
	return round(begin + difference * pow((double)currTime / (double)totalTime, 5)); // -> EXPONENTIAL OUT
}

void updateMotorMotion(long millis) {

	// Update speed
	if(millis - currentMotion.millis >= SPEED_CHANGE_STEP_MS) {
		currentMotion.speed = getIntervalValue(
			millis - startTimeCurrentState, 
			currentState.startSpeed, 
			currentState.endSpeed - currentState.startSpeed, 
			currentState.timeMs);
		currentMotion.millis = millis;
	}

	// Move on to the next state if the current one is over
	if(millis - startTimeCurrentState >= currentState.timeMs) {
		
		stateCycleIndex += 1;

		// start over if we got to the end cycle
		if(stateCycleIndex > (sizeof(STATES_CYCLE) / sizeof(motionState)) - 1) { 
			stateCycleIndex = 0; 
		}

		currentState = STATES_CYCLE[stateCycleIndex];
		startTimeCurrentState = millis;

		currentMotion.drive = currentState.driveDirection;
		currentMotion.speed = currentState.startSpeed;
		currentMotion.millis = millis;
	}
}

void setup() {
	
	pinMode(MOTOR_A1_PIN, OUTPUT);
	pinMode(MOTOR_B1_PIN, OUTPUT);
	pinMode(PWM_MOTOR_1, OUTPUT);
	pinMode(CURRENT_SEN_1, INPUT);
	pinMode(EN_PIN_1, OUTPUT);

	// Initiates the serial to do the monitoring
	Serial.begin(9600);
	Serial.println("Begin motor control");

	long milliseconds = millis();

	currentMotion = {
		CW,
		0,
		milliseconds
	};

	stateCycleIndex = 0;
	currentState = STATES_CYCLE[stateCycleIndex];
	startTimeCurrentState = milliseconds;
}

void loop() {

	unsigned long currMilis = millis();
	updateMotorMotion(currMilis);
	if(currMilis == currentMotion.millis) {
		Serial.print(stateCycleIndex);
		Serial.print("=");
		Serial.print(currentMotion.drive);
		Serial.print("->");
		Serial.print(currentMotion.speed);
		Serial.print("\n");
		motorGo(MOTOR_1, currentMotion.drive, currentMotion.speed);
	}

	if (analogRead(CURRENT_SEN_1) < CS_THRESHOLD) {
		//Serial.println("CS_THRESHOLD reached");
	}
}

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