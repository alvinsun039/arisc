/****************************************************************************
 * hdmirx/awCECLow.h
 *
 * Copyright (C) 2022-2024 AllWinnertech Ltd.
 * Author: huangwenhao <huangwenhao@allwinnertech.com>
 ***************************************************************************/

/**
 * @defgroup CECsubMCU CEC for subMCU
 * @{
 */

/**
 * @defgroup CECsubMCULow Low level CEC API
 * @{
 * This level APIs are implemented for physical layer and CEC bus arbitrator.\n
 * Compliance test is conformed by this level package.
 * APIs in callback group should be realized at high level.
 */

#ifndef __TCECLOW_H_
#define __TCECLOW_H_


/*CEC logical address definition */
typedef enum tagTCECLogicalAddr {
	_TCECLogicalAddr_TV_,
	_TCECLogicalAddr_RD1_,
	_TCECLogicalAddr_RD2_,
	_TCECLogicalAddr_TUNER1_,
	_TCECLogicalAddr_PD1_,
	_TCECLogicalAddr_AS_,
	_TCECLogicalAddr_TUNER2_,
	_TCECLogicalAddr_TUNER3_,
	_TCECLogicalAddr_PD2_,
	_TCECLogicalAddr_RD3_,
	_TCECLogicalAddr_TUNER4_,
	_TCECLogicalAddr_PD3_,
	_TCECLogicalAddr_Reserved1_,
	_TCECLogicalAddr_Reserved2_,
	_TCECLogicalAddr_FREE_,
	_TCECLogicalAddr_Unregisterd_
} TCECLogicalAddr;

/** CEC message structure */
#define MAX_OPERAND_NUM 15
typedef struct tagTCECMessage {
	union {
		uint8_t Buffer[MAX_OPERAND_NUM + 2];  //< message memory
		struct {
			uint8_t ucHeader;                     //< 4bit Initiator logical address + 4bit Destination logical address
			uint8_t ucOpcode;                     //< message opcode
			uint8_t ucOperands[MAX_OPERAND_NUM];  //< the maximun operand is 14 according to SPG, one more byte for feeding back txStatus
		} Msg;                                 //< message struct
	} Content;                                 //< message content
	uint8_t ucOperandNum;                         //<  the number of ucHeader + ucOpcode + ucOperands
	int index;
} TCECMessage;

typedef struct tagTCECSendBackACK {
	union {
		uint8_t Buffer[3];
		struct {
			uint8_t ucHeader;
			uint8_t ucOpcode;
			uint8_t ucACK;
		} Msg;
	} Content;
} TCECSendBackACK;


void aw_cec_low_assist_ctrl(void);

/**
 * Read interrupts
 * @param dev:  address and device information
 * @param mask: interrupt mask to read
 * @return INT content
 */
int aw_cec_low_interrupt_get_state(void);

/**
 * Clear interrupts state
 * @param dev:  address and device information
 * @param mask: interrupt mask to clear
 * @return INT content
 */
int aw_cec_low_interrupt_clear_state(unsigned state);

/**
 * Set cec logical address
 * @param dev:  address and device information
 * @param addr: logical address
 * @return INT content
 */
int aw_cec_low_set_logical_addr(void);

/**
 * @desc: cec get config logic address
 * @param dev:  address and device information
 * @return: logic addres
*/
int aw_cec_low_get_logical_addr(void);

/**
 * Write transmission buffer
 * @param dev:  address and device information
 * @param buf:  data to transmit
 * @param size: data length [byte]
 * @return error code or bytes configured
 */
int aw_cec_low_send_frame(unsigned char *buf, unsigned size, unsigned frame_type);

/**
 * Read reception buffer
 * @param dev:  address and device information
 * @param buf:  buffer to hold receive data
 * @return size: reception data length [byte]
 */
int aw_cec_low_receive_frame(unsigned char *buf, unsigned *size);

/**
 * Open a CEC controller
 * @warning Execute before start using a CEC controller
 * @param dev:  address and device information
 * @return error code
 */
void aw_cec_low_enable(void);

/**
 * Close a CEC controller
 * @warning Execute before stop using a CEC controller
 * @param dev:    address and device information
 * @return error code
 */
int aw_cec_low_disable(void);

#endif
/**
 * @}@}
 */
