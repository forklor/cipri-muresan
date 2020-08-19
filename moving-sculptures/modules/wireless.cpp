#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>
#include <printf.h>

#include "wireless.h"

#define ACK_TIMEOUT_MS 100
#define LOOP_LISTEN_TIME_MS 50

byte wireless_addresses[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node", "7Node"};

RF24 *radio;

void (*_w_ack_listener)(wirelessMessage);
void (*_w_send_timeout_listener)(int, wirelessMessage);
wirelessMessage (*_w_listener)(wirelessMessage);

void checkIfMultipleMessages();

bool listening;
int localAddress;

long lastLoopListenTime = 0;

unsigned long startWaitingForAckTime;

bool sendMessage;
wirelessMessage messageToSend;
byte sendMessagetargetAddress;

int targetAddressesLen;
int targetAddressIndex;

bool sendMesssageMultiple;

void _wireless_setup_remote(int pinA, int pinB) {

	listening = false;

	localAddress = 0;
	radio = new RF24(pinA, pinB);

	radio->begin();
	radio->setPALevel(RF24_PA_MAX);
	radio->setDataRate(RF24_1MBPS);
	radio->setChannel(108);
	//radio->enableAckPayload();
	radio->enableDynamicPayloads();
	radio->setRetries(15, 15);

	radio->openReadingPipe(0, wireless_addresses[1]); // motor 1
	radio->openReadingPipe(1, wireless_addresses[2]); // motor 2
	radio->openReadingPipe(2, wireless_addresses[3]); // motor 3
	radio->openReadingPipe(3, wireless_addresses[4]); // motor 3
	radio->openReadingPipe(4, wireless_addresses[5]); // motor 3
	radio->openReadingPipe(5, wireless_addresses[6]); // motor 6

	printf_begin();
	radio->printDetails();

}

void _wireless_setup(int pinA, int pinB, int address) {

	listening = false;

	localAddress = address;
	radio = new RF24(pinA, pinB);

	radio->begin();
	radio->setPALevel(RF24_PA_MAX);
	radio->setDataRate(RF24_1MBPS);
	radio->setChannel(108);
	//radio->enableAckPayload();
	radio->enableDynamicPayloads();
	//radio->printDetails();
	radio->setRetries(15, 15);
	radio->openReadingPipe(1, wireless_addresses[localAddress]); // motor x
	radio->openWritingPipe(wireless_addresses[localAddress]); // motor x

}

void _wireless_send_message(int targetAddress, wirelessMessage msg) {

	sendMessage = true;
	messageToSend = msg;
	sendMessagetargetAddress = targetAddress;

}

void _wireless_send_message(wirelessMessage msg) {

	_wireless_send_message(localAddress, msg);
}

void checkIfMultipleMessages() {

	if(!sendMesssageMultiple) return;

	targetAddressIndex += 1;

	if(targetAddressIndex < targetAddressesLen) {
		_wireless_send_message(targetAddressIndex + 1, messageToSend);
	} else {
		//Serial.println(F("finished sending message to multiple targets"));
		targetAddressIndex = -1;
		sendMesssageMultiple = false;
	}
}

void _wireless_loop(long milliseconds) {

	if(listening && milliseconds - lastLoopListenTime >= LOOP_LISTEN_TIME_MS) {

		byte pipeNo;
		if (radio->available(&pipeNo)) {
			wirelessMessage message;
			while (radio->available(&pipeNo)) {
				radio->read(&message, sizeof(message));
			}
			radio->stopListening();
			wirelessMessage ack = _w_listener(message);
			delay(5);
			Serial.print(F("send ack message "));
			Serial.print(ack.type);
			Serial.print(F(" size "));
			Serial.println(sizeof(ack));

			if(!radio->write(&ack, sizeof(ack))) {
				Serial.print(F("Failed writing ack message "));
				Serial.println(ack.type);
			}
			delay(5);
			radio->startListening();
		}

		lastLoopListenTime = milliseconds;
	} else if(sendMessage) {

		Serial.print(F("send message from "));
		Serial.print(localAddress);
		Serial.print(F(" to "));
		Serial.print(sendMessagetargetAddress);
		Serial.print(F(" msg "));
		Serial.print(messageToSend.type);
		Serial.print(F(" size "));
		Serial.println(sizeof(messageToSend));



		radio->stopListening();
		radio->openWritingPipe(wireless_addresses[sendMessagetargetAddress]); // module_x

		if (!radio->write(&messageToSend, sizeof(messageToSend))) {
			Serial.print(F("Failed writing message "));
			Serial.println(messageToSend.type);
		}

		delay(5);

		startWaitingForAckTime = millis();

		radio->startListening();

		bool timeout = false;
		while(!radio->available() && !timeout) {
			if(millis() - startWaitingForAckTime > ACK_TIMEOUT_MS) {
				timeout = true;
			}
		}

		if(timeout) {
			Serial.println(F("Failed, response timed out new."));
		} else {
			wirelessMessage ackMessage;
			radio->read(&ackMessage, sizeof(ackMessage));
			Serial.println(F("received ack right away "));
			if(_w_ack_listener != NULL) {
				_w_ack_listener(ackMessage);
			}
		}

		sendMessage = false;
		radio->stopListening();
		checkIfMultipleMessages();
	}
}

void wireless_send_message(wirelessMessage msg) {

	wireless_send_message(localAddress, msg);
}

void wireless_send_message(int targetAddress, wirelessMessage msg) {

	listening = false;

	targetAddressIndex = -1;
	sendMesssageMultiple = false;

	_wireless_send_message(targetAddress, msg);
}

void wireless_send_message_all(wirelessMessage msg) {

	listening = false;
	sendMessage = false;

	Serial.print(F("Send multiple messages to "));
	Serial.print(6);
	Serial.print(F(" msg "));
	Serial.println(msg.type);

	sendMesssageMultiple = true;
	targetAddressIndex = 0;
	targetAddressesLen = 6;

	messageToSend = msg;

	_wireless_send_message(targetAddressIndex + 1, messageToSend);
}

void wireless_listen_ack(void (*f)(wirelessMessage)) {
	_w_ack_listener = f;
}

void wireless_listen_send_timeout(void (*f)(int, wirelessMessage)) {
	_w_send_timeout_listener = f;
}

void wireless_listen(int targetAddress, wirelessMessage (*f)(wirelessMessage)) {

	_w_listener = f;

	Serial.print(F("Start listening Writing to "));
	Serial.print(targetAddress);
	Serial.print(F(" Reading from "));
	Serial.println(localAddress);

	listening = true;

	radio->startListening();
}
