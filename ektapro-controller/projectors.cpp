#include <SoftwareSerial.h>
#include "projectors.h"

//#define DEBUG

int PROJECTOR1 = 0;
int PROJECTOR2 = 1;
int PROJECTOR3 = 2;
// Part of byte 1, only the first 5 bits are relevant
// 								   BXXXXX___
byte DEFAULT_ADDRESS =             B11111000;

// Part of byte 1, only the last 3 bits are relevant
///								   B_____XXX
byte PARAMETER_MODE =              B00000001;
byte DIRECT_MODE =                 B00000101;
byte SET_RESET_MODE =              B00000011;
byte STATUS_REQUEST_MODE =         B00000111;

//Part of byte 2, only the first 4 bits are relevant
//								   BXXXX____
byte PARAMETER_CMD_RANDOM_ACCESS = B00000000;
byte PARAMETER_CMD_SET_BRIGHTNES = B00010000;

//Part of byte 2, only the first 6 bits are relevant
//								   BXXXXXX__
byte SET_RESET_CMD_AUTOFOCUS =     B00000000;
byte SET_RESET_CMD_HIGHLIGHT =     B00000100;
byte SET_RESET_CMD_AUTOSHUTTER =   B00001100;
byte SET_RESET_CMD_BLOCKKEYS =     B00010100;
byte SET_RESET_CMD_BLOCKFOCUS =    B00001000;
byte SET_RESET_CMD_STANDBY =       B00011100;

//Part of byte 2, only the first 6 bits are relevant
//								   BXXXXXX__
byte DIRECT_MODE_CMD_SLIDE_FWD =   B00000000;
byte DIRECT_MODE_CMD_SLIDE_BWD =   B00000100;
byte DIRECT_MODE_SHUTTER_OPEN =    B00011100;
byte DIRECT_MODE_SHUTTER_CLOSE =   B00100000;
byte DIRECT_MODE_RESET =           B00101100;

SoftwareSerial projector1(2, 3); // RX, TX
SoftwareSerial projector2(5, 4); // RX, TX
SoftwareSerial projector3(6, 7); // RX, TX

void projectors_init() {

	projector1.begin(9600);
	projector2.begin(9600);
	projector3.begin(9600);

}

void debug_command(byte type, byte cmd, int param) {

	Serial.print("type: ");
	
	if(type == PARAMETER_MODE) {

		Serial.print("PARAMETER_MODE");
		Serial.print(" cmd: ");

		if(cmd == PARAMETER_CMD_RANDOM_ACCESS) { 
			Serial.print("PARAMETER_CMD_RANDOM_ACCESS");
		}

		if(cmd == PARAMETER_CMD_SET_BRIGHTNES) {
			Serial.print("PARAMETER_CMD_SET_BRIGHTNES");
		}

		if(cmd == PARAMETER_CMD_SET_BRIGHTNES) {
			Serial.print("SET_RESET_CMD_AUTOFOCUS");
		}
	}

	if(type == DIRECT_MODE) {

		Serial.print("DIRECT_MODE");
		Serial.print(" cmd: ");

		if(cmd == DIRECT_MODE_CMD_SLIDE_FWD) {
			Serial.print("DIRECT_MODE_CMD_SLIDE_FWD");
		}

		if(cmd == DIRECT_MODE_CMD_SLIDE_BWD) {
			Serial.print("DIRECT_MODE_CMD_SLIDE_BWD");
		}

		if(cmd == DIRECT_MODE_SHUTTER_OPEN) {
			Serial.print("DIRECT_MODE_SHUTTER_OPEN");
		}

		if(cmd == DIRECT_MODE_SHUTTER_CLOSE) {
			Serial.print("DIRECT_MODE_SHUTTER_CLOSE");
		}

		if(cmd == DIRECT_MODE_RESET) {
			Serial.print("DIRECT_MODE_RESET");
		}
	}

	if(type == SET_RESET_MODE) {

		Serial.print("SET_RESET_MODE");
		Serial.print(" cmd: ");
		if(cmd == SET_RESET_CMD_AUTOSHUTTER) {
			Serial.print("SET_RESET_CMD_AUTOSHUTTER");
		}

		if(cmd == SET_RESET_CMD_BLOCKKEYS) {
			Serial.print("SET_RESET_CMD_BLOCKKEYS");
		}

		if(cmd == SET_RESET_CMD_BLOCKFOCUS) {
			Serial.print("SET_RESET_CMD_BLOCKFOCUS");
		}

		if(cmd == SET_RESET_CMD_STANDBY) {
			Serial.print("SET_RESET_CMD_STANDBY");
		}

		if(cmd == SET_RESET_CMD_HIGHLIGHT) {
			Serial.print("SET_RESET_CMD_HIGHLIGHT");
		}

		if(cmd == SET_RESET_CMD_AUTOFOCUS) {
			Serial.print("SET_RESET_CMD_AUTOFOCUS");
		}
	} 

	if(type == STATUS_REQUEST_MODE) {
		Serial.print("STATUS_REQUEST_MODE");
	}

	Serial.print(" param: ");
	Serial.println(param);
}

