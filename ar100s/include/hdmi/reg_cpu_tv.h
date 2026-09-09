/****************************************************************************
 * hdmirx/reg_cpu_tv.h
 *
 * Copyright (C) 2022-2024 AllWinnertech Ltd.
 * Author: huangwenhao <huangwenhao@allwinnertech.com>
 ***************************************************************************/

#ifndef _REG_CPU_TV303_H
#define _REG_CPU_TV303_H

extern uint32_t t_dwTimerCount;

#define EDID_BASE_ADDRESS (0x072C0800)
#define EDID_HDMI1_ADDRESS (EDID_BASE_ADDRESS + 0x0400)
#define EDID_HDMI2_ADDRESS (EDID_BASE_ADDRESS + 0x0500)
#define EDID_HDMI3_ADDRESS (EDID_BASE_ADDRESS + 0x0600)
#define I2C_CTRL_DEBUG_REG (EDID_BASE_ADDRESS + 0x308)
#define I2C_CTRL_REG (EDID_BASE_ADDRESS + 0x304)
#define I2C_ADDR_REG (EDID_BASE_ADDRESS + 0x300)
#define I2C_REG_1 (0x07010120)
#define I2C_REG_2 (0x07010124)
#define I2C_REG_3 (0x07022004)

#define EDID_TOP_BASE_ADDRESS (0x072C0000)
#define HDMIRX_5V_ON_INT_STATUS_REG (EDID_TOP_BASE_ADDRESS + 0)
#define HDMIRX_5V_ON_INT_EN_REG (EDID_TOP_BASE_ADDRESS + 0x004)
#define HDMIRX_5V_OFF_INT_STATUS_REG (EDID_TOP_BASE_ADDRESS + 0x008)
#define HDMIRX_5V_OFF_INT_EN_REG (EDID_TOP_BASE_ADDRESS + 0x00C)
#define HDMIRX_5V_ON_STATUS_REG (EDID_TOP_BASE_ADDRESS + 0x010)
#define HDMIRX_HPD_SET_REG (EDID_TOP_BASE_ADDRESS + 0x014)
#define HDMIRX_CEC_CTRL_REG (EDID_TOP_BASE_ADDRESS + 0x018)
#define HDMIRX_CEC_INT_STATUS_REG (EDID_TOP_BASE_ADDRESS + 0x01C)
#define HDMIRX_CEC_INT_EN_REG (EDID_TOP_BASE_ADDRESS + 0x020)
#define HDMIRX_5V_DEBOUNCE_CTRL_REG (EDID_TOP_BASE_ADDRESS + 0x024)
#define HDMIRX_5V_Rising_DEBOUNCE_CNT_REG (EDID_TOP_BASE_ADDRESS + 0x028)
#define HDMIRX_5V_Falling_DEBOUNCE_CNT_REG (EDID_TOP_BASE_ADDRESS + 0x02C)
#define HDMIRX_CEC_DEBOUNCE_CTRL_REG (EDID_TOP_BASE_ADDRESS + 0x030)
#define HDMIRX_CEC_Rising_DEBOUNCE_CNT_REG (EDID_TOP_BASE_ADDRESS + 0x034)
#define HDMIRX_CEC_Falling_DEBOUNCE_CNT_REG (EDID_TOP_BASE_ADDRESS + 0x038)
#define HDMIRX_ASSIST_CTR_REG (EDID_TOP_BASE_ADDRESS + 0x040)
#define HDMIRX_5V_DETECT_CTR_REG0 (EDID_TOP_BASE_ADDRESS + 0x044)
#define HDMIRX_5V_DETECT_CTR_REG1 (EDID_TOP_BASE_ADDRESS + 0x048)

