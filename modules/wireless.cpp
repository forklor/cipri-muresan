#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>

#include "wireless.h"

#define ACK_TIMEOUT_MS 200

byte wireless_addresses[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node", "7Node"};

RF24 *radio;

void (*_w_ack_listener)(wirelessMessage);
void (*_w_send_timeout_listener)(wirelessMessage);
wirelessMessage (*_w_listener)(wirelessMessage);

bool listening;
int localAddress;

bool waitingForAck;
unsigned long startWaitingForAckTime;

bool sendMessage;
wirelessMessage messageToSend;

int *targetAddresses;
int targetAddressesLen;
int targetAddressIndex;
bool sendMesssageMultiple;

int __targetAddress = 0;

void _wireless_setup(int pinA, int pinB, int address) {

	listening = false;
	waitingForAck = false;
	sendMessage = false;

	localAddress = address;
	radio = new RF24(pinA, pinB);

	radio->begin();
	radio->setPALevel(RF24_PA_HIGH);
	//radio->setChannel(76);
	radio->enableAckPayload();
	radio->enableDynamicPayloads();
	//radio->printDetails();
}


void checkIfMultipleMessages() {

	if(!sendMesssageMultiple) return;
	
	targetAddressIndex += 1;
	
	if(targetAddressIndex < targetAddressesLen) {
		//Serial.print(F("send message to index "));
		wireless_send_message(targetAddresses[targetAddressIndex], messageToSend);
	} else {
		Serial.println(F("finished sending message to multiple targets"));
		targetAddressIndex = -1;
		sendMesssageMultiple = false;
		free(targetAddresses);
	}
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
			wirelessMessage ack = _w_listener(message);
			radio->write(&ack, sizeof(ack));
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
			Serial.println(F("Failed, response timed out."));
			if(_w_send_timeout_listener != NULL) {
				_w_send_timeout_listener(messageToSend);
			}
			radio->stopListening();
			waitingForAck = false;
			checkIfMultipleMessages();
		} else {
			wirelessMessage ackMessage;
			while(radio->available()) {
				radio->read(&ackMessage, sizeof(ackMessage));
				if(_w_ack_listener != NULL) {
					_w_ack_listener(ackMessage);
				}
				radio->stopListening();
				waitingForAck = false;
			}
			if(!waitingForAck) {
				Serial.print(F("received ack"));
				Serial.println(__targetAddress);
				checkIfMultipleMessages();
			}
		}
	} else if(sendMessage) {

		radio->stopListening();
		if (!radio->write(&messageToSend, sizeof(messageToSend))) {
			Serial.println(F("Failed writing"));
		} 

		waitingForAck = true;
		startWaitingForAckTime = millis();
		radio->startListening();
		sendMessage = false;
	}
}


void wireless_send_message(int targetAddress, wirelessMessage msg) {

	Serial.print(F("send message from "));
	Serial.print(localAddress);
	Serial.print(F(" to "));
	Serial.println(targetAddress);
	__targetAddress = targetAddress;

	radio->openWritingPipe(wireless_addresses[targetAddress]); // module_x
	radio->openReadingPipe(1, wireless_addresses[localAddress]); // remote control

	radio->startListening();

	sendMessage = true;
	messageToSend = msg;
}

void wireless_send_message(int *targets, int targets_len, wirelessMessage msg) {

	targetAddresses = (int *)malloc(targets_len * sizeof(*targetAddresses));
	memcpy(targetAddresses, targets, targets_len * sizeof(*targetAddresses));

	sendMesssageMultiple = true;
	targetAddressIndex = 0;
	targetAddressesLen = targets_len;

	wireless_send_message(targetAddresses[targetAddressIndex], msg);
}


void wireless_listen_ack(void (*f)(wirelessMessage)) {
	_w_ack_listener = f;
}

void wireless_listen_send_timeout(void (*f)(wirelessMessage)) {
	_w_send_timeout_listener = f;
}

void wireless_listen(int targetAddress, wirelessMessage (*f)(wirelessMessage)) {
	
	_w_listener = f;
	
	Serial.print(F("Start listening Writing to "));
	Serial.print(targetAddress);
	Serial.print(F(" Reading from "));
	Serial.print(localAddress);
	Serial.print("\n");

	listening = true;

	radio->openWritingPipe(wireless_addresses[targetAddress]); // remtote control
	radio->openReadingPipe(1, wireless_addresses[localAddress]);  // module_x
	
	radio->startListening();
}
