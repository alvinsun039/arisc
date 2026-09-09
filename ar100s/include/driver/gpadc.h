#ifndef __GPADC_H__
#define __GPADC_H__

#ifdef CFG_GPADC_USED
extern int sunxi_gpadc_init(void);
#else /* CFG_GPADC_USED */
static inline int sunxi_gpadc_init(void) { return -1; };
#endif /* CFG_GPADC_USED */

#endif