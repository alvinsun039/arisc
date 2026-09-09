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
* Descript: hdmi assist tx header.
* Update  : date                auther      ver     notes
*           2025-11-5 15:44:00  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef _HDMITX_H_
#define _HDMITX_H_

#include "include.h"

int hdmitx_cmd_handler(uint8_t length, uint8_t *pdata);

void hdmitx_main_loop(void);

int hdmitx_init(void);

#endif /*_HDMITX_H_*/
