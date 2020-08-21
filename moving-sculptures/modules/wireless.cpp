#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>
#include <printf.h>
#include "print.h"

#include "wireless.h"

#define ACK_TIMEOUT_MS 100
#define LOOP_LISTEN_TIME_MS 30

byte wireless_addresses[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node", "7Node"};

RF24 *radio;

void (*_w_ack_listener)(wirelessMessageResponse);
void (*_w_send_fail_listener)(int);
void (*_w_listener)(wirelessMessageCommand);

void checkIfMultipleMessages(bool retry);

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

wirelessMessageResponse ackResponse;
bool retry = false;

void _radio_setup(int pinA, int pinB) {

	radio = new RF24(pinA, pinB);
	radio->begin();
	radio->setPALevel(RF24_PA_MAX); // RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
	radio->setDataRate(RF24_250KBPS); // RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
	radio->setChannel(108);
	radio->enableAckPayload();
	radio->enableDynamicPayloads();
	radio->setRetries(15, 15);
}

void _wireless_setup_remote(int pinA, int pinB) {

	listening = false;

	localAddress = 0;
	_radio_setup(pinA, pinB);
	radio->stopListening();

	//printf_begin();
	//radio->printDetails();

}

void _wireless_setup(int pinA, int pinB, int address) {

	listening = false;

	localAddress = address;
	_radio_setup(pinA, pinB);
	radio->openReadingPipe(1, wireless_addresses[localAddress]); // motor x

	//printf_begin();
	//radio->printDetails();
}

void _wireless_send_message(int targetAddress, wirelessMessageCommand msg) {

	sendMessage = true;
	messageToSend = msg;
	sendMessagetargetAddress = targetAddress;

}

void checkIfMultipleMessages(bool retry) {

	if(!sendMesssageMultiple) return;

	if(!retry) {
		targetAddressIndex += 1;
	}

	if(targetAddressIndex < targetAddressesLen) {
		_wireless_send_message(targetAddressIndex + 1, messageToSend);
	} else {
		//s_println(F("finished sending message to multiple targets"));
		targetAddressIndex = -1;
		sendMesssageMultiple = false;
	}
}

bool wireless_send_is_busy() {
	return sendMesssageMultiple;
}

void _wireless_loop(long milliseconds) {

	if(listening && milliseconds - lastLoopListenTime >= LOOP_LISTEN_TIME_MS) {

		if (radio->available()) {
			wirelessMessageCommand message;
			radio->read(&message, sizeof(message));		
			
			_w_listener(message);

			if(ackResponse.type != 0) {
				radio->writeAckPayload(1, &ackResponse, sizeof(ackResponse));
			}
		}

		lastLoopListenTime = milliseconds;

	} else if(sendMessage) {

		s_print(F("send message from "));
		s_print(localAddress);
		s_print(F(" to "));
		s_print(sendMessagetargetAddress);
		s_print(F(" msg "));
		s_print(messageToSend.type);
		s_print(F(" size "));
		s_println(sizeof(messageToSend));

		radio->openWritingPipe(wireless_addresses[sendMessagetargetAddress]); // module_x

		if (!radio->write(&messageToSend, sizeof(messageToSend))) {
			if(retry) {
				s_print(F("Failed writing message after 2 tries"));
				s_println(messageToSend.type);
				if(_w_send_fail_listener != NULL) {
					_w_send_fail_listener(sendMessagetargetAddress);
				}
			}
			retry = !retry;

		} else {

			retry = false;
			startWaitingForAckTime = millis();

			bool timeout = false;
			while(!radio->isAckPayloadAvailable() && !timeout) {
				if(millis() - startWaitingForAckTime > ACK_TIMEOUT_MS) {
					timeout = true;
				}
			}

			if(timeout) {
				s_println(F("Failed, ack response timed out."));
			} else {

				wirelessMessageResponse ackResponse;
				radio->read(&ackResponse, sizeof(ackResponse));

				s_println(F("received ack response"));
				if(_w_ack_listener != NULL) {
					_w_ack_listener(ackResponse);
				}
			}
		}
		sendMessage = false;
		checkIfMultipleMessages(retry);
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

	s_print(F("Send message to all"));
	s_println(msg.type);

	sendMesssageMultiple = true;
	targetAddressIndex = 0;
	targetAddressesLen = 6;

	messageToSend = msg;

	_wireless_send_message(targetAddressIndex + 1, messageToSend);
}

void wireless_listen_ack(void (*f)(wirelessMessageResponse)) {
	_w_ack_listener = f;
}


void wireless_setAckResponse(wirelessMessageResponse msg) {
	ackResponse = msg;
}

void wireless_send_message_all_fail_listener(void (*f)(int)) {
	_w_send_fail_listener = f;
}

void wireless_listen(int targetAddress,  void (*f)(wirelessMessageCommand)) {

	_w_listener = f;

	s_print(F("Start listening from "));
	s_println(localAddress);

	listening = true;

	radio->startListening();

	if(ackResponse.type != 0) {
		radio->writeAckPayload(1, &ackResponse, sizeof(ackResponse));
	}
}
