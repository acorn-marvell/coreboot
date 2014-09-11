/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 Rockchip Inc.
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

#include <assert.h>
#include <console/console.h>
#include <device/i2c.h>
#include <stdint.h>
#include <stdlib.h>
#include "rk808.h"

#define RK808_ADDR	0x1b

#define LDO_EN		0x24
#define LDO_ONSEL(i)	(0x39 + 2 * i)
#define LDO_SLPSEL(i)	(0x3a + 2 * i)

static void rk808_clrsetbits(uint8_t bus, uint8_t reg, uint8_t clr, uint8_t set)
{
	uint8_t value;

	if (i2c_readb(bus, RK808_ADDR, reg, &value) ||
	    i2c_writeb(bus, RK808_ADDR, reg, (value & ~clr) | set))
		printk(BIOS_ERR, "ERROR: Cannot set Rk808[%#x]!\n", reg);
}

void rk808_configure_ldo(uint8_t bus, int ldo, int millivolts)
{
	uint8_t vsel;

	switch (ldo) {
	case 1:
	case 2:
	case 4:
	case 5:
	case 8:
		vsel = millivolts / 100 - 18;
		break;
	case 3:
	case 6:
	case 7:
		vsel = millivolts / 100 - 8;
		break;
	default:
		die("Unknown LDO index!");
	}
	assert(vsel <= 0x10);

	rk808_clrsetbits(bus, LDO_ONSEL(ldo), 0x1f, vsel);
	rk808_clrsetbits(bus, LDO_EN, 0, 1 << (ldo - 1));
}
