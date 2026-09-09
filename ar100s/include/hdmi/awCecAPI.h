/****************************************************************************
 * hdmirx/awCecAPI.h
 *
 * Copyright (C) 2022-2024 AllWinnertech Ltd.
 * Author: huangwenhao <huangwenhao@allwinnertech.com>
 ***************************************************************************/
#ifndef CECAPI_H_
#define CECAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

//======================================================================
#define MAX_PING_COUNTER 1  // how many times of ping all logic adderss when enable auto ping

#define CEC_PA_HDMI1 0x1000
#define CEC_PA_HDMI2 0x2000
#define CEC_PA_HDMI3 0x3000
//#define CEC_PA_HDMI4    0x4000
//#define CEC_PA_HDMI5    0x5000
#define CEC_PA_WAITING 0xFFFF

#define CEC_LA_00 0   // TV
#define CEC_LA_01 1   // REC1
#define CEC_LA_05 5   // Audio System
#define CEC_LA_0C 12  // Reserved
#define CEC_LA_0D 13  // Reserved
#define CEC_LA_0E 14  // FreeUse
#define CEC_LA_0F 15  // all device

//-----------------------------------------------------------------------
typedef enum                      // Setting main chip power status //Difine from UART protocol
{ POWER_IS_READY = 0x00,          // Power is ready
  POWER_IS_STANDBY = 0x01,        // Poser is standby
  POWER_IS_GOING_TO_STBY = 0x02,  // Power is going to standby
  POWER_IS_STBY_TO_READY = 0x03   // Power is standby to ready
} POWER_STA;

typedef enum                     // Uart protocol response ACK or DATA
{ RESPONSE_NULL = 0x00,          // Not response data
  RESPONSE_ACT_SRC = 0x01,       // Response active source
  RESPONSE_CEC_EN = 0x02,        // Response CEC enable/disable
  RESPONSE_MENU_LANG = 0x03,     // Response menu language
  RESPONSE_PWR_STA = 0x04,       // Response power status
  RESPONSE_DECK_STA = 0x05,      // Response deck status
  RESPONSE_MENU_STA = 0x06,      // Response device menu status
  RESPONSE_PHYSICAL_ADR = 0x07,  // Response physical address
  RESPONSE_CEC_VER = 0x08,       // Response CEC version
  RESPONSE_VENDOR_ID = 0x09      // Response vendor id
} RESPONSE_STA;

typedef enum {
    MSG_TPY_RES_CEC_MSG = 0xC0,  // P8 received CEC message from CEC line
    MSG_TPY_RES_ACT_SRC = 0xC3,  // Response active source/physical address to main chip
    //  MSG_TPY_RES_CEC_EN          = 0xCB,         //Response CEC enable/disable to main chip//v5E disable
    MSG_TPY_RES_MENU_LANG = 0xCC,  // Response menu language to main chip
    MSG_TPY_RES_PWR_STA = 0xCD,    // Response power status to main chip
    MSG_TPY_RES_UART_STA = 0xCE,   // Response Uart communication status to main chip
    MSG_TPY_RES_CEC_ACK = 0xCF,    // Response CEC status, like ACK or NACK to main chip

    MSG_TPY_SEND_IR_CODE = 0x49,  // Micro send ir code to host
    MSG_TPY_SEND_KEY_CODE = 0x4B  // Micro send key code to host
} MSG_TYPE_TX;

typedef enum {
    MSG_TPY_RQS_CEC_MSG = 0x40,  // Main chip send CEC meaage request
    MSG_TPY_RQS_ACT_SRC = 0x43,  // Request active source/physical address from main chip
    //  MSG_TPY_RQS_CEC_EN          = 0x4B,         //Request CEC enable/disable from main chip/            /v5E disable
    MSG_TPY_RQS_MENU_LANG = 0x4C,  // Request menu language from main chip
    MSG_TPY_RQS_PWR_STA = 0x4D,    // Request power status from main chip
    MSG_TPY_RQS_UART_STA = 0x4E,   // Request Uart communication status from main chip
    MSG_TPY_RQS_CEC_ACK = 0x4F,    // Request CEC status, like ACK or NACK from main chip
    MSG_TPY_SET_CEC_EN = 0x5B,     // Set CEC enable/disable
    MSG_TPY_SET_MENU_LANG = 0x5C,  // Set menu language
    MSG_TPY_SET_PWR_STA = 0x5D     // Set power status
} MSG_TYPE_RX;

