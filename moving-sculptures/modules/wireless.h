#ifndef WIRELESS_H_
#define WIRELESS_H_

#include "wireless_message.h"

#define WIRELESS_REMOTE 0
#define WIRELESS_MODULE_1 1
#define WIRELESS_MODULE_2 2
#define WIRELESS_MODULE_3 3
#define WIRELESS_MODULE_4 4
#define WIRELESS_MODULE_5 5
#define WIRELESS_MODULE_6 6

void _wireless_setup_remote(int pinA, int pinB);
void _wireless_setup(int pinA, int pinB, int localAddress);
void _wireless_loop(long milliseconds);

void wireless_listen_ack(void (*f)(wirelessMessageResponse));
void wireless_setAckResponse(wirelessMessageResponse msg);
void wireless_listen(int targetAddress, void (*f)(wirelessMessageCommand));
void wireless_send_message(int targetAddress, wirelessMessageCommand msg);
void wireless_send_message_all(wirelessMessageCommand msg);
void wireless_send_message_all_fail_listener(void (*f)(int));
bool wireless_send_is_busy();

#endif //WIRELESS_H_