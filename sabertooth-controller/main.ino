
#define CONTROLLER_A1 3
#define CONTROLLER_A2 4

#define CONTROLLER_S1 5

#define SWITCH_1 7
#define SWITCH_2 8

#define BUTTON_START 9
#define BUTTON_STOP 10
#define BUTTON_PAUSE 12
#define BUTTON_CHANGE_DIRECTION 11

#define KNOB A0

#define INPUT_UPDATE_STEP_MS 200
#define SPEED_UPDATE_STEP_MS 400
#define SPEED_CHANGE_STEP 5

#define DEFAULT_STEP_VALUE -1000

#define ACCELERATE_TIME_MS 10000
#define DECELERATE_TIME_MS 5000

#define BUTTON_READ_TIME_MS 500

enum State {
  STOPPED,
  ACCELERATING,
  DECELERATING,
  RUNNING
};

State state;
State nextState;

int _stopped_from_button = 0;
int _paused_after_stop = 0;

int _curr_speed;

/**
 * Switches motor direction.
 * This is done by changing values for A1 and A2
 * connection on the controole
 */
int _direction = 0;
void toggle_motor_direction() {

	Serial.print(F("change direction"));
	Serial.println(_direction);
	analogWrite(CONTROLLER_A1, _direction ? 0 : 255);
	analogWrite(CONTROLLER_A2, _direction ? 0 : 255);
	_direction = _direction ? 0 : 1;
}

void set_motor_direction(int dir) {
	_direction = dir;
	analogWrite(CONTROLLER_A1, dir ? 255 : 0);
	analogWrite(CONTROLLER_A2, dir ? 255 : 0);
}

/**
 * Reads value from cappacitator and updates max speed
 */
int _max_speed = 0;
int _knob_value = -1000;
void read_knob_value() {
	int new_knob_val = analogRead(KNOB);

	if(
		abs(new_knob_val - _knob_value) > 50 || 
		(new_knob_val == 0 && _knob_value != 0) ||
		(new_knob_val == 1023 && _knob_value != 1023)
		) {

		_knob_value = new_knob_val;
		_max_speed = (int)((float)_knob_value / 1023 * 255);
		Serial.print(F("Max speed changed: "));
		Serial.println(_max_speed);
	}
}

int sign(int val) {
	return abs(val) / val;
}


int _button_pause = -1;
long _read_button_pause_ms = 0;
void read_pause_button() {

	int changed = 0;
	int val = digitalRead(BUTTON_PAUSE);
	
	if(val != _button_pause) {
		_button_pause = val;
		changed = 1;
		Serial.print(F("Pause button changed "));
		Serial.println(val);
	}


	long milliseconds = millis();
	if(changed && milliseconds -_read_button_pause_ms >= BUTTON_READ_TIME_MS) {
		_read_button_pause_ms = milliseconds;
		
		if(val == HIGH) {
			if(_stopped_from_button) {
				_paused_after_stop = 1;
			}
			Serial.println(F("Pause button high, pausing..."));
			stop();
		} else if(
			val == LOW &&
				(
					!_stopped_from_button ||
					(_stopped_from_button && _paused_after_stop)
				)
		) {
			Serial.println(F("Pause button low, resuming..."));
			start();
		}
	}
}


int _button_start = 0;
long _read_button_start_ms = 0;
void read_start_button() {

	int val = digitalRead(BUTTON_START);
	if(val != _button_start) {
		_button_start = val;
		Serial.print(F("Start button changed "));
		Serial.println(val);
	}

	long milliseconds = millis();
	if(digitalRead(BUTTON_START) == HIGH && milliseconds -_read_button_start_ms >= BUTTON_READ_TIME_MS) {
		_read_button_start_ms = milliseconds;
		Serial.println(F("Start button pressed"));
		_stopped_from_button = 0;
		_paused_after_stop = 0;
		start();
	}
}