enum CECOPCode {
    OPCODE_FEATURE_ABORT = 0x00,
    OPCODE_IMAGE_VIEW_ON = 0x04,
    OPCODE_TUNER_STEP_INC = 0x05,
    OPCODE_TUNER_STEP_DEC = 0x06,
    OPCODE_TUNER_DEVICE_STATUS = 0x07,
    OPCODE_GIVE_TUNER_DEVICE_STATUS = 0x08,
    OPCODE_RECORD_ON = 0x09,
    OPCODE_RECORD_STATUS = 0x0A,
    OPCODE_RECORD_OFF = 0x0B,
    OPCODE_TEXT_VIEW_ON = 0x0D,
    OPCODE_RECORD_TV_SCREEN = 0x0F,
    OPCODE_GIVE_DECK_STATUS = 0x1A,
    OPCODE_DECK_STATUS = 0x1B,
    OPCODE_SET_MENU_LANGUAGE = 0x32,
    OPCODE_CLEAR_ANALOGUE_TIMER = 0x33,
    OPCODE_SET_ANALOGUE_TIMER = 0x34,
    OPCODE_TIMER_STATUS = 0x35,
    OPCODE_STANDBY = 0x36,
    OPCODE_PLAY = 0x41,
    OPCODE_DECK_CONTROL = 0x42,
    OPCODE_TIMER_CLEARED_STATUS = 0x43,
    OPCODE_USER_CONTROL_PRESSED = 0x44,
    OPCODE_USER_CONTROL_RELEASED = 0x45,
    OPCODE_GIVE_OSD_NAME = 0x46,
    OPCODE_SET_OSD_NAME = 0x47,
    OPCODE_SET_OSD_STRING = 0x64,
    OPCODE_SET_TIMER_PROGRAM_TITLE = 0x67,
    OPCODE_SYSTEM_AUDIO_MODE_REQUEST = 0x70,
    OPCODE_GIVE_AUDIO_STATUS = 0x71,
    OPCODE_SET_SYSTEM_AUDIO_MODE = 0x72,
    OPCODE_REPORT_AUDIO_STATUS = 0x7A,
    OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS = 0X7D,
    OPCODE_SYSTEM_AUDIO_MODE_STATUS = 0x7E,
    OPCODE_ROUTING_CHANGE = 0x80,
    OPCODE_ROUTING_INFORMATION = 0x81,
    OPCODE_ACTIVE_SOURCE = 0x82,
    OPCODE_GIVE_PHYSICAL_ADDRESS = 0x83,
    OPCODE_REPORT_PHYSICAL_ADDRESS = 0x84,
    OPCODE_REQUEST_ACTIVE_SOURCE = 0x85,
    OPCODE_SET_STREAM_PATH = 0x86,
    OPCODE_DEVICE_VENDOR_ID = 0x87,
    OPCODE_VENDOR_COMMAND = 0x89,
    OPCODE_VENDOR_REMOTE_BUTTON_DOWN = 0x8A,
    OPCODE_VENDOR_REMOTE_BUTTON_UP = 0x8B,
    OPCODE_GIVE_DEVICE_VENDOR_ID = 0x8C,
    OPCODE_MENU_REQUEST = 0x8D,
    OPCODE_MENU_STATUS = 0x8E,
    OPCODE_GIVE_DEVICE_POWER_STATUS = 0x8F,
    OPCODE_VENDOR_REMOTE_BOTTON_DOWN = 0x8A,
    OPCODE_VENDOR_REMOTE_BOTTON_UP = 0x8B,
    OPCODE_REPORT_POWER_STATUS = 0x90,
    OPCODE_GET_MENU_LANGUAGE = 0x91,
    OPCODE_SELECT_ANALOGUE_SERVICE = 0x92,
    OPCODE_SELECT_DIGITAL_SERVICE = 0x93,
    OPCODE_SET_DIGITAL_TIMER = 0x97,
    OPCODE_CLEAR_DIGITAL_TIMER = 0x99,
    OPCODE_SET_AUDIO_RATE = 0x9A,
    OPCODE_INACTIVE_SOURCE = 0x9D,
    OPCODE_CEC_VERSION = 0x9E,
    OPCODE_GET_CEC_VERSION = 0x9F,
    OPCODE_VENDOR_COMMAND_WITH_ID = 0xA0,
    OPCODE_CLEAR_EXTERNAL_TIMER = 0xA1,
    OPCODE_SET_EXTERNAL_TIMER = 0xA2,

