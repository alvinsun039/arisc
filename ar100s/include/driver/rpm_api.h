
#ifndef __RPM_API_H__
#define __RPM_API_H__

#include "stdint.h"

#define RPM_CB_MSG_FROM_ARM     0
#define RPM_CB_MSG_FROM_MIPS    1
typedef void (*rpm_message_cb)(uint8_t, uint8_t, uint8_t *);

#ifdef CFG_HW_MSGBOX_RPM_API_USED

/*
*********************************************************************************************************
*                            INITIALIZE RPM(Remote Processor Messaging) API
*
* Description:  initialize rpm api.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize succeeded, others if failed.
*********************************************************************************************************
*/
int rpm_init(void);

/*
*********************************************************************************************************
*                                       SEND MESSAGE TO ARM BY RPM API
*
* Description:  send one message to arm processor by rpm api.
*
* Arguments  :  type        : the message type to identify this message, which is defined by user.
*               length      : the payload length of pdata, in unit of byte.
*               pdata       : pointer to data that need to be send
*
* Returns    :  OK if send message succeeded, other if failed.
*********************************************************************************************************
*/
int rpm_send_to_arm(uint8_t type, uint8_t lenght, const uint8_t *pdata);

/*
*********************************************************************************************************
*                                       SEND MESSAGE TO MIPS BY RPM API
*
* Description:  send one message to MIPS processor by rpm api.
*
* Arguments  :  type        : the message type to identify this message, which is defined by user.
*               length      : the payload length of pdata, in unit of byte.
*               pdata       : pointer to data that need to be send
*
* Returns    :  OK if send message succeeded, other if failed.
*********************************************************************************************************
*/
int rpm_send_to_mips(uint8_t type, uint8_t lenght, const uint8_t *pdata);

/*
*********************************************************************************************************
*                                  Register RPM message callback function
*
* Description:  register a callback function to handle incoming message from remote processor
*
* Arguments  :  callback_type  : the callback type,
*                                should be RPM_CB_MSG_FROM_ARM or RPM_CB_MSG_FROM_ARM.
*               rpm_message_cb : the callback function to be register
*
* Returns    :  OK if callback func is registered, ENOSPC if other.
*********************************************************************************************************
*/
int rpm_register_message_callback(int callback_type, rpm_message_cb cb);
#else
static inline int rpm_init(void) { return -1; }
static inline int rpm_send(uint8_t type, uint8_t lenght, const uint8_t *pdata) { return -1; }
static inline int rpm_register_message_callback(int callback_type, rpm_message_cb cb) { return -1; }
#endif

#endif
