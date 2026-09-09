/****************************************************************************
 * hdmirx/hdmi/awHDMI.c
 *
 * Copyright (C) 2022-2024 AllWinnertech Ltd.
 * Author: huangwenhao <huangwenhao@allwinnertech.com>
 ***************************************************************************/

#include "include.h"

/*
*********************************************************************************************************
*
*
* Description:
*
* Arguments  :  none.
*
* Returns    :
*********************************************************************************************************
*/
// uint8_t MsgBox[68];

#define CECDebounceCounter 24

uint8_t HOST_ASSIGNED_EDID_HDMI14[EDID_DATA_LEN] = {
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x5e, 0x78, 0x43, 0x48, 0x21, 0x03, 0x00, 0x00, 0x0d, 0x1a, 0x01, 0x03, 0x81, 0x46,
	0x27, 0x78, 0xeb, 0xd5, 0x7c, 0xa3, 0x57, 0x49, 0x9c, 0x25, 0x11, 0x48, 0x4b, 0xbf, 0xef, 0x00, 0xa9, 0x40, 0x81, 0x59, 0x81, 0x80,
	0x61, 0x59, 0x45, 0x59, 0x31, 0x59, 0x31, 0x19, 0x31, 0xd9, 0x04, 0x74, 0x00, 0x30, 0xf2, 0x70, 0x5a, 0x80, 0xb0, 0x58, 0x8a, 0x00,
	0xc4, 0x8e, 0x21, 0x00, 0x00, 0x1e, 0x02, 0x3a, 0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c, 0x45, 0x00, 0xc4, 0x8e, 0x21, 0x00,
	0x00, 0x1e, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x53, 0x47, 0x44, 0x20, 0x53, 0x58, 0x38, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00,
	0x00, 0xfd, 0x00, 0x17, 0x78, 0x0f, 0x8c, 0x1e, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x82, 0x02, 0x03, 0x52, 0xf1,
	0x55, 0x5f, 0x64, 0x5d, 0x62, 0x5e, 0x10, 0x22, 0x20, 0x1f, 0x14, 0x05, 0x04, 0x07, 0x06, 0x03, 0x02, 0x01, 0x11, 0x12, 0x13, 0x15,
	0x2c, 0x57, 0x06, 0x00, 0x3f, 0x07, 0x50, 0x09, 0x7f, 0x01, 0x15, 0x07, 0x50, 0x83, 0x4f, 0x00, 0x00, 0x70, 0x03, 0x0c, 0x00, 0x10,
	0x00, 0xb8, 0x3c, 0xaf, 0x5b, 0x5b, 0x80, 0x80, 0x01, 0x02, 0x03, 0x04, 0xeb, 0x01, 0x46, 0xd0, 0x00, 0x44, 0x6f, 0x6a, 0x8a, 0x58,
	0x65, 0xad, 0xe2, 0x00, 0xff, 0xe2, 0x0f, 0xf0, 0xe3, 0x06, 0x0f, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf1,
};// HDMI 1.4 edid

uint8_t HOST_ASSIGNED_EDID_HDMI20[EDID_DATA_LEN] = {
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x5e, 0x78, 0x43, 0x48, 0x21, 0x03, 0x00, 0x00, 0x0d, 0x1a, 0x01, 0x03, 0x81, 0x46,
	0x27, 0x78, 0xeb, 0xd5, 0x7c, 0xa3, 0x57, 0x49, 0x9c, 0x25, 0x11, 0x48, 0x4b, 0xbf, 0xef, 0x00, 0xa9, 0x40, 0x81, 0x59, 0x81, 0x80,
	0x61, 0x59, 0x45, 0x59, 0x31, 0x59, 0x31, 0x19, 0x31, 0xd9, 0x04, 0x74, 0x00, 0x30, 0xf2, 0x70, 0x5a, 0x80, 0xb0, 0x58, 0x8a, 0x00,
	0xc4, 0x8e, 0x21, 0x00, 0x00, 0x1e, 0x02, 0x3a, 0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c, 0x45, 0x00, 0xc4, 0x8e, 0x21, 0x00,
	0x00, 0x1e, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x53, 0x47, 0x44, 0x20, 0x53, 0x58, 0x38, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00,
	0x00, 0xfd, 0x00, 0x17, 0x78, 0x0f, 0x8c, 0x3c, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x64, 0x02, 0x03, 0x6d, 0xf1,
	0x55, 0x5f, 0x64, 0x5d, 0x62, 0x5e, 0x10, 0x22, 0x20, 0x1f, 0x14, 0x05, 0x04, 0x07, 0x06, 0x03, 0x02, 0x01, 0x11, 0x12, 0x13, 0x15,
	0x2c, 0x57, 0x06, 0x00, 0x3f, 0x07, 0x50, 0x09, 0x7f, 0x01, 0x15, 0x07, 0x50, 0x83, 0x4f, 0x00, 0x00, 0x70, 0x03, 0x0c, 0x00, 0x10,
	0x00, 0xb8, 0x3c, 0xaf, 0x5b, 0x5b, 0x80, 0x80, 0x01, 0x02, 0x03, 0x04, 0x67, 0xd8, 0x5d, 0xc4, 0x01, 0x78, 0xc8, 0x03, 0xeb, 0x01,
	0x46, 0xd0, 0x00, 0x44, 0x6f, 0x6a, 0x8a, 0x58, 0x65, 0xad, 0xe2, 0x00, 0xff, 0xe2, 0x0f, 0xf0, 0xe3, 0x06, 0x0f, 0x01, 0xe3, 0x05,
	0xe0, 0x00, 0x68, 0x1a, 0x00, 0x00, 0x01, 0x01, 0x30, 0x78, 0x00, 0xe5, 0x01, 0x8b, 0x84, 0x90, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb8,
};// HDMI 2.0 edid
uint8_t hotplug_pullup_timer[HDMI_PORT_NUM_MAX];  // delay timer before pull-up hotplug pin

