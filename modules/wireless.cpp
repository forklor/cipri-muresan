#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>
#include <printf.h>

#include "wireless.h"

RF24 *radio;
//RF24 radio(9,10);

// Topology
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };

void (*_w_listener)(wirelessMessage);
bool listening;

void _wireless_setup(int pinA, int pinB) {

	listening = false;
	radio = new RF24(pinA, pinB);

	printf_begin();
	radio->begin();
	radio->setAutoAck(1);
	radio->enableAckPayload();
	radio->setRetries(0,15);
	radio->enableDynamicPayloads();
	radio->openWritingPipe(pipes[1]);
	radio->openReadingPipe(1,pipes[0]);
}

void _wireless_loop(long milliseconds) {

	if(!listening) return;

	byte pipeNumber;
	if (radio->available(&pipeNumber)) {
		wirelessMessage message;
		while (!radio->available(&pipeNumber)) {
			radio->read(&message, sizeof(message));
			radio->writeAckPayload(&pipeNumber, &message, sizeof(message));
		}
		_w_listener(message);
	}
}

void wireless_listen(void (*f)(wirelessMessage)) {
	
	listening = true;

	_w_listener = f;
	radio->startListening();
	radio->printDetails();
}

