/****************************************************************************
 * hdmirx/awTimer.h
 *
 * Copyright (C) 2022-2024 AllWinnertech Ltd.
 * Author: huangwenhao <huangwenhao@allwinnertech.com>
 ***************************************************************************/

#ifndef __TTIMER_H
#define __TTIMER_H

#ifdef __cplusplus
extern "C" {
#endif

s32 tdCallBack10msTimer(void *parg);

Void tdSleep(Word wMilliseconds);
Byte tdGetDetailTimeInterval(Bool bIR);
Void tdInitTimer2(Void);

#ifdef __cplusplus
};
#endif

#endif