int _button_stop = 0;
long _read_button_stop_ms = 0;
void read_stop_button() {

	int val = digitalRead(BUTTON_STOP);
	if(val != _button_stop) {
		_button_stop = val;
		Serial.print(F("Stop button changed "));
		Serial.println(val);
	}

	long milliseconds = millis();
	if(digitalRead(BUTTON_STOP) == HIGH && milliseconds -_read_button_stop_ms >= BUTTON_READ_TIME_MS) {
		_read_button_stop_ms = milliseconds;
		Serial.println(F("Stop button pressed"));
		_stopped_from_button = 1;
		_paused_after_stop = 0;
		stop();
	}
}


int _button_toggle_direction = 0;
long _read_button_toggle_ms = 0;
void read_toggle_direction_button() {

	int val = digitalRead(BUTTON_CHANGE_DIRECTION);
	if(val != _button_toggle_direction) {
		_button_toggle_direction = val;
		Serial.print(F("Toggle direction button changed "));
		Serial.println(val);
	}

	long milliseconds = millis();
	if(digitalRead(BUTTON_CHANGE_DIRECTION) == HIGH && milliseconds - _read_button_toggle_ms  >= BUTTON_READ_TIME_MS) {
		_read_button_toggle_ms = milliseconds;
		Serial.println(F("Toggle dirrect button pressed"));
		switch_direction();
	}
}

int _switch_1_value = 0;
void read_direction_switch_1() {
	
	int val = digitalRead(SWITCH_1);
	if(val != _switch_1_value) {
		_switch_1_value = val;
		Serial.print(F("Switch 1 changed "));
		Serial.println(val);
	}
	if(digitalRead(SWITCH_1) == LOW) {
		Serial.println(F("Switch 1 trigger"));
		switch_direction(0);
	}
}

int _switch_2_value = 0;
void read_direction_switch_2() {

	int val = digitalRead(SWITCH_2);
	if(val != _switch_2_value) {
		_switch_2_value = val;
		Serial.print(F("Switch 2 changed "));
		Serial.println(val);
	}
	if(digitalRead(SWITCH_2) == LOW) {
		Serial.println(F("Switch 2 trigger"));
		switch_direction(1);
	}
}

void stop() {
	Serial.println(F("Stopping..."));
	switch(state) {
		case RUNNING:
		case DECELERATING:
		case ACCELERATING:
			reset_speed_change_step();
			nextState = STOPPED;
			state = DECELERATING;
			break;
		case STOPPED:
			Serial.println(F("Already STOPPED, staying this way"));
			break;

	}
}

void start() {
	Serial.println(F("Starting..."));
	switch(state) {
		case STOPPED:
			nextState = RUNNING;
			state = ACCELERATING;
			break;
		case DECELERATING:
			if(nextState == RUNNING) {
				Serial.println(F("Already running, staying this way"));	
			} else {
				reset_speed_change_step();
				nextState = RUNNING;
				state = ACCELERATING;
			}
			break;
		case ACCELERATING:
		case RUNNING:
			Serial.println(F("Already STARTED, staying this way"));
			break;

	}
}

void toggle_start_stop() {

	if((state == RUNNING && nextState == RUNNING) ||
		(state == ACCELERATING && nextState == RUNNING) ||
		(state == DECELERATING && nextState == RUNNING)) {
		stop();
	} else {
		start();
	}
}

void switch_direction() {
	
	nextState = RUNNING;

	if(state == RUNNING) {

		state = DECELERATING;
		reset_speed_change_step();

	} else if(state == ACCELERATING) {

		reset_speed_change_step();
		state = DECELERATING;

	} else if(state == STOPPED) {

		toggle_motor_direction();
	}

}


void switch_direction(int dir) {

	if(dir != _direction) {
		switch_direction();
	}
}

void read_serial_input() {

	if (Serial.available() > 0) {
		char receivedChar = Serial.read();
		switch(receivedChar) {
			case 's':
				start();
				break;
			case 'd':
				stop();
				break;
			case 'c':
				switch_direction(1);
				break;
			case 'x':
				switch_direction(0);
				break;
			case 'v':
				switch_direction();
				break;
		}
	}
}