void debug_command(byte* cmd) {
	
	Serial.print("byte1: ");
	Serial.print(cmd[0], BIN);

	Serial.print(" byte2: ");
	Serial.print(cmd[1], BIN);

	Serial.print(" byte3: ");
	Serial.println(cmd[2], BIN);
}

byte get_command_byte1(byte type) {
	
	byte address = DEFAULT_ADDRESS;
	byte byte1 = address | type;
	return byte1;
}

byte get_command_byte2(byte type, byte command, int param) {

	byte byte2 = command;
	if(type == SET_RESET_MODE) {

		param = param << 1;
		byte2 = command | param;
	}
	return byte2;
}

byte get_command_byte3(byte type, byte command, int param) {
	byte byte3 = 0x0;
	if(type == PARAMETER_MODE) {

		if(command == PARAMETER_CMD_SET_BRIGHTNES ||
			command == PARAMETER_CMD_RANDOM_ACCESS) {
			byte3 = param << 1;
		}

	}
	return byte3;
}

void projectors_send_command(int projector, byte type, byte command, int param) {

	SoftwareSerial serial = (projector == PROJECTOR1 ? projector1 : (projector == PROJECTOR2 ? projector2 : projector3));
#ifdef DEBUG
	Serial.print("send to ");
	Serial.print(projector);
	Serial.print(" -> ");
	debug_command(type, command, param);
#endif

	byte byte1 = get_command_byte1(type);
	byte byte2 = get_command_byte2(type, command, param);
	byte byte3 = get_command_byte3(type, command, param);
	byte cmd[3] = {byte1, byte2, byte3};
	
#ifdef DEBUG
	Serial.print("send to ");
	Serial.print(projector);
	Serial.print(" -> ");
	debug_command(cmd);
#endif

	serial.write(cmd, sizeof(cmd));
}

void projectors_send_command(int projector, byte type, byte command, bool param) {
	projectors_send_command(projector, type, command, (int)param);
}

void projectors_send_command(int projector, byte type, byte command) {
	projectors_send_command(projector, type, command, 0);
}

void projectors_send_command_all(byte type, byte command, int param) {

#ifdef DEBUG
	Serial.print("send to all -> ");
	debug_command(type, command, param);
#endif

	byte byte1 = get_command_byte1(type);
	byte byte2 = get_command_byte2(type, command, param);
	byte byte3 = get_command_byte3(type, command, param);
	byte cmd[3] = {byte1, byte2, byte3};

#ifdef DEBUG
	Serial.print("send to all -> ");
	debug_command(cmd);
#endif

	projector1.write(cmd, sizeof(cmd));
	projector2.write(cmd, sizeof(cmd));
	projector3.write(cmd, sizeof(cmd));
}

void projectors_send_command_all(byte type, byte command, bool param) {
	projectors_send_command_all(type, command, (int)param);
}

void projectors_send_command_all(byte type, byte command) {
	projectors_send_command_all(type, command, 0);
}
