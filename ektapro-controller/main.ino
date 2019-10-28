#include <SoftwareSerial.h>
#include <Chrono.h> 

#include "projectors.h"
#include "times.h"

#define BUTTON_PIN 12
#define BUTTON_DEBOUNCE_MS 5000

#define STATE_STOPPED 0
#define STATE_CHANGING 1
#define STATE_VIEWING_1 2
#define STATE_VIEWING_2 3
#define STATE_VIEWING_3 4
#define STATE_VIEWING_ALL 5
#define STATE_CLOSING_ALL 6
#define STATE_INIT 7
#define STATE_PRE_INIT 9
#define STATE_CLOSING_ALL_END 10
#define STATE_STOPPED_LOOP 11

#define PRE_INIT_TIME 500
#define INIT_TIME 3000
#define CLOSE_TIME_END 3000

#define IGNORE_SENSOR_ON_STOP_TIME_PASSED_MS 60000

//#define DEBUG

int current_state;
int current_step;
int max_steps;
int current_projector;
int view_all_time;

int closed_all_end;

Chrono chrono;
Chrono btn_chrono;
Chrono ignore_sensor_chrono;

int prev_btn_state;

void print_state_name(int state) {
	char *name;
	switch(state) {
		case STATE_STOPPED:
			name = (char *)"stopped";
		break;

		case STATE_CHANGING:
			name = (char *)"changing";
		break;

		case STATE_VIEWING_1:
			name = (char *)"viewing 1";
		break;

		case STATE_VIEWING_2:
			name = (char *)"viewing 2";
		break;

		case STATE_VIEWING_3:
			name = (char *)"viewing 3";
		break;

		case STATE_VIEWING_ALL:
			name = (char *)"viewing all";
		break;

		case STATE_CLOSING_ALL:
			name = (char *)"closing all";
		break;

		case STATE_CLOSING_ALL_END:
			name = (char *)"closing all end";
		break;

		case STATE_INIT:
			name = (char *)"init";
		break;

		case STATE_PRE_INIT:
			name = (char *)"pre init";
		break;
		case STATE_STOPPED_LOOP:
			name = (char *)"stop loop";
		break;
	}

	Serial.println(name);
}

void exit_state(int state) {

	//Serial.print("exit state");
	//print_state_name()
}

void enter_state(int state) {

#ifdef DEBUG
	Serial.print("enter state ");
	print_state_name(state);
#endif

	int empty_slides;
	int i;

	switch(current_state) {
		
		case STATE_CHANGING:
			current_step += 1;

			if(current_step >= max_steps) {
				set_state(STATE_CLOSING_ALL_END);
			} else {

			#ifdef DEBUG
				Serial.print("going to slide ");
				Serial.println(TIMES[current_step][0]);
			#endif
				projectors_send_command_all(PARAMETER_MODE, PARAMETER_CMD_RANDOM_ACCESS, TIMES[current_step][0]);
				chrono.restart();
			}	
			break;

		default:

		case STATE_VIEWING_1:
			if(TIMES[current_step][1] > 0) {
			
			#ifdef DEBUG
				Serial.print("waiting on shutter 1 for ");
				Serial.println(TIMES[current_step][1]);
			#endif

				projectors_send_command(PROJECTOR1, DIRECT_MODE, DIRECT_MODE_SHUTTER_OPEN);
			}
			chrono.restart();
			break;

		case STATE_VIEWING_2:
			if(TIMES[current_step][2] > 0) {
				
			#ifdef DEBUG
				Serial.print("waiting on shutter 2 for ");
				Serial.println(TIMES[current_step][2]);
			#endif

				projectors_send_command(PROJECTOR2, DIRECT_MODE, DIRECT_MODE_SHUTTER_OPEN);
			}
			chrono.restart();
			break;

		case STATE_VIEWING_3:
			if(TIMES[current_step][3] > 0) {

			#ifdef DEBUG
				Serial.print("waiting on shutter 3 for ");
				Serial.println(TIMES[current_step][3]);
			#endif

				projectors_send_command(PROJECTOR3, DIRECT_MODE, DIRECT_MODE_SHUTTER_OPEN);
			}
			chrono.restart();
			break;

		case STATE_VIEWING_ALL:
			empty_slides = 0;
			for(i = 1; i < ENTRY_SIZE; i++) {
				if(TIMES[current_step][i] == 0) {
					empty_slides += 1;
				}
			}

			view_all_time = VIEW_TIME_ALL;
			if(empty_slides == 1) {
				view_all_time = VIEW_TIME_TWO;
			} else if(empty_slides == 2) {
				view_all_time = VIEW_TIME_ONE;
			}

		#ifdef DEBUG
			Serial.print("waiting for ");
			Serial.println(view_all_time);
		#endif

			chrono.restart();
			break;

		case STATE_CLOSING_ALL:
			chrono.restart();
			projectors_send_command_all(DIRECT_MODE, DIRECT_MODE_SHUTTER_CLOSE);
			break;

		case STATE_CLOSING_ALL_END:
			closed_all_end = 1;
			chrono.restart();
			ignore_sensor_chrono.restart();
			projectors_send_command_all(DIRECT_MODE, DIRECT_MODE_SHUTTER_CLOSE);
			projectors_send_command_all(PARAMETER_MODE, PARAMETER_CMD_RANDOM_ACCESS, TIMES[0][0]);
			break;

		case STATE_STOPPED:
			projectors_send_command_all(SET_RESET_MODE, SET_RESET_CMD_STANDBY, true);
			chrono.stop();
			current_step = -1;
			break;

		case STATE_STOPPED_LOOP:
			projectors_send_command_all(SET_RESET_MODE, SET_RESET_CMD_STANDBY, true);
			chrono.restart();
			current_step = -1;
			break;

		case STATE_PRE_INIT:
			closed_all_end = 0;
			current_step = -1;
			projectors_send_command_all(SET_RESET_MODE, SET_RESET_CMD_STANDBY, true);
			projectors_send_command_all(SET_RESET_MODE, SET_RESET_CMD_AUTOSHUTTER, false);
			projectors_send_command_all(SET_RESET_MODE, SET_RESET_CMD_HIGHLIGHT, false);
			projectors_send_command_all(SET_RESET_MODE, SET_RESET_CMD_BLOCKKEYS, true);
			projectors_send_command_all(SET_RESET_MODE, SET_RESET_CMD_AUTOFOCUS, false);
			chrono.restart();
			break;

		case STATE_INIT:
			projectors_send_command_all(SET_RESET_MODE, SET_RESET_CMD_STANDBY, false);
			projectors_send_command_all(DIRECT_MODE, DIRECT_MODE_SHUTTER_CLOSE);
			projectors_send_command_all(PARAMETER_MODE, PARAMETER_CMD_RANDOM_ACCESS, TIMES[0][0]);
			chrono.restart();
			break;
	}

}