#define HDMIRX_HW_CEC_BASE_ADDRESS (0x072C0100)
/* CEC Interrupt Status Register (Functional Operation Interrupts) */
#define HDMIRX_IH_CEC_STAT0  (HDMIRX_HW_CEC_BASE_ADDRESS + 0x0)
#define IH_CEC_STAT0_WAKEUP_MASK  0x00000001 /* CEC Wake-up indication */
#define IH_CEC_STAT0_DONE_MASK  0x00000002 /* CEC Done Indication */
#define IH_CEC_STAT0_EOM_MASK  0x00000004 /* CEC End of Message Indication */
#define IH_CEC_STAT0_NACK_MASK  0x00000008 /* CEC Not Acknowledge indication */
#define IH_CEC_STAT0_ARB_LOST_MASK  0x00000010 /* CEC Arbitration Lost indication */
#define IH_CEC_STAT0_ERROR_INITIATOR_MASK  0x00000020 /* CEC Error Initiator indication */
#define IH_CEC_STAT0_ERROR_FOLLOW_MASK  0x00000040 /* CEC Error Follow indication */

/* CEC Interrupt Enable Control Register */
#define HDMIRX_IH_ENABLE_CEC_STAT0  (HDMIRX_HW_CEC_BASE_ADDRESS + 0x4)
#define IH_ENABLE_CEC_STAT0_WAKEUP_MASK  0x00000001 /* When set to 1, enable ih_cec_stat0[0] */
#define IH_ENABLE_CEC_STAT0_DONE_MASK  0x00000002 /* When set to 1, enable ih_cec_stat0[1] */
#define IH_ENABLE_CEC_STAT0_EOM_MASK  0x00000004 /* When set to 1, enable ih_cec_stat0[2] */
#define IH_ENABLE_CEC_STAT0_NACK_MASK  0x00000008 /* When set to 1, enable ih_cec_stat0[3] */
#define IH_ENABLE_CEC_STAT0_ARB_LOST_MASK  0x00000010 /* When set to 1, enable ih_cec_stat0[4] */
#define IH_ENABLE_CEC_STAT0_ERROR_INITIATOR_MASK  0x00000020 /* When set to 1, enable ih_cec_stat0[5] */
#define IH_ENABLE_CEC_STAT0_ERROR_FOLLOW_MASK  0x00000040 /* When set to 1, enable ih_cec_stat0[6] */

/* CEC Control Register This register handles the main control of the CEC initiator */
#define HDMIRX_CEC_CTRL  (HDMIRX_HW_CEC_BASE_ADDRESS + 0x10)
#define CEC_CTRL_SEND_MASK  0x00000001 /* - 1'b1: Set by software to trigger CEC sending a frame as an initiator */
#define CEC_CTRL_FRAME_TYP_MASK  0x00000006 /* - 2'b00: Signal Free Time = 3-bit periods */
#define CEC_CTRL_FRAME_TYP_RETRY  0x00000000
#define CEC_CTRL_FRAME_TYP_NORMAL  0x00000002
#define CEC_CTRL_FRAME_TYP_IMMED  0x00000004
#define CEC_CTRL_BC_NACK_MASK  0x00000008 /* - 1'b1: Set by software to NACK the received broadcast message */
#define CEC_CTRL_STANDBY_MASK  0x00000010 /* - 1: CEC controller responds with a NACK to all messages and generates a wakeup status for opcode */

/* CEC Interrupt Mask Register This read/write register masks/unmasks the interrupt events */
#define HDMIRX_CEC_MASK  (HDMIRX_HW_CEC_BASE_ADDRESS + 0x14)
#define CEC_MASK_DONE_MASK  0x00000001 /* The current transmission is successful (for initiator only) */
#define CEC_MASK_EOM_MASK  0x00000002 /* EOM is detected so that the received data is ready in the receiver data buffer (for follower only) */
#define CEC_MASK_NACK_MASK  0x00000004 /* A frame is not acknowledged in a directly addressed message */
#define CEC_MASK_ARB_LOST_MASK  0x00000008 /* The initiator losses the CEC line arbitration to a second initiator */
#define CEC_MASK_ERROR_INITIATOR_MASK  0x00000010 /* An error is detected on a CEC line (for initiator only) */
#define CEC_MASK_ERROR_FLOW_MASK  0x00000020 /* An error is notified by a follower */
#define CEC_MASK_WAKEUP_MASK  0x00000040 /* Follower wake-up signal mask */