uint8_t pEdidBuffer[EDID_DATA_LEN] = {0};

// Short Audio Descriptor for DTS, to be used when force support DTS.
//#define DTS_SAD_TABLE_LENGTH 3
// uint8_t DTS_SAD[DTS_SAD_TABLE_LENGTH] = {0x3E, 0x07, 0x50};  // DTS, 7 channels,  48/44/32Khz, max bitrate 640k.

void tdHdmiEdidEnable(uint8_t port_masks, uint8_t bEnable);
void SysGPIOResetHotplug(uint8_t port_mask, uint8_t bYesNo, uint8_t time);

HDMI_MAP_STRUCT hdmi_map_table[HDMI_PORT_NUM_MAX];

HDMI_MAP_STRUCT hdmi_soc_data[HDMI_PORT_NUM_MAX] = {
	{HDMI1_PORT_MASK, HDMI1_PA, HPD_PIN_0, DDC0_PORT_MASK, HDMI_DISPLAY_P0},
	{HDMI2_PORT_MASK, HDMI2_PA, HPD_PIN_1, DDC1_PORT_MASK, HDMI_DISPLAY_P1},
	{HDMI3_PORT_MASK, HDMI3_PA, HPD_PIN_2, DDC2_PORT_MASK, HDMI_DISPLAY_P2},
};

/*****************************************************************************
*  Name        : UpdateExpiringTimers
*  Description : to be called by tdCallBack1msTimer. (10ms step)
*  Params      :
*  Returns     :
*****************************************************************************/
void awUpdateExpiringTimers(void)
{
	uint8_t i;

	for (i = 0; i < HDMI_PORT_NUM_MAX; i++) {
		if (hotplug_pullup_timer[i] > TIMER_EXPIRED) {
			hotplug_pullup_timer[i]--;
		}
	}
}

/*****************************************************************************
*  Name        : HandleExpiredTimers
*  Description : to be called by tdCallBack1msTimer.
*  Params      :
*  Returns     :
*****************************************************************************/
void awHandleExpiredTimers(void)
{
	uint8_t i;

	for (i = 0; i < HDMI_PORT_NUM_MAX; i++) {
		if (hotplug_pullup_timer[i] == TIMER_EXPIRED) {
			LOG("hpd %d up by timer\n", i);
			hotplug_pullup_timer[i] = TIMER_INACTIVE;
			SysGPIOSetHotplug(HDMI1_PORT + i, _HI_);  // pull-up hotplug pin
		}
	}

}

/*
 * Init hdmi_map_table
 */
static void tdHdmiMapInit(void)
{
	uint8_t port;
	for (port = HDMI1_PORT; port < HDMI_PORT_NUM_MAX; port++) {
		hdmi_map_table[port].port_mask = hdmi_soc_data[port].port_mask;
		hdmi_map_table[port].pa = hdmi_soc_data[port].pa;
		hdmi_map_table[port].hotplug_pin = hdmi_soc_data[port].hotplug_pin;
		hdmi_map_table[port].ddc_mask = hdmi_soc_data[port].ddc_mask;
		hdmi_map_table[port].display = hdmi_soc_data[port].display;
	}
}

