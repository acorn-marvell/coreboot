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

#if 0
        /* used for MMU and CBMEM setup, in MB */
        u32 dram_start = (CONFIG_SYS_SDRAM_BASE >> 20);
        u32 dram_end = 2048/*1024*/;//sdram_max_addressable_mb();      /* plus one... */
        u32 dram_size = dram_end - dram_start;
#endif
	mmu_init();
#if 0
	/* DRAM is cached. */
	mmu_config_range(dram_start, dram_size, DCACHE_WRITEBACK);
	/* Everything beyond the memory is uncached */
	mmu_config_range(dram_end, 4096-dram_end, DCACHE_OFF);
#else
	/* For simpliciy, set everything uncached */
	mmu_config_range(0,4096, DCACHE_OFF);
#endif

#if 0 /* MMU is disabled for now */
	dcache_mmu_enable();
#endif
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
