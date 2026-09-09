/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                              interrupt  module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : cec.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-3
* Descript: hdmi assist tx cec internal header.
* Update  : date                auther      ver     notes
*           2025-11-5 15:44:00  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef _CEC_H_
#define _CEC_H_

#include "include.h"

#define CEC_MSG_MAX_SIZE	(16)

int hwcec_is_receive(void);

u8 hwcec_msg_receive(u8 *buf, u8 buf_size);

u8 hwcec_msg_send(u8 *buf, u8 len);

void hwcec_reg_dump(void);

void hwcec_init(void);

#define CECTOP_BASE_ADDR			(0x072C1000)
/*******************************************************************************
 * @desc: assist cec top register bank
 ******************************************************************************/
#define CECTOP_CONTROL               (0x0000)
#define CECTOP_RSTN_MASK                      BIT(9)
#define CECTOP_EN_MASK                        BIT(8)
#define CECTOP_SW_MASK                        BIT(4)
#define CECTOP_SW_RSTN_MASK                   BIT(2)

#define CECTOP_INT_STS               (0x0004)
#define CECTOP_RISING_EDGE_INT_STS_MASK       BIT(1)
#define CECTOP_FALLING_EDGE_INT_STS_MASK      BIT(0)

#define CECTOP_INT_EN                (0x0008)
#define CECTOP_RISING_EDGE_INT_EN_MASK        BIT(1)
#define CECTOP_FALLING_EDGE_INT_EN_MASK       BIT(0)

#define CECTOP_DEBOUNCE_CTRL         (0x000C)
#define CECTOP_DEBOUNCE_BYPASS_EN_MASK        BIT(0)

#define CECTOP_RISING_DEBOUNCE       (0x0010)
#define CECTOP_RISING_DEBOUNCE_CNT_MASK       (0xFFFFFFFF)

#define CECTOP_FALLING_DEBOUNCE      (0x0014)
#define CECTOP_FALLING_DEBOUNCE_CNT_MASK      (0xFFFFFFFF)

#define CECTOP_CONTROL2              (0x0018)
#define CECTOP_INPUT_CONTROL_MASK             (BIT(3) | BIT(2))
#define CECTOP_OUTPUT_SET_BIT_MASK            BIT(1)
#define CECTOP_INPUT_STATUS_MASK              BIT(0)

/*******************************************************************************
 * @desc: assist cec control register bank
 ******************************************************************************/
#define HWCEC_OFFSET(x)      (x + 0x100)

#define HWCEC_INT_STS                HWCEC_OFFSET(0x0000)
#define HWCEC_RCV_ERR_INT_STS_MASK            BIT(6)
#define HWCEC_SEND_ERR_INT_STS_MASK           BIT(5)
#define HWCEC_ARBLST_INT_STS_MASK             BIT(4)
#define HWCEC_NACK_INT_STS_MASK               BIT(3)
#define HWCEC_RCV_DONE_INT_STS_MASK           BIT(2)
#define HWCEC_SEND_DONE_INT_STS_MASK          BIT(1)
#define HWCEC_WAKEUP_INT_STS_MASK             BIT(0)

#define HWCEC_INT_EN                 HWCEC_OFFSET(0x0004)
#define HWCEC_RCV_ERR_INT_EN_MASK             BIT(6)
#define HWCEC_SEND_ERR_INT_EN_MASK            BIT(5)
#define HWCEC_ARBLST_INT_EN_MASK              BIT(4)
#define HWCEC_NACK_INT_EN_MASK                BIT(3)
#define HWCEC_RCV_DONE_INT_EN_MASK            BIT(2)
#define HWCEC_SEND_DONE_INT_EN_MASK           BIT(1)
#define HWCEC_WAKEUP_INT_EN_MASK              BIT(0)

#define HWCEC_CTRL                   HWCEC_OFFSET(0x0010)
#define HWCEC_SLOW_DRIVE_DISABLED_MASK        BIT(5)
#define HWCEC_STANDBY_MASK                    BIT(4)
#define HWCEC_BC_NACK_MASK                    BIT(3)
#define HWCEC_FRAMETYPE_MASK                  (BIT(2) | BIT(1))
#define HWCEC_SEND_MASK                       BIT(0)

#define HWCEC_INT_MASK               HWCEC_OFFSET(0x0014)
#define HWCEC_WAKEUP_INT_MASK                 BIT(6)
#define HWCEC_RCV_ERR_INT_MASK                BIT(5)
#define HWCEC_SEND_ERR_INT_MASK               BIT(4)
#define HWCEC_ARB_LST_INT_MASK                BIT(3)
#define HWCEC_NACK_INT_MASK                   BIT(2)
#define HWCEC_RCV_DONE_INT_MASK               BIT(1)
#define HWCEC_SEND_DONE_INT_MASK              BIT(0)

#define HWCEC_ADDR_L                 HWCEC_OFFSET(0x0018)
#define HWCEC_LADDR_MASK                      (0xFF)

#define HWCEC_ADDR_H                 HWCEC_OFFSET(0x001C)
#define HWCEC_HADDR_MASK                      (0xFF)

#define HWCEC_SEND_LENGTH            HWCEC_OFFSET(0x0020)
#define HWCEC_SEND_LEN_MASK                   (0x1F)

#define HWCEC_RECEIVE_LENGTH         HWCEC_OFFSET(0x0024)
#define HWCEC_RECEIVE_LEN_MASK                (0x1F)

#define HWCEC_LOCK                   HWCEC_OFFSET(0x0028)
#define HWCEC_LOCK_MASK                       BIT(0)

#define HWCEC_WAKEUP_CONTROL         HWCEC_OFFSET(0x002C)
#define HWCEC_WAKEUP_OPCODE_86_MASK           BIT(7)
#define HWCEC_WAKEUP_CPCODE_82_MASK           BIT(6)
#define HWCEC_WAKEUP_OPCODE_70_MASK           BIT(5)
#define HWCEC_WAKEUP_OPCODE_44_MASK           BIT(4)
#define HWCEC_WAKEUP_OPCODE_42_MASK           BIT(3)
#define HWCEC_WAKEUP_OPCODE_41_MASK           BIT(2)
#define HWCEC_WAKEUP_OPCODE_0D_MASK           BIT(1)
#define HWCEC_WAKEUP_OPCODE_04_MASK           BIT(0)

#define HWCEC_SEND_DATA              HWCEC_OFFSET(0x0030)
#define HWCEC_SEND_DATA_INDEX(x)     (HWCEC_SEND_DATA + (x * 0x0004))
#define HWCEC_SEND_DATA_MASK                  (0xFF)

#define HWCEC_RECEIVE_DATA           HWCEC_OFFSET(0x0070)
#define HWCEC_RECEIVE_DATA_INDEX(x)  (HWCEC_RECEIVE_DATA + (x * 0x0004))
#define HWCEC_RECEIVE_DATA_MASK               (0xFF)

#endif