void tdHdmiMapSet(uint8_t *pMap)
{
	uint8_t port;
	for (port = HDMI1_PORT; port < HDMI_PORT_NUM_MAX; port++) {
		/* hdmi_map_table[port].port_mask is fixed value
		* hdmi_map_table[port].pa is fixed value
		*/
		hdmi_map_table[port].hotplug_pin = *(pMap + port);
		hdmi_map_table[port].ddc_mask = 1 << (*(pMap + port));
		hdmi_map_table[port].display = *(pMap + port);

		INF("[%x].port_mask = %x\n", port, hdmi_map_table[port].port_mask);
		INF("[%x].pa =%x \n", port, hdmi_map_table[port].pa);
		INF("[%x].hotplug_pin = %x\n", port, hdmi_map_table[port].hotplug_pin);
		INF("[%x].ddc_mask = %x\n", port, hdmi_map_table[port].ddc_mask);
		INF("[%x].display = %x\n", port, hdmi_map_table[port].display);
	}
	tdHdmiEdidUpdate(HDMI_ALL_PORT_MASK);
}

uint8_t map_data[3] = {0, 1, 2};

/*
 * Get ddc mask of specific hdmi port.
 */
static uint8_t tdHdmiGetDdcByPort(uint8_t port)
{
	return hdmi_map_table[port].ddc_mask;
}

/*
 * Get hdmi port by given displaymips port
 */
uint8_t tdHdmiGetPortByDisplay(uint8_t disp_port)
{
	uint8_t port;
	for (port = HDMI1_PORT; port < HDMI_PORT_NUM_MAX; port++) {
		if (hdmi_map_table[port].display == disp_port)
			return port;
	}

	return HDMI_PORT_INVALID;
}

/*
 * tdHdmiGetHpdByPort
 */
static uint8_t tdHdmiGetHpdByPort(uint8_t port)
{
	return hdmi_map_table[port].hotplug_pin;
}

/*
 * tdHdmiGetMaskByPort
 */
static uint8_t tdHdmiGetMaskByPort(uint8_t port)
{
	return hdmi_map_table[port].port_mask;
}

/*
 * tdHdmiGetPaByPort
 */
static uint8_t tdHdmiGetPaByPort(uint8_t port)
{
	return hdmi_map_table[port].pa;
}

/*
 * tdHdmiGetMappedPort
 */
// static uint8_t tdHdmiGetMappedPort(uint8_t port) { return hdmi_map_table[port].hotplug_pin; }

/*
 * Convert hdmi port mask bits to ddc mask bits.
 */
static uint8_t convertDdcMask(uint8_t port_masks)
{
	uint8_t port, ddc_masks;

	ddc_masks = 0;
	for (port = HDMI1_PORT; port < HDMI_PORT_NUM_MAX; port++) {
		if (port_masks & (1 << port)) {
			ddc_masks |= tdHdmiGetDdcByPort(port);
		}
	}

	return ddc_masks;
}

/*
 * tdHdmiGetPortByMask
 */
static uint8_t tdHdmiGetPortByMask(uint8_t mask)
{
	switch (mask) {
	case HDMI1_PORT_MASK:
		return HDMI1_PORT;
	case HDMI2_PORT_MASK:
		return HDMI2_PORT;
	case HDMI3_PORT_MASK:
		return HDMI3_PORT;
	}

	return HDMI_PORT_INVALID;
}

void tdHdmiEdidInit(void)
{
	//uint8_t i;
	uint8_t I2C_ADDR_0 = 0xA0;
	uint8_t I2C_ADDR_1 = 0xA0;
	uint8_t I2C_ADDR_2 = 0xA0;
	uint32_t value = 0;
	uint32_t reg_val;

	/*  open EDID clock */
	writel(0x82000000, HDMIRX_ASSIST_CLOCK);
	writel(0x00010000, HDMIRX_ASSIST_BUS_GATING);
	writel(0x00000000, HDMIRX_ASSIST_BUS_GATING);
	writel(0x00010000, HDMIRX_ASSIST_BUS_GATING);

	tdHdmiPinInit();

	/* set I2C_CTRL_DEBUG_REG(0x07091b08) to 0xC0C0C0C0 */
	writel(0xc0c0c0c0, I2C_CTRL_DEBUG_REG);

	/* set I2C_CTRL_REG(0x07091b04) bit[31:24] to 0 */
	reg_val = readl(I2C_CTRL_REG);
	reg_val &= ~(0xff << 24);
	writel(reg_val, I2C_CTRL_REG);

	tdHdmiMapInit();

	g_Data.bEdidDataReady = _FALSE_;  // EDID data buffer is empty after AC power on, must get data from host cpu.
	g_Data.bEdidRamReady = _FALSE_;   // EDID RAM is empty after AC power on, must be filled with EDID data
	g_Data.ucEdidHdmi20Setting = 0x0;  // all ports use HDMI1.4 EDID by default.
	tdHdmiEdidEnable(HDMI_ALL_PORT_MASK, _TRUE_);

	value = (I2C_ADDR_0 | (I2C_ADDR_1 << 8) | (I2C_ADDR_2 << 16));
	writel(value, I2C_ADDR_REG);
	g_Data.bEdidDataReady = _TRUE_;  // EDID data is downloaded from ARM.
	tdHdmiEdidUpdate(HDMI_ALL_PORT_MASK);
}

