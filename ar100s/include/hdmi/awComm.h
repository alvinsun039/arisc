/****************************************************************************
 * hdmirx/awComm.h
 *
 * Copyright (C) 2022-2024 AllWinnertech Ltd.
 * Author: huangwenhao <huangwenhao@allwinnertech.com>
 ***************************************************************************/
#ifndef _COMM_H_
#define _COMM_H_

#include <stdint.h>

#define PACKET_LENGTH_A 8
#define PACKET_LENGTH_B 16
#define PACKET_LENGTH_C 32
#define PACKET_LENGTH_D 72

#define PACKET_START_BYTE 0xFF
#define PACKET_END_BYTE 0xFE

enum msgbox_processor_type {
	MESSAGE_TYPE_FROM_ARM,
	MESSAGE_TYPE_TO_ARM,
	MESSAGE_TYPE_FROM_DISPMIPS,
};

//#define ucData_LENGTH 66

typedef union tagPACKET_A {
	uint8_t Buffer[PACKET_LENGTH_A];
	struct _tagContentA_t {
		uint8_t STB;
		uint8_t Type;
		uint8_t SubType;
		uint8_t Len;
		uint8_t Data[PACKET_LENGTH_A - 6];
		uint8_t Checksum;
		uint8_t ETB;
	} Msg;
} PACKET_A;

typedef union tagPACKET_B {
	uint8_t Buffer[PACKET_LENGTH_B];
	struct _tagContentB_t {
		uint8_t STB;
		uint8_t Type;
		uint8_t SubType;
		uint8_t Len;
		uint8_t Data[PACKET_LENGTH_B - 6];
		uint8_t Checksum;
		uint8_t ETB;
	} Msg;
} PACKET_B;

typedef union tagPACKET_C {
	uint8_t Buffer[PACKET_LENGTH_C];
	struct _tagContentC_t {
		uint8_t STB;
		uint8_t Type;
		uint8_t SubType;
		uint8_t Len;
		uint8_t Data[PACKET_LENGTH_C - 6];
		uint8_t Checksum;
		uint8_t ETB;
	} Msg;
} PACKET_C;

typedef union tagPACKET_D {
	uint8_t Buffer[PACKET_LENGTH_D];
	struct _tagContentD_t {
		uint8_t STB;
		uint8_t Type;
		uint8_t SubType;
		uint8_t Len;
		uint8_t Data[PACKET_LENGTH_D - 6];
		uint8_t Checksum;
		uint8_t ETB;
	} Msg;
} PACKET_D;

typedef union tagPACKETDATA {
	PACKET_A PacketA;
	PACKET_B PacketB;
	PACKET_C PacketC;
	PACKET_D PacketD;
	PACKET_D Packet;
	uint8_t Buffer[PACKET_LENGTH_D];
} PACKETDATA;

typedef PACKETDATA* RPPACKETDATA;

extern PACKETDATA PacketSend;

void awSendPacketToHost(RPPACKETDATA rpPacket, uint8_t ucLen);

#define SendPacketToHostA(PacketSend) awSendPacketToHost(PacketSend, sizeof(PACKET_A))
#define SendPacketToHostB(PacketSend) awSendPacketToHost(PacketSend, sizeof(PACKET_B))
#define SendPacketToHostC(PacketSend) awSendPacketToHost(PacketSend, sizeof(PACKET_C))
#define SendPacketToHostD(PacketSend) awSendPacketToHost(PacketSend, sizeof(PACKET_D))

#define SendCECtoHost(x) SendPacketToHostC(x);

#endif  // _COMM_H_
