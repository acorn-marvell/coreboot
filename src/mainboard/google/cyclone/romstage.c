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

static void __attribute__((noinline)) romstage(void)
{
	uint64_t romstage_start_time = timestamp_get();
	console_init();
	exception_init();

	cbmem_initialize_empty();

	timestamp_init(0);
	timestamp_add(TS_START_ROMSTAGE, romstage_start_time);

#if CONFIG_CONSOLE_CBMEM
	cbmemc_reinit();
#endif

        timestamp_add(TS_START_COPYRAM, timestamp_get());
        void *entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA,
                                      "fallback/ramstage");
        timestamp_add(TS_END_COPYRAM, timestamp_get());

	stage_exit(entry);
}

/* Stub to force arm_init_caches to the top, before any stack/memory accesses */
void main(void)
{
	asm volatile ("bl arm_init_caches"
		      ::: "r0","r1","r2","r3","r4","r5","ip");
	romstage();
}