void tdHdmiPinInit(void)
{
	pin_set_multi_sel(PIN_GRP_PL, 10, 2);
	pin_set_multi_sel(PIN_GRP_PL, 11, 2);
	pin_set_multi_sel(PIN_GRP_PL, 12, 2);
	pin_set_multi_sel(PIN_GRP_PL, 13, 2);
	pin_set_multi_sel(PIN_GRP_PL, 14, 2);
	pin_set_multi_sel(PIN_GRP_PL, 15, 2);

	pin_set_multi_sel(PIN_GRP_PL, 16, 2);
	pin_set_multi_sel(PIN_GRP_PL, 17, 2);
	pin_set_multi_sel(PIN_GRP_PL, 18, 2);
	pin_set_multi_sel(PIN_GRP_PL, 19, 2);
	pin_set_multi_sel(PIN_GRP_PL, 20, 2);
	pin_set_multi_sel(PIN_GRP_PL, 21, 2);
	pin_set_multi_sel(PIN_GRP_PL, 22, 2);
	pin_set_multi_sel(PIN_GRP_PL, 23, 2);
}

/*****************************************************************************
*  Name        : tdHdmiEdidEnable
*  Description :
		- 1: SRAM can only br read by I2C host.
*****************************************************************************/
void tdHdmiEdidEnable(uint8_t port_masks, uint8_t bEnable)
{
	u32 HDMIEDID_CTL;
	uint8_t ddc_ports = convertDdcMask(port_masks);

	HDMIEDID_CTL = readl(I2C_CTRL_REG);

	if (bEnable && g_Data.bEdidDataReady) {
		// HDMIEDID_CTL |= HDMI3EDID_Enable | HDMI2EDID_Enable |HDMI1EDID_Enable;
		if (ddc_ports & DDC0_PORT_MASK) {
			HDMIEDID_CTL |= HDMI1EDID_Enable;
		}
		if (ddc_ports & DDC0_PORT_MASK) {
			HDMIEDID_CTL |= HDMI2EDID_Enable;
		}
		if (ddc_ports & DDC0_PORT_MASK) {
			HDMIEDID_CTL |= HDMI3EDID_Enable;
		}

		LOG("enable EDID\n");
	} else {
		// HDMIEDID_CTL &= ~(HDMI3EDID_Enable | HDMI2EDID_Enable |HDMI1EDID_Enable);
		if (ddc_ports & HDMI1_PORT_MASK) {
			HDMIEDID_CTL &= ~HDMI1EDID_Enable;
		}
		if (ddc_ports & DDC0_PORT_MASK) {
			HDMIEDID_CTL &= ~HDMI2EDID_Enable;
		}
		if (ddc_ports & DDC0_PORT_MASK) {
			HDMIEDID_CTL &= ~HDMI3EDID_Enable;
		}
	}
	writel(HDMIEDID_CTL, I2C_CTRL_REG);
}

