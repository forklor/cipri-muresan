#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>

#include "wireless.h"

#define ACK_TIMEOUT_MS 2000

byte wireless_addresses[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node", "7Node"};

RF24 *radio;

void (*_w_listener)(wirelessMessage);

bool listening;
int localAddress;

bool waitingForAck;
unsigned long startWaitingForAckTime;

bool sendMessage;
wirelessMessage messageToSend;

void _wireless_setup(int pinA, int pinB, int address) {

	listening = false;
	waitingForAck = false;
	sendMessage = false;

	localAddress = address;
	radio = new RF24(pinA, pinB);

	radio->begin();
	radio->enableAckPayload();
	radio->enableDynamicPayloads();
}

void _wireless_loop(long milliseconds) {

	if(listening) {

		byte pipeNo;
		if (radio->available(&pipeNo)) {
			wirelessMessage message;
			while (radio->available(&pipeNo)) {
				radio->read(&message, sizeof(message));
			}
			radio->stopListening();
			radio->write(&message, sizeof(message));
			_w_listener(message);
			radio->startListening();
		}

	} else if(waitingForAck) {
		bool timeout = false;
		if(!radio->available()) {
			if(milliseconds - startWaitingForAckTime > ACK_TIMEOUT_MS) {
				timeout = true;
			}
		}

		if (timeout) {
			Serial.println("Failed, response timed out.");
			radio->stopListening();
			waitingForAck = false;
		} else {
			wirelessMessage ackMessage;
			while(radio->available()) {
				radio->read(&ackMessage, sizeof(ackMessage));
				Serial.print("Got ack");
				radio->stopListening();
				waitingForAck = false;
			}
		}
	} else if(sendMessage) {

		radio->stopListening();

		if (!radio->write(&messageToSend, sizeof(messageToSend))) {
			Serial.println("Failed writing");
			delay(1000);
		} else {
			waitingForAck = true;
			startWaitingForAckTime = millis();
			radio->startListening();
			sendMessage = false;
		}
	}
}

void wireless_send_message(int targetAddress, wirelessMessage msg) {

	radio->openWritingPipe(wireless_addresses[localAddress]); // remote_control
	radio->openReadingPipe(1, wireless_addresses[targetAddress]); // module_x

	radio->startListening();

	sendMessage = true;
	messageToSend = msg;
}

void wireless_listen(int targetAddress, void (*f)(wirelessMessage)) {
		
	Serial.print("Start listening Writing to ");
	Serial.print(targetAddress);
	Serial.print(" Reading from ");
	Serial.print(localAddress);
	Serial.print("\n");

	listening = true;
	_w_listener = f;

	radio->openWritingPipe(wireless_addresses[localAddress]); // module_x
	radio->openReadingPipe(1, wireless_addresses[targetAddress]);  // remote_control
	
	radio->startListening();
}

