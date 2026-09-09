obj-y += cpu/

obj-y += plat-$(CFG_CHIP_PLATFORM)/
CFLAGS-y += -Iarch/openrisc/plat-$(CFG_CHIP_PLATFORM)/inc
CFLAGS-y += -Iservice/standby/suspend_resume/mem-$(CFG_CHIP_PLATFORM)/
LINK_RAM-y := arch/openrisc/plat-$(CFG_CHIP_PLATFORM)/ram.ld

