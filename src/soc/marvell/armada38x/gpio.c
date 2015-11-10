/*
 * This file is part of the coreboot project.
 *
 * Copyright 2015 Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.
 */

#include <arch/io.h>
#include <gpio.h>
#include <console/console.h>

#define GPIO_MAX_NUM  64

#define MV_GPP_MAX_GROUP                        3
#define MV_GPP_IN       0xFFFFFFFF      /* GPP input */
#define MV_GPP_OUT      0               /* GPP output */

#define MV_GPP_REGS_BASE(unit) 			(0x18100 + ((unit) * 0x40))
#define GPP_DATA_OUT_EN_REG(grp)                (MV_GPP_REGS_BASE(grp) + 0x04)
#define GPP_DATA_IN_REG(grp)                    (MV_GPP_REGS_BASE(grp) + 0x10)
#define GPP_DATA_OUT_REG(grp)                   (MV_GPP_REGS_BASE(grp) + 0x00)


#define MV_OK 0
#define MV_ERROR -1
#define MV_BAD_PARAM -2


#define INTER_REGS_BASE         0xF1000000
#define MV_REG_READ(offset)             \
        (read32((void *)(INTER_REGS_BASE | (offset))))

#define MV_REG_WRITE(offset, val)    \
        write32((void *)(INTER_REGS_BASE | (offset)), val)


void gppRegSet(u32 group, u32 regOffs, u32 mask, u32 value);
int  mvGppTypeSet(u32 group, u32 mask, u32 value);
u32 mvGppValueGet(u32 group, u32 mask);
int mvGppValueSet(u32 group, u32 mask, u32 value);

void gppRegSet(u32 group, u32 regOffs, u32 mask, u32 value)
{
        u32 gppData;
        gppData = MV_REG_READ(regOffs);
        gppData &= ~mask;
        gppData |= (value & mask);
        MV_REG_WRITE(regOffs, gppData);
}

int  mvGppTypeSet(u32 group, u32 mask, u32 value)
{
        if (group >= MV_GPP_MAX_GROUP) {
                printk(BIOS_INFO, "mvGppTypeSet: ERR. invalid group number \n");
                return MV_BAD_PARAM;
        }
        gppRegSet(group, GPP_DATA_OUT_EN_REG(group), mask, value);
        return MV_OK;
}

u32 mvGppValueGet(u32 group, u32 mask)
{
        u32 gppData;
        gppData = MV_REG_READ(GPP_DATA_IN_REG(group));
        gppData &= mask;
        return gppData;
}

int mvGppValueSet(u32 group, u32 mask, u32 value)
{
        u32 outEnable;
        u32 i;

        if (group >= MV_GPP_MAX_GROUP) {
                printk(BIOS_INFO, "mvGppValueSet: Error invalid group number \n");
                return MV_BAD_PARAM;
        }

        /* verify that the gpp pin is configured as output              */
        /* Note that in the register out enabled -> bit = '0'.  */
        outEnable = ~MV_REG_READ(GPP_DATA_OUT_EN_REG(group));

        for (i = 0; i < 32; i++) {
                if (((mask & (1 << i)) & (outEnable & (1 << i))) != (mask & (1 << i))) {
                        printk(BIOS_INFO, "mvGppValueSet: Err. An attempt to set output "
                                   "value to GPP %d in input mode.\n", i);
                        return MV_ERROR;
                }
        }
        gppRegSet(group, GPP_DATA_OUT_REG(group), mask, value);
        return MV_OK;

}

/*******************************************************
Function description: check for invalid GPIO #
Arguments :
gpio_t gpio - Gpio number

Return : GPIO Valid(0)/Invalid(1)
*******************************************************/

static inline int gpio_not_valid(gpio_t gpio)
{
	return (gpio > GPIO_MAX_NUM);
}

/*******************************************************
Function description: get GPIO IO functinality details
Arguments :
gpio_t gpio - Gpio number
unsigned *in - Value of GPIO input
unsigned *out - Value of GPIO output

Return : None
*******************************************************/
int gpio_get(gpio_t gpio)
{
	u32 group = 0;
	u32 gpp_data;
	if (gpio_not_valid(gpio))
		return -1;

	if(gpio>=32){
		group =1;
		gpio -=32;
	}

	mvGppTypeSet(group, (1 << gpio), MV_GPP_IN & (1 << gpio));
	gpp_data = mvGppValueGet(group, (1 << gpio));
	return (gpp_data!=0);
}

void gpio_set(gpio_t gpio, int value)
{
	u32 group = 0;
	if (gpio_not_valid(gpio))
		return;
	if(gpio>=32){
                group =1;
                gpio -=32;
        }

	mvGppTypeSet(group, (1 << gpio), MV_GPP_OUT & (1 << gpio));
	mvGppValueSet(group, (1 << gpio), (value?(1 << gpio):0));
}
