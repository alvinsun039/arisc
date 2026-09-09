/****************************************************************************
 * hdmirx/awHDMI.h
 *
 * Copyright (C) 2022-2024 AllWinnertech Ltd.
 * Author: huangwenhao <huangwenhao@allwinnertech.com>
 ***************************************************************************/

#ifndef __HDMI_H__
#define __HDMI_H__

//#include "include.h"

#define EDID_DATA_LEN 256
#define EDID_Array_LEN (EDID_DATA_LEN / 4)
#define Extended_EDID_Array_LEN (EDID_Array_LEN / 2)

#define TIME_OUT_RESET_HPD TIME_OUT_830MS  // use 830ms same as Samsung TV
#define TIME_OUT_RELOAD_EDID TIME_OUT_830MS

#define EDID_DWORD_PER_PACK 16
#define EDID_DATA_PER_PACK 64
#define EDID_PACK_NUM 4
#define LAST_PACK_TABLE1 (EDID_PACK_NUM - 1)
#define LAST_PACK_TABLE2 (EDID_PACK_NUM * 2 - 1)

typedef enum _tagHDMICmdID {
	HDMI_CMD_UpdateEDID,
	HDMI_CMD_EDIDStatus,
	HDMI_CMD_DynamicAudioMode,
	HDMI_CMD_EDIDVersion,
	//HDMI_CMD_HDMI5VFlag,
	HDMI_CMD_REQ_EDID_data,
	HDMI_CMD_PrintEDIDTable,
	HDMI_CMD_HDMIPortNumber,
	HDMI_CMD_HOTPLUG,
	HDMI_CMD_ResetEDIDModule,
} HDMICmdID;

typedef enum _tagEDID_Version {
	HDMI_EDID_Version_14,
	HDMI_EDID_Version_20,
} EDID_Version;

typedef enum _tagHDMI_HOTPLUG {
	CMD_HOTPLUG_PULLUP = 0x1,
	CMD_HOTPLUG_PULLDOWN = 0x2,
	CMD_HOTPLUG_RESET = 0x3,
} HDMI_HOTPLUG;

typedef struct _HDMI_MAP_STRUCT {
	/* port number follow UI sequence: "HDMI-1", "HDMI-2", "HDMI-3" */
	uint8_t port_mask;   /* port mask for bit shift: HDMI1_PORT_MASK ... */
	uint8_t pa;          /* physical address: 0x10 0x20 0x30 */
	uint8_t hotplug_pin; /* hotplug pin: 0 1 2  */
	uint8_t ddc_mask;    /* ddc port: DDC0_PORT_MASK ... */
	uint8_t display;     /* display port: P0, P1, P2 */
} HDMI_MAP_STRUCT;

enum _tagHDMI_PORT {
	HDMI1_PORT,
	HDMI2_PORT,
	HDMI3_PORT,
	HDMI_PORT_NUM_MAX,
	HDMI_PORT_INVALID = HDMI_PORT_NUM_MAX,
} HDMI_PORT;
//#define HDMI_PORT_NUM	3
extern uint8_t hotplug_pullup_timer[HDMI_PORT_NUM_MAX];

#define HDMI1_PORT_MASK _BIT0_
#define HDMI2_PORT_MASK _BIT1_
#define HDMI3_PORT_MASK _BIT2_
#define HDMI_ALL_PORT_MASK (HDMI1_PORT_MASK + HDMI2_PORT_MASK + HDMI3_PORT_MASK)

#define HDMI1EDID_Enable _BIT0_
#define HDMI2EDID_Enable _BIT1_
#define HDMI3EDID_Enable _BIT2_

#define HDMI1_PA 0x10  // HDMI1 has physical address 0x1000
#define HDMI2_PA 0x20  // HDMI2 has physical address 0x2000
#define HDMI3_PA 0x30  // HDMI3 has physical address 0x3000

/*
 * Display port no. for HDMI channel
 */
#define HDMI_DISPLAY_P0 0
#define HDMI_DISPLAY_P1 1
#define HDMI_DISPLAY_P2 2

/*
 * DDC channel mask
 */
#define DDC0_PORT_MASK _BIT0_
#define DDC1_PORT_MASK _BIT1_
#define DDC2_PORT_MASK _BIT2_

/*
 * HPD pin no. for HDMI channel
 */
#define HPD_PIN_0 0
#define HPD_PIN_1 1
#define HPD_PIN_2 2

#define EDID_DYNAMIC_MODE_OFF 0  // use default EDID table
#define EDID_DYNAMIC_MODE_ARC 1  // audio data block will be updated by SAD bytes from ARC device
#define EDID_DYNAMIC_MODE_DTS 2  // use default EDID table, and insert SAD bytes to support DTS
#define EDID_DYNAMIC_MODE_MAX 2

#define RegistrationID_0 0x03
#define RegistrationID_1 0x0C
#define RegistrationID_2 0x00

void awUpdateExpiringTimers(void);
void awHandleExpiredTimers(void);
void tdHdmiEdidInit(void);
void tdHdmiPinInit(void);
void tdHdmiResCalibrate(void);
void SysGPIOSetHotplug(uint8_t port, uint8_t bOn);
uint8_t awProcessHDMICommand(uint8_t Type, uint8_t *rpPacket);
// Void vsHdmiInit5vStatus(Void);
// uint8_t tdHdmiUpdate5vStatus(uint8_t mask, uint8_t value);
void tdHdmiEdidUpdate(uint8_t hdmi_port_masks);
void tdHdmiMapSet(uint8_t *pMap);

#endif /*__HDMI_H__*/
