/****************************************************************************
 * hdmirx/awDefs.h
 *
 * Copyright (C) 2022-2024 AllWinnertech Ltd.
 * Author: huangwenhao <huangwenhao@allwinnertech.com>
 ***************************************************************************/

#ifndef __VSDEFS_H
#define __VSDEFS_H

#define _BIT0_ 1
#define _BIT1_ 2
#define _BIT2_ 4
#define _BIT3_ 8
#define _BIT4_ 0x10
#define _BIT5_ 0x20
#define _BIT6_ 0x40
#define _BIT7_ 0x80

typedef unsigned long long	uint64_t;
typedef long long			sint64_t;
typedef long long			int64_t;
typedef unsigned long		uint32_t;
typedef signed long			sint32_t;
typedef signed long			int32_t;
typedef unsigned short		uint16_t;
typedef signed short		sint16_t;
typedef signed short		int16_t;
typedef signed char 		sint8_t;
typedef signed char 		int8_t;
typedef unsigned char		uint8_t;

#ifdef _TRUE_
#undef _TRUE_
#endif
#ifdef _FALSE_
#undef _FALSE_
#endif
#define _FALSE_ (uint8_t)0
#define _TRUE_ (uint8_t)(!_FALSE_)
#define _HI_ _TRUE_
#define _LO_ _FALSE_
#define _ON_ _TRUE_
#define _OFF_ _FALSE_

#ifndef _NULL_
#define _NULL_ 0
#endif

#ifndef NULL
#define NULL (void *)0
#endif

#define TIMER_INACTIVE 0
#define TIMER_EXPIRED 1

#endif  //__VSDEFS_H