/* CEC Logical Address Register Low This register indicates the logical address(es) allocated to the CEC device */
#define HDMIRX_CEC_ADDR_L  (HDMIRX_HW_CEC_BASE_ADDRESS + 0x18)
#define CEC_ADDR_L_CEC_ADDR_L_0_MASK  0x00000001 /* Logical address 0 - Device TV */
#define CEC_ADDR_L_CEC_ADDR_L_1_MASK  0x00000002 /* Logical address 1 - Recording Device 1 */
#define CEC_ADDR_L_CEC_ADDR_L_2_MASK  0x00000004 /* Logical address 2 - Recording Device 2 */
#define CEC_ADDR_L_CEC_ADDR_L_3_MASK  0x00000008 /* Logical address 3 - Tuner 1 */
#define CEC_ADDR_L_CEC_ADDR_L_4_MASK  0x00000010 /* Logical address 4 - Playback Device 1 */
#define CEC_ADDR_L_CEC_ADDR_L_5_MASK  0x00000020 /* Logical address 5 - Audio System */
#define CEC_ADDR_L_CEC_ADDR_L_6_MASK  0x00000040 /* Logical address 6 - Tuner 2 */
#define CEC_ADDR_L_CEC_ADDR_L_7_MASK  0x00000080 /* Logical address 7 - Tuner 3 */

/* CEC Logical Address Register High This register indicates the logical address(es) allocated to the CEC device */
#define HDMIRX_CEC_ADDR_H  (HDMIRX_HW_CEC_BASE_ADDRESS + 0x1c)
#define CEC_ADDR_H_CEC_ADDR_H_0_MASK  0x00000001 /* Logical address 8 - Playback Device 2 */
#define CEC_ADDR_H_CEC_ADDR_H_1_MASK  0x00000002 /* Logical address 9 - Playback Device 3 */
#define CEC_ADDR_H_CEC_ADDR_H_2_MASK  0x00000004 /* Logical address 10 - Tuner 4 */
#define CEC_ADDR_H_CEC_ADDR_H_3_MASK  0x00000008 /* Logical address 11 - Playback Device 3 */
#define CEC_ADDR_H_CEC_ADDR_H_4_MASK  0x00000010 /* Logical address 12 - Reserved */
#define CEC_ADDR_H_CEC_ADDR_H_5_MASK  0x00000020 /* Logical address 13 - Reserved */
#define CEC_ADDR_H_CEC_ADDR_H_6_MASK  0x00000040 /* Logical address 14 - Free use */
#define CEC_ADDR_H_CEC_ADDR_H_7_MASK  0x00000080 /* Logical address 15 - Unregistered (as initiator address), Broadcast (as destination address) */

/* CEC TX Frame Size Register This register indicates the size of the frame in bytes (including header and data blocks), which are available in the transmitter data buffer */
#define HDMIRX_CEC_TX_CNT  (HDMIRX_HW_CEC_BASE_ADDRESS + 0x20)
#define CEC_TX_CNT_CEC_TX_CNT_MASK  0x0000001F /* CEC Transmitter Counter register 5'd0: No data needs to be transmitted 5'd1: Frame size is 1 byte */

/* CEC RX Frame Size Register This register indicates the size of the frame in bytes (including header and data blocks), which are available in the receiver data buffer */
#define HDMIRX_CEC_RX_CNT  (HDMIRX_HW_CEC_BASE_ADDRESS + 0x24)
#define CEC_RX_CNT_CEC_RX_CNT_MASK  0x0000001F /* CEC Receiver Counter register: 5'd0: No data received 5'd1: 1-byte data is received */

/* CEC Buffer Lock Register */
#define HDMIRX_CEC_LOCK  (HDMIRX_HW_CEC_BASE_ADDRESS + 0x28)
#define CEC_LOCK_LOCKED_BUFFER_MASK  0x00000001 /* When a frame is received, this bit would be active */