void setup() {

	Serial.begin(9600);
	Serial.println(F("Program running..."));

	state = STOPPED;
	_curr_speed = 0;

	set_motor_direction(0);
	read_knob_value();

	pinMode(SWITCH_1, INPUT);
	pinMode(SWITCH_2, INPUT);

	pinMode(BUTTON_START, INPUT);
	pinMode(BUTTON_STOP, INPUT);
	pinMode(BUTTON_PAUSE, INPUT_PULLUP);
	pinMode(BUTTON_CHANGE_DIRECTION, INPUT);

	_switch_1_value = digitalRead(SWITCH_1);
	_switch_2_value = digitalRead(SWITCH_2);

	_button_toggle_direction = digitalRead(BUTTON_CHANGE_DIRECTION);

	Serial.print(F("Switch 1 value: "));
	Serial.println(_switch_1_value);

	Serial.print(F("Switch 2 value: "));
	Serial.println(_switch_2_value);


	Serial.print(F("Toggle direction button state: "));
	Serial.println(_button_toggle_direction);

}


int _step = DEFAULT_STEP_VALUE;
void compute_speed_change_step(int target_speed, long time) {
	if(_step == DEFAULT_STEP_VALUE) {
		_step = ((float)(target_speed - _curr_speed)) / (time / SPEED_UPDATE_STEP_MS);
		if(_step == 0) {
			_step = target_speed > _curr_speed ? 1 : -1;
		}
		Serial.print(F("Speed change step "));
		Serial.println(_step);
	}
}

void reset_speed_change_step() {
	_step = DEFAULT_STEP_VALUE;
}


long _last_update_input_millis = 0;
long _last_update_speed_millis = 0;
void loop() {

	long milliseconds = millis();
	bool isInputUpdateStep = milliseconds - _last_update_input_millis >= INPUT_UPDATE_STEP_MS;
	bool isSpeedUpdateStep = milliseconds - _last_update_speed_millis >= SPEED_UPDATE_STEP_MS;
	
	if(isInputUpdateStep) {
		_last_update_input_millis = milliseconds;

		read_start_button();
		read_stop_button();
		read_pause_button();
		read_toggle_direction_button();
		read_direction_switch_1();
		read_direction_switch_2();
		read_serial_input();
		read_knob_value();
	}

	if(isSpeedUpdateStep) {
		_last_update_speed_millis = milliseconds;
		switch (state) {
		
			case RUNNING:
				if(_curr_speed != _max_speed) {
					//Serial.println(sign(_max_speed - _curr_speed));
					//Serial.println(sign(_max_speed - _curr_speed) * SPEED_CHANGE_STEP);
					_curr_speed = max(0, min(_max_speed, _curr_speed + sign(_max_speed - _curr_speed) * SPEED_CHANGE_STEP));
				}
			break;

			case ACCELERATING:
				
				if(_curr_speed != _max_speed) {
					compute_speed_change_step(_max_speed, ACCELERATE_TIME_MS);
					_curr_speed = max(0, min(_max_speed, _curr_speed + _step));
				} else {
					reset_speed_change_step();
					state = RUNNING;
				}
			break;

			case DECELERATING:
				if(_curr_speed > 0) {
					compute_speed_change_step(0, DECELERATE_TIME_MS);
					_curr_speed = max(0, _curr_speed + _step);
				} else if(_curr_speed == 0) {
					reset_speed_change_step();
					if(nextState == RUNNING) {
						toggle_motor_direction();
						state = ACCELERATING;
					} else if(nextState == STOPPED) {
						state = STOPPED;
					}
				}
			break;

			case STOPPED:
			// do nothing
			break;
		};

		// if(_curr_speed) {
		// 	Serial.print(_direction ? "->" : "<-");
		// 	Serial.println(_curr_speed);
		// }
		analogWrite(CONTROLLER_S1, _curr_speed);
	}
}