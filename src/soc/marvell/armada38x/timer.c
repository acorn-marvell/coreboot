/*
 * This file is part of the coreboot project.
 *
 * Copyright 2013 Google Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <console/console.h>
#include <timer.h>
#include <delay.h>
#include <thread.h>

#define TIMER_CTRL_REG        0xf1020300
#define TIMER_RELOAD_REG      0xf1020310
#define TIMER_REG             0xf1020314


#define MHZ_NUM               25

unsigned int timer_raw_value(void);
static inline void _delay(unsigned int delta);

void init_timer(void)
{
	unsigned int reg;
	/* Set the reload timer */
	* (volatile unsigned int *)TIMER_RELOAD_REG = 0xffffffff;
	/* And the initial value */
	* (volatile unsigned int *)TIMER_REG = 0xffffffff;
	reg = * (volatile unsigned int *)TIMER_CTRL_REG;
	/* Let it start counting */
	reg |= 0x3;
	* (volatile unsigned int *)TIMER_CTRL_REG = reg;
}

unsigned int timer_raw_value(void)
{
        return *(volatile unsigned int *)TIMER_REG;
}

static inline void _delay(unsigned int delta)
{
        unsigned int now = 0;
        unsigned int change;
        unsigned int last = timer_raw_value();
        while (delta) {
                now = timer_raw_value();
                if (last >= now)
                        change = (last - now);
                else
                        change = last + (0xffffffff - now);

                if (change < delta)
                        delta -= change;
                else
                        break;
                last = now;
        }
}

void udelay(unsigned int n)
{
        _delay((unsigned int)n * MHZ_NUM);
}