void set_state(int state) {

	exit_state(current_state);
	current_state = state;
	enter_state(state);

}

void setup() {
	
	Serial.begin(9600);

	current_state = 0;
	closed_all_end = 0;
	max_steps = sizeof(TIMES) / (sizeof(int) * ENTRY_SIZE);
	projectors_init();

	pinMode(BUTTON_PIN, INPUT_PULLUP);
   	prev_btn_state = digitalRead(BUTTON_PIN);

   	projectors_send_command_all(DIRECT_MODE, DIRECT_MODE_SHUTTER_CLOSE);
	projectors_send_command_all(PARAMETER_MODE, PARAMETER_CMD_RANDOM_ACCESS, TIMES[0][0]);

	set_state(STATE_STOPPED);

#ifdef DEBUG
	Serial.print("maxsteps ");
	Serial.println(max_steps);
#endif

	// delay(5000);
	// set_state(STATE_PRE_INIT);
}

void loop() {

	int new_btn_state = digitalRead(BUTTON_PIN);
	
	if(
		(
			!closed_all_end || 
			(closed_all_end && ignore_sensor_chrono.hasPassed(IGNORE_SENSOR_ON_STOP_TIME_PASSED_MS))
		) 
	  ) {

		if(btn_chrono.hasPassed(BUTTON_DEBOUNCE_MS)) {
			btn_chrono.restart();

		#ifdef DEBUG
			Serial.println("Button pressed");
		#endif

			if(new_btn_state == HIGH) {

				// if(chrono.isRunning()) {
				// 	projectors_send_command_all(SET_RESET_MODE, SET_RESET_CMD_STANDBY, true);
				// 	chrono.stop();
				// }
				#ifdef DEBUG
					Serial.println("Sensor off, just continue current state");
				#endif

			} else if(new_btn_state == LOW) {


				if(current_state == STATE_STOPPED || current_state == STATE_STOPPED_LOOP) {
					set_state(STATE_PRE_INIT);
				}
				// else if(!chrono.isRunning()) {
				// 	projectors_send_command_all(SET_RESET_MODE, SET_RESET_CMD_STANDBY, false);
				// 	chrono.restart();
				// }
			}
		}
	}

	switch(current_state) {
		

		case STATE_CHANGING:
			if(chrono.hasPassed(CHANGE_TIME)) {
				chrono.stop();
				set_state(STATE_VIEWING_1);
			}
			break;

		case STATE_VIEWING_1:
			if(chrono.hasPassed(TIMES[current_step][1])) {
				chrono.stop();
				set_state(STATE_VIEWING_2);
			}
			break;

		case STATE_VIEWING_2:
			if(chrono.hasPassed(TIMES[current_step][2])) {
				chrono.stop();
				set_state(STATE_VIEWING_3);
			}
			break;

		case STATE_VIEWING_3:
			if(chrono.hasPassed(TIMES[current_step][3])) {
				chrono.stop();
				set_state(STATE_VIEWING_ALL);
			}
			break;

		case STATE_VIEWING_ALL:
			if(chrono.hasPassed(view_all_time)) {
				chrono.stop();
				set_state(STATE_CLOSING_ALL);
			}
			break;

		case STATE_CLOSING_ALL:
			if(chrono.hasPassed(CLOSE_TIME)) {
				chrono.stop();
				set_state(STATE_CHANGING);
			}
			break;

		case STATE_CLOSING_ALL_END:
			if(chrono.hasPassed(CLOSE_TIME)) {
				chrono.stop();
				set_state(STATE_STOPPED);
			}

		case STATE_PRE_INIT:
			if(chrono.hasPassed(PRE_INIT_TIME)) {
				chrono.stop();
				set_state(STATE_INIT);
			}
			break;

		case STATE_INIT:
			if(chrono.hasPassed(INIT_TIME)) {
				chrono.stop();
				set_state(STATE_CHANGING);
			}
			break;

		case STATE_STOPPED_LOOP:
			if(chrono.hasPassed(STOP_BEFORE_LOOP_TIME)) {
				chrono.restart();
				set_state(STATE_PRE_INIT);
			}
			break;

	}

}