    OPCODE_REPORT_SHORT_AUDIO_DESCRIPTOR = 0xA3,
    OPCODE_INIT_ARC = 0xC0,
    OPCODE_REPORT_ARC_INIT = 0xC1,
    OPCODE_REPORT_ARC_TERM = 0xC2,
    OPCODE_REQUEST_ARC_INIT = 0xC3,
    OPCODE_REQUEST_ARC_TERM = 0xC4,
    OPCODE_TERM_ARC = 0xC5,
    OPCODE_ABORT = 0xFF
};

typedef enum {
    AbortReason_Unrecognized_Opcode = 0,
    AbortReason_Not_In_Correct_Mode,
    AbortReason_Can_Not_Provide_Source,
    AbortReason_Invalid_Oprand,
    AbortReason_Refused,
    AbortReason_Unable_To_Determine,
} ABORT_REASON;

typedef enum {
    DeviceType_TV = 0,
    DeviceType_Record,
    DeviceType_Reserved,
    DeviceType_Tuner,
    DeviceType_Playback,
    DeviceType_Audio,
    DeviceType_PureCECSwitch,
    DeviceType_VideoProc,
    DeviceType_FreeUse,
    DeviceType_Unreg,
    DeviceType_Invalid = 0xFF,
} TDeviceType;

enum {  // HOST_TO_MCU_CEC_SETTING
    HOST_CEC_CMD_EnableCEC = 01,
    HOST_CEC_CMD_SetVendorID,
    HOST_CEC_CMD_SetPowerStatus,
    HOST_CEC_CMD_ARCOnly,
    HOST_CEC_CMD_PingAllDevice,
    HOST_CEC_CMD_WakeupEnable,
};

//#define MSG_CMD_DECK_CONTROL 0x66

//#define MSG_CMD_SEND_CEC_CMD 0x6F
//=============================================
// For host send get command to micro
// micro need response
//#define MSG_CMD_GET_DEVICE_PA 0x72  // Get Device Pyhsicall Address
/*reserved CEC msg commands types */
//#define MSG_CMD_GET_DEVICE_VENDOR_ID 0x75

//#define MSG_CMD_GET_MY_OWN_LA 0x76
//#define MSG_CMD_GET_CEC_VERSION 0x7B  // Get CEC driver's version
//#define MSG_CMD_ENABLE_CEConAuxPort 0x7C
//#define MSG_CMD_CHK_DEVICE_ONLINE 0x7D  // check presence of devices

// Micro response MSG to main chip
#define MSG_CMD_RES_ALL_DEVICE_LA 0xF0
#define MSG_CMD_RES_HISTORY 0xF1
#define MSG_CMD_RES_DEVICE_PA 0xF2
#define MSG_CMD_RES_MY_OWN_LA 0xF6

/*reserved CEC response types */
#define MSG_CMD_RES_BYPASS 0xFA
//#define MSG_CMD_RES_PROCESSED 0xFB
#define MSG_CMD_RES_SENDBACK 0xFC

// Micro send MSG to main chip
//#define MSG_CMD_RES_ACTIVE_CH 0xE1  // Active channel now
//#define MSG_CMD_RES_CEC_WAKEUP_EVENT 0xE2  // CEC wake up event

#ifdef __cplusplus
};
#endif
#endif /*CECAPI_H_*/
