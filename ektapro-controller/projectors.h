#ifndef PROJECTORS_H_
#define PROJECTORS_H_

#include <Arduino.h>

//extern MenuBackend menu;

extern int PROJECTOR1;
extern int PROJECTOR2;
extern int PROJECTOR3;

// Part of byte 1, only the last 3 bits are relevant
extern byte PARAMETER_MODE;
extern byte DIRECT_MODE;
extern byte SET_RESET_MODE;
extern byte STATUS_REQUEST_MODE;

//Part of byte 2, only the first 4 bits are relevant
extern byte PARAMETER_CMD_RANDOM_ACCESS;
extern byte PARAMETER_CMD_SET_BRIGHTNES;

//Part of byte 2, only the first 6 bits are relevant
extern byte SET_RESET_CMD_AUTOFOCUS;
extern byte SET_RESET_CMD_HIGHLIGHT;
extern byte SET_RESET_CMD_AUTOSHUTTER;
extern byte SET_RESET_CMD_BLOCKKEYS;
extern byte SET_RESET_CMD_BLOCKFOCUS;
extern byte SET_RESET_CMD_STANDBY;

//Part of byte 2, only the first 6 bits are relevant
extern byte DIRECT_MODE_CMD_SLIDE_FWD;
extern byte DIRECT_MODE_CMD_SLIDE_BWD;
extern byte DIRECT_MODE_SHUTTER_OPEN;
extern byte DIRECT_MODE_SHUTTER_CLOSE;
extern byte DIRECT_MODE_RESET;

void projectors_init();

void projectors_send_command(int projector, byte type, byte command, int param);

void projectors_send_command(int projector, byte type, byte command, bool param);

void projectors_send_command(int projector, byte type, byte command);

void projectors_send_command_all(byte type, byte command, int param);

void projectors_send_command_all(byte type, byte command, bool param);

void projectors_send_command_all(byte type, byte command);

#endif //PROJECTORS_H_