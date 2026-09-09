#ifndef __FAKE_POWER_OFF_H__
#define __FAKE_POWER_OFF_H__

#ifdef CFG_FAKE_POWER_OFF
s32 fake_poweroff_pmu_dts_parse(void);
s32 fake_poweroff_pin_dts_parse(void);
void fake_poweroff_pmu_wakeup_init(void);
void fake_poweroff_pins_wakeup_init(void);
#endif
#endif /*__FAKE_POWER_OFF_H__*/