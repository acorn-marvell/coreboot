/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 SolidRun ltd.
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
#include <device/device.h>
#include <elog.h>
#include <boot/coreboot_tables.h>
#include <vendorcode/google/chromeos/chromeos.h>

static void setup_pinmux(void)
{
}

static void setup_kernel_info(void)
{
}

static void mainboard_init(device_t dev)
{
	setup_pinmux();
	setup_kernel_info();
	elog_init();
#if CONFIG_VBOOT_VERIFY_FIRMWARE
	elog_add_boot_reason();
#endif
}

static void mainboard_enable(device_t dev)
{
	dev->ops->init = &mainboard_init;
}

struct chip_operations mainboard_ops = {
	.name	= "acorn",
	.enable_dev = mainboard_enable,
};

void lb_board(struct lb_header *header)
{
	struct lb_range *dma;

	dma = (struct lb_range *)lb_new_record(header);
	dma->tag = LB_TAB_DMA;
	dma->size = sizeof(*dma);
	dma->range_start = CONFIG_DRAM_DMA_START;
	dma->range_size = CONFIG_DRAM_DMA_SIZE;
}
