/*
 * Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of Google, Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