/*****************************************************************************
*  Name        : setSingleEdidTable
*  Description :

*****************************************************************************/
static void setSingleEdidTable(uint8_t hdmi_port_mask, uint8_t *pEdidData)
{
#define DATA_BLOCK_START_OFFSET (128 + 4)
#define DATA_BLOCK_END_OFFSET (128 + *(pEdidData + 130))
#define RESERVED_BLOCK_LENGTH RESERVED_AUDIO_BLOCK_SIZE

#define BLOCK_TYPE_MASK 0xE0    // Bit7:5
#define BLOCK_LENGTH_MASK 0x1F  // Bit4:0

#define BLOCK_TYPE_AUDIO 0x20
#define BLOCK_TYPE_VIDEO 0x40
#define BLOCK_TYPE_VENDOR 0x60
#define BLOCK_TYPE_SPEAKER 0x80
#define BLOCK_TYPE_VESA_DTC 0xA0
#define BLOCK_TYPE_EXTENDED 0xE0

	uint16_t i;
	uint8_t cecPhyAddrOffset = 0xFF, ddc_port, hdmi_port;
	uint8_t checksum;
	uint32_t EdidRegAddress, EDID_value;

	hdmi_port = tdHdmiGetPortByMask(hdmi_port_mask);
	ddc_port = tdHdmiGetDdcByPort(hdmi_port);
	/* check which port will be updated */
	if (ddc_port == DDC0_PORT_MASK) {
		EdidRegAddress = EDID_HDMI1_ADDRESS;
	} else if (ddc_port == DDC1_PORT_MASK) {
		EdidRegAddress = EDID_HDMI2_ADDRESS;
	} else if (ddc_port == DDC2_PORT_MASK) {
		EdidRegAddress = EDID_HDMI3_ADDRESS;
	} else {
		ERR("ERROR port_mask=0x%x\n", hdmi_port);
		return;  // didn't find a correct EDID target, do nothing.
	}

	for (i = DATA_BLOCK_START_OFFSET; i < DATA_BLOCK_END_OFFSET;) {
		uint8_t blockType = *(pEdidData + i) & BLOCK_TYPE_MASK;
		uint8_t blockLength = *(pEdidData + i) & BLOCK_LENGTH_MASK;
		// LOG("BlockType 0x%x  ", blockType);
		// LOG("Length 0x%x\n", blockLength);

		switch (blockType) {
		case BLOCK_TYPE_VIDEO:
		case BLOCK_TYPE_SPEAKER:
		case BLOCK_TYPE_VESA_DTC:
		case BLOCK_TYPE_EXTENDED:
			i += blockLength + 1;  // jump to next block
			break;

		case BLOCK_TYPE_VENDOR:
			/* Compare 24bit IEEE Registration Identifier (0x000C03) */
			if ((*(pEdidData + i + 1) == RegistrationID_0) && (*(pEdidData + i + 2) == RegistrationID_1) &&
				(*(pEdidData + i + 3) == RegistrationID_2)) {
				cecPhyAddrOffset = i + 4;  // found CEC physical address offset
			}
			i += blockLength + 1;  // jump to next block
			break;

		case BLOCK_TYPE_AUDIO:
			i += blockLength + 1;  // jump to next block
			break;

		default:
			ERR("WRONG EDID BLOCK TYPE 0x%x\n", blockType);
			i = 256;  // exit
			break;
		}
	}

	/* Load 0~127 bytes: Basic block */
	/* Load 128~255 bytes: EIA/CEA-861 extension block */
	for (i = 0; i < 255; i++) {
		*(pEdidBuffer + i) = *(pEdidData + i);
	}

	/* Update CEC physical address */
	//*(pEdidBuffer + cecPhyAddrOffset) = convertHdmiPortToPA(hdmi_port);
	*(pEdidBuffer + cecPhyAddrOffset) = tdHdmiGetPaByPort(hdmi_port);

	// calculate checksum for block1
	checksum = 0;
	for (i = 128; i < 255; i++) {
		checksum -= *(pEdidBuffer + i);
	}
	*(pEdidBuffer + 255) = checksum;

	for (i = 0; i < EDID_Array_LEN; i++) {
		EDID_value =
			pEdidBuffer[(i * 4)] | (pEdidBuffer[(i * 4 + 1)] << 8) | (pEdidBuffer[(i * 4 + 2)] << 16) | (pEdidBuffer[(i * 4 + 3)] << 24);
		writel(EDID_value, EdidRegAddress + (i * 4));
	}
}

/*****************************************************************************
*  Name        : loadHostEdidTable
*  Description :
		- load HDMI1.4/2.0 EDID table which is downloaded from ARM.
*****************************************************************************/
static void loadHostEdidTable(uint8_t port_mask)
{
	uint8_t port;

	if (g_Data.bEdidDataReady) {
		// Load HDMI1/2/3 EDID
		for (port = 0; port < HDMI_PORT_NUM_MAX; port++) {
			if (port_mask & tdHdmiGetMaskByPort(port)) {
				if (g_Data.ucEdidHdmi20Setting & tdHdmiGetMaskByPort(port)) {
					setSingleEdidTable(tdHdmiGetMaskByPort(port), HOST_ASSIGNED_EDID_HDMI20);  // hdmi2.0 edid
				} else {
					setSingleEdidTable(tdHdmiGetMaskByPort(port), HOST_ASSIGNED_EDID_HDMI14);  // hdmi1.4 edid
				}
			}
		}
	}
}

/*****************************************************************************
*  Name        : tdHdmiEdidUpdate
*  Description :
*  Params      :
*  Returns     :
*****************************************************************************/
void tdHdmiEdidUpdate(uint8_t hdmi_port_masks)
{
	if (g_Data.bEdidDataReady) {
		// host edid is ready
		/* Start reset hotplug pins before update EDID table */
		//SysGPIOResetHotplug(hdmi_port_masks, _TRUE_, TIME_OUT_RELOAD_EDID);
		//LOG("Reset HPD, update edid, mask=0x%x\n", hdmi_port_masks);
		tdHdmiEdidEnable(hdmi_port_masks, _FALSE_);
		loadHostEdidTable(hdmi_port_masks);
		LOG("edid updated\n");
		tdHdmiEdidEnable(hdmi_port_masks, _TRUE_);
		g_Data.bEdidRamReady = _TRUE_;
	}
}