/* CEC Wake-up Control Register After receiving a message in the CEC_RX_DATA1 (OPCODE) registers, the CEC engine verifies the message opcode[7:0] against one of the previously defined values to generate the wake-up status: Wakeupstatus is 1 when: received opcode is 0x04 and opcode0x04en is 1 or received opcode is 0x0D and opcode0x0Den is 1 or received opcode is 0x41 and opcode0x41en is 1 or received opcode is 0x42 and opcode0x42en is 1 or received opcode is 0x44 and opcode0x44en is 1 or received opcode is 0x70 and opcode0x70en is 1 or received opcode is 0x82 and opcode0x82en is 1 or received opcode is 0x86 and opcode0x86en is 1 Wakeupstatus is 0 when none of the previous conditions are true */
#define HDMIRX_CEC_WAKEUPCTRL  (HDMIRX_HW_CEC_BASE_ADDRESS + 0x2c)
#define CEC_WAKEUPCTRL_OPCODE0X04EN_MASK  0x00000001 /* OPCODE 0x04 wake up enable */
#define CEC_WAKEUPCTRL_OPCODE0X0DEN_MASK  0x00000002 /* OPCODE 0x0D wake up enable */
#define CEC_WAKEUPCTRL_OPCODE0X41EN_MASK  0x00000004 /* OPCODE 0x41 wake up enable */
#define CEC_WAKEUPCTRL_OPCODE0X42EN_MASK  0x00000008 /* OPCODE 0x42 wake up enable */
#define CEC_WAKEUPCTRL_OPCODE0X44EN_MASK  0x00000010 /* OPCODE 0x44 wake up enable */
#define CEC_WAKEUPCTRL_OPCODE0X70EN_MASK  0x00000020 /* OPCODE 0x70 wake up enable */
#define CEC_WAKEUPCTRL_OPCODE0X82EN_MASK  0x00000040 /* OPCODE 0x82 wake up enable */
#define CEC_WAKEUPCTRL_OPCODE0X86EN_MASK  0x00000080 /* OPCODE 0x86 wake up enable */

/* CEC TX Data Register Array Address offset: i = 0 to 15 These registers (8 bits each) are the buffers used for storing the data waiting for transmission (including header and data blocks) */
#define HDMIRX_CEC_TX_DATA  (HDMIRX_HW_CEC_BASE_ADDRESS + 0x30)
#define CEC_TX_DATA_SIZE  16

/* CEC RX Data Register Array Address offset: i =0 to 15 These registers (8 bit each) are the buffers used for storing the received data (including header and data blocks) */
#define HDMIRX_CEC_RX_DATA  (HDMIRX_HW_CEC_BASE_ADDRESS + 0x70)
#define CEC_RX_DATA_SIZE  16

#define HDMIRX_ASSIST_BUS_GATING (R_PRCM_REG_BASE + 0x0120)
#define HDMIRX_ASSIST_CLOCK (R_PRCM_REG_BASE + 0x0124)
#define HDMIRX_32K_CLOCK (R_PRCM_REG_BASE + 0x0128)

#define R_INTC_BASE_ADDRESS (0x07021000)
#define R_INTC_PRIO0_REG    (R_INTC_BASE_ADDRESS + 0x80)
#define R_INTC_PRIO1_REG    (R_INTC_BASE_ADDRESS + 0x84)
#define R_INTC_PRIO2_REG    (R_INTC_BASE_ADDRESS + 0x88)
#define R_INTC_PRIO3_REG    (R_INTC_BASE_ADDRESS + 0x8c)
#define R_INTC_PRIO4_REG    (R_INTC_BASE_ADDRESS + 0x90)
#define R_INTC_PRIO5_REG    (R_INTC_BASE_ADDRESS + 0x94)

#define RTC_BASE_ADDRESS (0x07090000)
#define RTC_POWER_OFF_RECORD1_REG (RTC_BASE_ADDRESS + 0x110)

/*****************************************************************************
 *																			 *
 *								 CEC Registers								 *
 *																			 *
 *****************************************************************************/
#endif  //_REG_CPU_TV303_H
