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

#include <assert.h>
#include <arch/cache.h>
#include <arch/exception.h>
#include <arch/hlt.h>
#include <bootblock_common.h>
#include <cbfs.h>
#include <console/console.h>
#include <delay.h>

static void setup_pinmux(void)
{
	/* Hard coded pin mux configuration */
#if 1
	/* Armada 385 DB AP board pin muxing */
	* (volatile unsigned int *) 0xf1018000 = 0x11111111;
	* (volatile unsigned int *) 0xf1018004 = 0x11111111;
	* (volatile unsigned int *) 0xf1018008 = 0x55066011;
	* (volatile unsigned int *) 0xf101800c = 0x05050050;
	* (volatile unsigned int *) 0xf1018010 = 0x05055555;
	* (volatile unsigned int *) 0xf1018014 = 0x01100565;
	* (volatile unsigned int *) 0xf1018018 = 0x00000000;
	* (volatile unsigned int *) 0xf101801c = 0x00004444;
#else
	/* Armada 385 RD board pin muxing */
	* (volatile unsigned int *) 0xf1018000 = 0x11111111;
	* (volatile unsigned int *) 0xf1018004 = 0x11111111;
	* (volatile unsigned int *) 0xf1018008 = 0x11244011;
	* (volatile unsigned int *) 0xf101800c = 0x22222111;
	* (volatile unsigned int *) 0xf1018010 = 0x22200002;
	* (volatile unsigned int *) 0xf1018014 = 0x30042022;
	* (volatile unsigned int *) 0xf1018018 = 0x55550555;
	* (volatile unsigned int *) 0xf101801c = 0x00005550;
#endif
}

#if CONFIG_VBOOT2_VERIFY_FIRMWARE
#include "verstage.h"
#include <vendorcode/google/chromeos/chromeos.h>

static void configure_ec_spi_bus(void)
{
}

static void configure_tpm_i2c_bus(void)
{
}
#endif

void main(void)
{
	volatile unsigned int reg;
	void (*entry)(void);
	* (volatile unsigned int *) 0xd0020080 = 0xf1000000; /* Remap internal registers to 0xf1000000 */
	reg = * (unsigned int *) 0xf1000000; /* Forces read from the just newly remapped registers offset */
	if (CONFIG_BOOTBLOCK_CONSOLE) {
		console_init();
		exception_init();
	}
	init_timer();

	bootblock_mainboard_init();

	setup_pinmux(); /* This was previously under CONFIG_VBOTT2_VERIFY_FIRMWARE below */
#if CONFIG_VBOOT2_VERIFY_FIRMWARE
	configure_ec_spi_bus();
	configure_tpm_i2c_bus();
	entry = (void *)verstage_vboot_main;
#else
	entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA, "fallback/romstage");
#endif
	
	ASSERT(entry);
	entry();
}