/*****************************************************************************
*  Name        : tdHdmiEdidPrint
*  Description : This function will print EDID table on UART.
*  Params      :
*  Returns     :
******************************************************************************/
void tdHdmiEdidPrint(void)
{
	uint8_t i;
	//LOG("\nPanelID: 0x%x ", SysGetPanelID());

	tdHdmiEdidEnable(HDMI_ALL_PORT_MASK, _FALSE_);
	LOG("\nEDID HDMI 1:\n");
	for (i = 0; i < EDID_Array_LEN; i++) {
		LOG("0x%x\n", readl(EDID_HDMI1_ADDRESS + (i * 4)));
	}
	LOG("\nEDID HDMI 2:\n");
	for (i = 0; i < EDID_Array_LEN; i++) {
		LOG("0x%x\n", readl(EDID_HDMI2_ADDRESS + (i * 4)));
	}
	LOG("\nEDID HDMI 3:\n");
	for (i = 0; i < EDID_Array_LEN; i++) {
		LOG("0x%x\n", readl(EDID_HDMI3_ADDRESS + (i * 4)));
	}

	tdHdmiEdidEnable(HDMI_ALL_PORT_MASK, _TRUE_);
	LOG("\n");
}

void resetHdmiEdid(void)
{
	uint32_t reg_val = 0;
	reg_val = readl(HDMIRX_ASSIST_CTR_REG);
	reg_val &= ~(0x1 << 3);
	writel(reg_val, HDMIRX_ASSIST_CTR_REG);

	reg_val |= (0x1 << 3);
	writel(reg_val, HDMIRX_ASSIST_CTR_REG);


	tdHdmiEdidUpdate(HDMI_ALL_PORT_MASK);
}

/*****************************************************************************
*  Name        : vsHotplugHandler
*  Description : This function will pull Hotplug.
*  Params      : mapped HPD number
*  Returns     :
******************************************************************************/
void awHotplugHandler(uint8_t Type, uint8_t port)
{
	if (port < HDMI_PORT_NUM_MAX) {
		switch (Type) {
		case CMD_HOTPLUG_PULLUP:
			/* pull up hotplug pin by timer, to avoid conflict.
			* the delay is about 10ms which can be ignored. */
			hotplug_pullup_timer[port]++;
			LOG("hpd %d UP\n", port);
			break;

		case CMD_HOTPLUG_PULLDOWN:
			SysGPIOSetHotplug(port, _LO_);  // pull-down hotplug pin immediately
			LOG("hpd %d DOWN\n", port);
			break;

		case CMD_HOTPLUG_RESET:
			SysGPIOSetHotplug(port, _LO_);                    // pull-down hotplug pin immediately
			hotplug_pullup_timer[port] = TIME_OUT_RESET_HPD;  // pull-up hotplug pin after timeout
			LOG("hpd %d RESET\n", port);
			break;

		default:
			ERR("Unknown Hotplug Type %d\n", Type);
			break;
		}
	} else {
		ERR("Unknown HDMI port %x: ", port);
	}
}

