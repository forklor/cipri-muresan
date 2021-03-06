#include "timer.h"

#define MIN_STEP_TIME_MS 500

long currentTime;
long currentStateStartTime;
long lastStepTime;

bool runningState;
bool paused;

long runTime;
long stopTime;

void (*_t_listener)(bool running);

long timer_get_run_time() {
	return runTime;
}

long timer_get_stop_time() {
	return stopTime;
}

void timer_set_run_time(long time) {
	runTime = time;
}

void timer_set_stop_time(long time) {
	stopTime = time;
}

char *timer_get_display_time(long time) {
	int allSecs = time / 1000;
	int minutes = allSecs / 60;
	int seconds = allSecs % 60;
	char *result = (char *)malloc(5 * sizeof(char));
	sprintf(result, "%02d:%02d", minutes, seconds);
	return result;
}

void timer_stop() {
	paused = true;
}

void timer_start() {
	paused = false;
	currentStateStartTime = millis();
}

void timer_set_state(bool running) {
	runningState = running;
	currentStateStartTime = millis();
}

bool timer_is_paused() {
	return paused;
} 

bool timer_is_running() {
	return runningState;
}

long timer_get_current_time() {
	return currentTime;
}

void _timer_setup(void (*f)(bool)) {

	_t_listener = f;
	currentTime = millis();
	currentStateStartTime = millis();
	lastStepTime = millis();

	runningState = false;
	paused = true;

	runTime = 30000L; // 30 seconds
	stopTime = 10000L * 60L; // 10 minutes
}

void _timer_loop(long milliseconds) {

	if((milliseconds - lastStepTime < MIN_STEP_TIME_MS) || paused) return;
	
	lastStepTime = milliseconds;
	long maxTime = runningState ? runTime : stopTime;
	currentTime = maxTime - (milliseconds - currentStateStartTime);
	if(currentTime <= 0) {
		currentTime = 0;
		_t_listener(!runningState);
		timer_set_state(!runningState);
	}
}