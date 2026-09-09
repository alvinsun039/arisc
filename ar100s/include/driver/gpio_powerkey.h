#ifndef __GPIO_POWERKEY_H__
#define __GPIO_POWERKEY_H__

#ifdef CFG_GPIO_POWERKEY_USED
s32 gpio_pwrkey_wakeup_init(void);
#else
static inline s32 gpio_pwrkey_wakeup_init(void) { return 0; }
#endif

#endif /*__POWERKEY_H__*/