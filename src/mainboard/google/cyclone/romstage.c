/*
 * This file is part of the coreboot project.
 *
 * Copyright 2015 Marvell ltd.
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

#include <arch/cache.h>
#include <arch/cpu.h>
#include <arch/exception.h>
#include <arch/io.h>
#include <arch/stages.h>
#include <cbfs.h>
#include <cbmem.h>
#include <console/cbmem_console.h>
#include <console/console.h>
#include <romstage_handoff.h>
#include <timestamp.h>
# include <lib.h>
#include <vendorcode/google/chromeos/chromeos.h>

void main(void)
{
	void *entry;

	console_init();
	exception_init();

	cbmem_initialize_empty();

	entry = vboot2_load_ramstage();

	printk(BIOS_ERR, "cyclone enter ram stage\n");
	/*
	 * Presumably the only reason vboot2 would return NULL is that we're
	 * running in recovgery mode, otherwise it would have reset the
	 * device.
	 */
	if (!entry)
		entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA,
					CONFIG_CBFS_PREFIX "/ramstage");

	stage_exit(entry);
}