/*****************************************************************************
*  Name        : vsProcessHDMICommand
*  Description :
*  Params      :
*  Returns     :
*****************************************************************************/
uint8_t awProcessHDMICommand(uint8_t Type, uint8_t *rpPacket)
{
	uint8_t bProcessed = _FALSE_;
	// uint8_t CPUS_MsGBox=0x01;

	uint8_t i, j, k;
	u32 EDID_value;
	// uint8_t port_mask;

	switch (Type) {
	case HDMI_CMD_UpdateEDID: {
		/* an EDID table is 256 bytes length, every packet we update 64 bytes of it,
			so it needs 4 packets to update a complete table.
			Params     : Buffer[0] = packet number
											0~3: HDMI 1.4
											4~7: HDMI 2.0
					pdata[1] ~ pdata[65]: EDID data
		*/
		uint8_t last_packet_received = _FALSE_;
		uint8_t *pEDID;

		j = rpPacket[0];
		if (j == LAST_PACK_TABLE2) {
			last_packet_received = _TRUE_;
		}

		if (j <= LAST_PACK_TABLE1) {
			pEDID = HOST_ASSIGNED_EDID_HDMI14;  // to update HDMI1 1.4 EDID
		} else if (j <= LAST_PACK_TABLE2) {
			pEDID = HOST_ASSIGNED_EDID_HDMI20;  // to update HDMI1 2.0 EDID
		} else {
			ERR("ERROR: Invalid EDID block ");
			return bProcessed;
		}

		j = j % EDID_PACK_NUM;
		if (j < EDID_PACK_NUM) {
			/* Update 64 bytes for current packet */
			for (i = 0; i < EDID_DATA_PER_PACK; i++) {
				*(pEDID + i + j * EDID_DATA_PER_PACK) = rpPacket[i + 1];
			}
			g_Data.bEdidDataReady = _FALSE_;  // update not finished yet.
		}

		if (last_packet_received) { // the last packet.
			g_Data.bEdidDataReady = _TRUE_;  // EDID data is downloaded from ARM.

			/* Update EDID */
			tdHdmiEdidUpdate(HDMI_ALL_PORT_MASK);
		}
		bProcessed = _TRUE_;
	} break;

	case HDMI_CMD_EDIDStatus:
		LOG("Request EDID Status\n");
		memset(&PacketSend, 0, sizeof(PACKET_A));
		// PacketSend.Packet.Msg.STB = PACKET_START_BYTE;
		PacketSend.Packet.Msg.Type = MCU_CMD_ReportInfo;
		PacketSend.Packet.Msg.Len = 1;
		PacketSend.Packet.Msg.SubType = MCU_RPT_EDID_Ready;
		PacketSend.Packet.Msg.Data[0] = g_Data.bEdidDataReady;
		SendPacketToHostA(&PacketSend);
		bProcessed = _TRUE_;
		break;

	case HDMI_CMD_EDIDVersion:
		LOG("Host Set EDID Version.  0x%x\n", rpPacket[0]);
		if (g_Data.ucEdidHdmi20Setting != rpPacket[0]) {
			/* Bit3: HDMI4 use HDMI2.0
				Bit2: HDMI3 use HDMI2.0
				Bit1: HDMI2 use HDMI2.0
				Bit0: HDMI1 use HDMI2.0
			*/
			uint8_t mask = g_Data.ucEdidHdmi20Setting ^ rpPacket[0];
			g_Data.ucEdidHdmi20Setting = rpPacket[0] & 0x0F;
			LOG("mask.  0x%x\n", mask);
			tdHdmiEdidUpdate(mask & 0x0F);
		}
		bProcessed = _TRUE_;
		break;

	case HDMI_CMD_REQ_EDID_data: {
		uint8_t i, j = 0;
		// uint8_t  EDIDindex =0;
		int EdidRegAddress = EDID_HDMI1_ADDRESS;  // pointer to HDMI1_EDIDData, (EDID_BASE_ADDRESS + 0x0400)

		/*rpPacket[0] is table index:
		* 0: HDMI1; 1: HDMI2; 2: HDMI3; 	 */
		LOG("Read EDID. MSG= 0x%x,  0x%x\n", Type, rpPacket[0]);

		if (rpPacket[0] >= HDMI_PORT_NUM_MAX) {
			ERR("Wrong HDMI Port Number\n");
			return bProcessed;
		}

		EdidRegAddress += 0x100 * rpPacket[0];

		memset(&PacketSend, 0, sizeof(PACKET_D));
		// PacketSend.Packet.Msg.STB = PACKET_START_BYTE;
		PacketSend.Packet.Msg.Type = MCU_CMD_ReportInfo;
		PacketSend.Packet.Msg.SubType = MCU_RPT_HDMI_EDID_DATA;
		PacketSend.Packet.Msg.Len = EDID_DATA_PER_PACK + 1;

		tdHdmiEdidEnable(HDMI_ALL_PORT_MASK, _FALSE_);

		for (j = 0; j < EDID_PACK_NUM; j++) {
			PacketSend.Packet.Msg.Data[0] = j;  // packet number

			for (i = 0; i < EDID_DWORD_PER_PACK; i++) {
				EDID_value = readl(EdidRegAddress + (i * 4) + j * EDID_DATA_PER_PACK);

				k = 0;
				while (k < 4) {
					PacketSend.Packet.Msg.Data[1 + (i * 4) + k] = (EDID_value >> (k * 8)) & 0xFF;
					k++;
				}
			}
			SendPacketToHostD(&PacketSend);
		}

		tdHdmiEdidEnable(HDMI_ALL_PORT_MASK, _TRUE_);
	}
		bProcessed = _TRUE_;
		break;

	case HDMI_CMD_PrintEDIDTable:
		LOG("Output EDID\n", Type);
		tdHdmiEdidPrint();
		bProcessed = _TRUE_;
		break;

	case HDMI_CMD_HDMIPortNumber:
		LOG("Host Read HDMI port Number\n");
		memset(&PacketSend, 0, sizeof(PACKET_A));
		// PacketSend.Packet.Msg.STB = PACKET_START_BYTE;
		PacketSend.Packet.Msg.Type = MCU_CMD_ReportInfo;
		PacketSend.Packet.Msg.SubType = MCU_RPT_HDMI_PORT_NUMBER;
		PacketSend.Packet.Msg.Len = 1;
		PacketSend.Packet.Msg.Data[0] = g_Data.bHDMIPortNumber;
		SendPacketToHostA(&PacketSend);
		bProcessed = _TRUE_;
		break;

	case HDMI_CMD_HOTPLUG: {
		LOG("Host pull Hotplug port %d, value = %d\n", rpPacket[0], rpPacket[1]);
		awHotplugHandler(rpPacket[1], rpPacket[0]);
		bProcessed = _TRUE_;
	} break;

	case HDMI_CMD_ResetEDIDModule: {
		LOG("Host Reset EDID Module\n");
		time_mdelay(200);
		resetHdmiEdid();
		bProcessed = _TRUE_;
	} break;

	default:
		bProcessed = _FALSE_;
		break;
	}
	return bProcessed;
}

