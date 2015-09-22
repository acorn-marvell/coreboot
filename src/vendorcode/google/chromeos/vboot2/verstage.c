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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.
 */

#include <arch/exception.h>
#include <arch/hlt.h>
#include <console/console.h>
#include <program_loading.h>
#include "../vboot_common.h"

void __attribute__((weak)) verstage_mainboard_init(void)
{
	/* Default empty implementation. */
}

void verstage(void)
{
	console_init();
	exception_init();
	verstage_mainboard_init();

	if (IS_ENABLED(CONFIG_RETURN_FROM_VERSTAGE)) {
		verstage_main();
	} else {
		run_romstage();
		hlt();
	}

	printk(BIOS_INFO, "Slot %c is selected\n", is_slot_a(&ctx) ? 'A' : 'B');
	vb2_set_selected_region(wd, &fw_main);
	timestamp_add_now(TS_END_VBOOT);
	
	/* @SASAKI Force Depthcharge to recovery mode boot */
	{
                extern void vb2_check_recovery(struct vb2_context *ctx); /* vboot_reference/firmware/2lib/2misc.c */
 
                printk(BIOS_INFO, "Force depthcharge to recovery mode\n");
                ctx.flags |= VB2_CONTEXT_FORCE_RECOVERY_MODE;
                vb2_check_recovery(&ctx);
        }
}

#if !IS_ENABLED(CONFIG_CHIPSET_PROVIDES_VERSTAGE_MAIN_SYMBOL)
/* This is for boards that rely on main() for an entry point of a stage. */
void main(void) __attribute__((alias ("verstage")));
#endif
