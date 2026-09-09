/****************************************************************************
 * hdmirx/awRam.h
 *
 * Copyright (C) 2014-2016 AllWinnertech Ltd.
 * Author: huangwenhao <huangwenhao@allwinnertech.com>
 ***************************************************************************/
#ifndef __RAM_H
#define __RAM_H

#define RESERVED_AUDIO_BLOCK_SIZE 12

typedef struct tagGlobalData {
	//uint8_t ucHdmi5vStatus;       // Bit3: HDMI-4, Bit2: HDMI-3, Bit1: HDMI-2, Bit0: HDMI-1 (1:5v detected, 0:5v lost)
	uint8_t bCECPrint;  // 1: Enable CEC Debug print, 0: disable
						//   uint8_t ucCECWakeLA;
	uint8_t bEdidDataReady;  // 1: EDID data ready, 0: no EDID data, must download from ARM
	uint8_t bEdidRamReady;   // 1: EDID RAM ready, 0: EDID RAM is empty, 1: EDID RAM is filled with data
	uint8_t ucEdidHdmi20Setting;  // Bit3: HDMI4 use HDMI2.0, Bit2: HDMI3, Bit1: HDMI2, Bit0: HDMI1.

	uint8_t uc1000msUpdateFlags;
	uint8_t uc500msUpdateFlags;
	uint8_t uc50msUpdateFlags;
	uint8_t uc10msUpdateFlags;
	uint8_t uc10MsCount;

	uint8_t bHDMIPortNumber;
} GlobalData;

extern GlobalData g_Data;

/* 1000ms update flags */
#define tdReset1000msUpdateFlags()         \
    do {                                   \
		g_Data.uc1000msUpdateFlags = 0xFF; \
    } while (0)

/* 500ms update flags */
#define tdReset500msUpdateFlags()         \
    do {                                  \
		g_Data.uc500msUpdateFlags = 0xFF; \
    } while (0)

/* 50ms update flags */
#define tdReset50msUpdateFlags()         \
    do {                                 \
		g_Data.uc50msUpdateFlags = 0xFF; \
    } while (0)

#define isWakeupByEthFlag() ((g_Data.uc50msUpdateFlags & _BIT0_) == _BIT0_)
#define clrWakeupByEthFlag() (g_Data.uc50msUpdateFlags &= ~_BIT0_)

#define isWakeupByWLFlag() ((g_Data.uc50msUpdateFlags & _BIT1_) == _BIT1_)
#define clrWakeupByWLFlag() (g_Data.uc50msUpdateFlags &= ~_BIT1_)

#define isWakeupByBTFlag() ((g_Data.uc50msUpdateFlags & _BIT2_) == _BIT2_)
#define clrWakeupByBTFlag() (g_Data.uc50msUpdateFlags &= ~_BIT2_)

/* 10ms update flags */
#define tdReset10msUpdateFlags()         \
    do {                                 \
		g_Data.uc10msUpdateFlags = 0xFF; \
    } while (0)

#define isCheckDispmipsFlag() ((g_Data.uc10msUpdateFlags & _BIT2_) == _BIT2_)
#define clrCheckDispmipsFlag() (g_Data.uc10msUpdateFlags &= ~_BIT2_)
//#endif

#endif
