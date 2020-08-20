#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>
#include <printf.h>

#include "wireless.h"

#define ACK_TIMEOUT_MS 100
#define LOOP_LISTEN_TIME_MS 10

byte wireless_addresses[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node", "7Node"};

RF24 *radio;

void (*_w_ack_listener)(wirelessMessageAck);
void (*_w_resp_listener)(wirelessMessageResponse);
wirelessMessageAck (*_w_listener)(wirelessMessageCommand);
wirelessMessageResponse (*_w_listener_resp)(wirelessMessageCommand);

void checkIfMultipleMessages();

bool listening;
int localAddress;

long lastLoopListenTime = 0;

unsigned long startWaitingForAckTime;

bool sendMessage;
wirelessMessageCommand messageToSend;
byte sendMessagetargetAddress;

int targetAddressesLen;
int targetAddressIndex;

bool sendMesssageMultiple;

void _wireless_setup_remote(int pinA, int pinB) {

	listening = false;

	localAddress = 0;
	radio = new RF24(pinA, pinB);

	radio->begin();
	radio->setPALevel(RF24_PA_HIGH);
	radio->setDataRate(RF24_250KBPS);
	radio->setChannel(108);
	//radio->enableAckPayload();
	radio->enableDynamicPayloads();
	radio->setRetries(10, 15);

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
	radio->setPALevel(RF24_PA_HIGH);
	radio->setDataRate(RF24_250KBPS);
	radio->setChannel(108);
	//radio->enableAckPayload();
	radio->enableDynamicPayloads();
	//radio->printDetails();
	radio->setRetries(15, 15);
	radio->openReadingPipe(1, wireless_addresses[localAddress]); // motor x
	radio->openWritingPipe(wireless_addresses[localAddress]); // motor x

}

void _wireless_send_message(int targetAddress, wirelessMessageCommand msg) {

	sendMessage = true;
	messageToSend = msg;
	sendMessagetargetAddress = targetAddress;

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

bool wireless_send_is_busy() {
	return sendMesssageMultiple;
}

void _wireless_loop(long milliseconds) {

	if(listening) {

		byte pipeNo;
		if (radio->available(&pipeNo)) {
			wirelessMessageCommand message;
			while (radio->available(&pipeNo)) {
				radio->read(&message, sizeof(message));
			}
			radio->stopListening();


			if(message.type == MESSAGE_GET_PARAMS || message.type == MESSAGE_MOTOR_STATUS) {
				wirelessMessageResponse resp = _w_listener_resp(message);
				
				delay(5);
				Serial.print(F("send resp message "));
				Serial.print(resp.type);
				Serial.print(F(" size "));
				Serial.println(sizeof(resp));

				if(!radio->write(&resp, sizeof(resp))) {
					Serial.print(F("Failed writing ack message "));
					Serial.println(resp.type);
				}
			} else {
				wirelessMessageAck ack = _w_listener(message);
				delay(5);
				Serial.print(F("send ack message size "));
				Serial.println(sizeof(ack));

				if(!radio->write(&ack, sizeof(ack))) {
					Serial.println(F("Failed writing ack message "));
				}
			}
			
			delay(5);
			radio->startListening();
		}

		//lastLoopListenTime = milliseconds;

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
			Serial.println(F("Failed, response timed out."));
		} else {

			if(messageToSend.type == MESSAGE_GET_PARAMS || messageToSend.type == MESSAGE_MOTOR_STATUS) {
				
				wirelessMessageResponse respMessage;
				radio->read(&respMessage, sizeof(respMessage));
				//Serial.println(F("received resp right away "));
				if(_w_resp_listener != NULL) {
					_w_resp_listener(respMessage);
				}

			} else {

				wirelessMessageAck ackMessage;
				radio->read(&ackMessage, sizeof(ackMessage));
				//Serial.println(F("received ack right away "));
				if(_w_ack_listener != NULL) {
					_w_ack_listener(ackMessage);
				}
			}
		}

		sendMessage = false;
		radio->stopListening();
		checkIfMultipleMessages();
	}
}

void wireless_send_message(int targetAddress, wirelessMessageCommand msg) {

	listening = false;

	targetAddressIndex = -1;
	sendMesssageMultiple = false;

	_wireless_send_message(targetAddress, msg);
}

void wireless_send_message_all(wirelessMessageCommand msg) {

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

void wireless_listen_ack(void (*f)(wirelessMessageAck)) {
	_w_ack_listener = f;
}

void wireless_listen_response(void (*f)(wirelessMessageResponse)) {
	_w_resp_listener = f;
}

void wireless_listen(int targetAddress, wirelessMessageAck (*f)(wirelessMessageCommand), wirelessMessageResponse (*fr)(wirelessMessageCommand)) {

	_w_listener = f;
	_w_listener_resp = fr;

	Serial.print(F("Start listening Writing to "));
	Serial.print(targetAddress);
	Serial.print(F(" Reading from "));
	Serial.println(localAddress);

	listening = true;

	radio->startListening();
}
