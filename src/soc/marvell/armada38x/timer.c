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

void init_timer(void)
{
	unsigned int reg;
	/* Set the reload timer */
	* (volatile unsigned int *) 0xf1020310 = 0xffffffff;
	/* And the initial value */
	* (volatile unsigned int *) 0xf1020314 = 0xffffffff;
	reg = * (volatile unsigned int *)0xf1020300;
	/* Let it start counting */
	reg |= 0x3;
	* (volatile unsigned int *)0xf1020300 = reg;
}