/*****************************************************************************
*  Name        : SysGPIOSetHotplug
*  Description :
*  Params      :
*  Returns     :
*****************************************************************************/
void SysGPIOSetHotplug(uint8_t port, uint8_t bOn)
{
	uint32_t EDID_value;
	uint8_t HPD_port = tdHdmiGetHpdByPort(port);

	if (!g_Data.bEdidRamReady) {
		bOn = _FALSE_;  // EDID is not ready, keep HPD pin low to avoid HDMI device load wrong EDID data.
		LOG("EDID-HPD %d LOW\n", HPD_port - HDMI1_PORT);
	}
	EDID_value = readl(HDMIRX_HPD_SET_REG);

	//Invert HPD output
	switch (HPD_port) {
	case HDMI1_PORT:
		if (!bOn) {
			EDID_value |= HDMI1_PORT_MASK;
		} else {
			EDID_value &= ~HDMI1_PORT_MASK;
		}
		LOG("Chip HPD RX0 %d\n", bOn);
		break;

	case HDMI2_PORT:
		if (!bOn) {
			EDID_value |= HDMI2_PORT_MASK;
		} else {
			EDID_value &= ~HDMI2_PORT_MASK;
		}
		LOG("Chip HPD RX1 %d\n", bOn);
		break;

	case HDMI3_PORT:
		if (!bOn) {
			EDID_value |= HDMI3_PORT_MASK;
		} else {
			EDID_value &= ~HDMI3_PORT_MASK;
		}
		LOG("Chip HPD RX2 %d\n", bOn);
		break;

	default:
		LOG("Wrong HDMI Port\n");
		break;
	}
	writel(EDID_value, HDMIRX_HPD_SET_REG);
}

/*****************************************************************************
*  Name        : SysGPIOSetHotplugMasked
*  Description :
*  Params      :
*  Returns     :
*****************************************************************************/
void SysGPIOSetHotplugMasked(uint8_t port_mask, uint8_t bOn)
{
	/*  mask:
		HDMI1_PORT_MASK	_BIT0_
		HDMI2_PORT_MASK	_BIT1_
		HDMI3_PORT_MASK	_BIT2_
	*/

	uint8_t port;
	LOG("port_mask=%x  bOn = %d\n", port_mask, bOn);

	for (port = HDMI1_PORT; port < HDMI_PORT_NUM_MAX; port++) {
		if ((port_mask & (1 << port)) != 0) {
			SysGPIOSetHotplug(port, bOn);
		}
	}
}

/*****************************************************************************
*  Name        : SysGPIOResetHotplug
*  Description :
*  Params      :
*  Returns     :
*****************************************************************************/
void SysGPIOResetHotplug(uint8_t port_mask, uint8_t bYesNo, uint8_t time)
{
	if (bYesNo) {
		uint8_t port;
		SysGPIOSetHotplugMasked(port_mask, _LO_);

		for (port = 0; port < HDMI_PORT_NUM_MAX; port++) {
			if ((port_mask & (1 << port)) != 0) {  // release hotplug pins after timeout
				hotplug_pullup_timer[hdmi_map_table[port].hotplug_pin - HDMI1_PORT] = time;
				LOG("SysResetHotplug-%d ", (hdmi_map_table[port].hotplug_pin - HDMI1_PORT));
				LOG("%dms\n", (uint16_t)(time - TIMER_EXPIRED) * 10);
			}
		}
	} else {
		SysGPIOSetHotplugMasked(port_mask, _HI_);
	}
}
