#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>

#include "wireless.h"

#define ACK_TIMEOUT_MS 200000

byte wireless_addresses[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node", "7Node"};

RF24 *radio;

void (*_w_listener)(wirelessMessage);

bool listening;
int localAddress;

bool waitingForAck;
unsigned long startWaitingForAckTime;

void _wireless_setup(int pinA, int pinB, int address) {

	listening = false;
	waitingForAck = false;
	localAddress = address;
	radio = new RF24(pinA, pinB);

	radio->begin();
	radio->enableAckPayload();
	radio->enableDynamicPayloads();
}

void _wireless_loop(long milliseconds) {

	bool timeout = false;

	if(listening) {
		if (radio->available()) {
			Serial.println("radio available");
			wirelessMessage message;
			while (!radio->available()) {
				radio->read(&message, sizeof(message));
			}
			_w_listener(message);
			radio->stopListening();
			radio->write(&message, sizeof(message));
			radio->startListening();
		}
	} else if(waitingForAck) {

		while(!radio->available()) {
			if(milliseconds - startWaitingForAckTime > ACK_TIMEOUT_MS) {
				timeout = true;
				break;
			}
		}

		if (timeout) {
			Serial.println(F("Failed, response timed out."));
			radio->stopListening();
			waitingForAck = false;
		} else {
			wirelessMessage ackMessage;
			radio->read( &ackMessage, sizeof(ackMessage) );
			Serial.print("Got ack");
			radio->stopListening();
			waitingForAck = false;
		}
	}
}

void wireless_send_message(int targetAddress, wirelessMessage msg) {

	Serial.print("Writing to ");
	Serial.print(targetAddress);
	Serial.print(" Reading from ");
	Serial.print(localAddress);
	Serial.print("\n");


	radio->stopListening();

	radio->openWritingPipe(wireless_addresses[targetAddress]);
	radio->openReadingPipe(1, wireless_addresses[localAddress]);

	if (!radio->write(&msg, sizeof(msg))){
		Serial.println(F("failed writing"));
	} else {
		waitingForAck = true;
		startWaitingForAckTime = millis();
		radio->startListening();
	}	
}

void wireless_listen(int targetAddress, void (*f)(wirelessMessage)) {
		
	Serial.print("Start listening Writing to ");
	Serial.print(targetAddress);
	Serial.print(" Reading from ");
	Serial.print(localAddress);
	Serial.print("\n");

	listening = true;
	_w_listener = f;

	radio->openWritingPipe(wireless_addresses[localAddress]);
	radio->openReadingPipe(1, wireless_addresses[targetAddress]);
	
	radio->startListening();
}

