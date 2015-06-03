/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2009 coresystems GmbH
 * Copyright (C) 2011 The ChromiumOS Authors.  All rights reserved.
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

#include <arch/io.h>
#include <console/console.h>
#include <device/device.h>
#include <vendorcode/google/chromeos/chromeos.h>

#include "chip.h"

static void soc_enable(device_t dev)
{
	ram_resource(dev, 0, CONFIG_SYS_SDRAM_BASE/KiB,
		(1024)*KiB -
		CONFIG_SYS_SDRAM_BASE/KiB);
}

static void soc_init(device_t dev)
{
	printk(BIOS_INFO, "CPU: Armada 38X\n");
}

static void soc_noop(device_t dev)
{
}

static struct device_operations soc_ops = {
	.read_resources   = soc_noop,
	.set_resources    = soc_noop,
	.enable_resources = soc_enable,
	.init             = soc_init,
	.scan_bus         = 0,
};

static void enable_armada38x_dev(device_t dev)
{
	dev->ops = &soc_ops;
}

struct chip_operations soc_marvell_armada38x_ops = {
	CHIP_NAME("SOC Marvell Armada 38x")
	.enable_dev = enable_armada38x_dev,
};
