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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.
 */

#include <assert.h>
#include <arch/cache.h>
#include <arch/exception.h>
#include <arch/hlt.h>
#include <bootblock_common.h>
#include <cbfs.h>
#include <console/console.h>
#include <delay.h>
#include <arch/stages.h>
#include <symbols.h>
#include <vendorcode/google/chromeos/chromeos.h>

//cube proto2
#define A38x_CUSTOMER_BOARD_0_MPP0_7        0x00001111
#define A38x_CUSTOMER_BOARD_0_MPP8_15       0x46200000
#define A38x_CUSTOMER_BOARD_0_MPP16_23      0x00400444
#define A38x_CUSTOMER_BOARD_0_MPP24_31      0x00043300
#define A38x_CUSTOMER_BOARD_0_MPP32_39      0x44400000
#define A38x_CUSTOMER_BOARD_0_MPP40_47      0x00000004
#define A38x_CUSTOMER_BOARD_0_MPP48_55      0x00444444
#define A38x_CUSTOMER_BOARD_0_MPP56_63      0x00004444

#define DRAM_START          ((uintptr_t)_dram / MiB)
#define DRAM_SIZE           (CONFIG_DRAM_SIZE_MB)
/* DMA memory for drivers */
#define DMA_START            ((uintptr_t)_dma_coherent / MiB)
#define DMA_SIZE             (_dma_coherent_size / MiB)

static void setup_pinmux(void)
{
	/* Hard coded pin mux configuration */
	*(volatile unsigned int *) 0xf1018000 = A38x_CUSTOMER_BOARD_0_MPP0_7;
	*(volatile unsigned int *) 0xf1018004 = A38x_CUSTOMER_BOARD_0_MPP8_15;
	*(volatile unsigned int *) 0xf1018008 = A38x_CUSTOMER_BOARD_0_MPP16_23;
	*(volatile unsigned int *) 0xf101800c = A38x_CUSTOMER_BOARD_0_MPP24_31;
	*(volatile unsigned int *) 0xf1018010 = A38x_CUSTOMER_BOARD_0_MPP32_39;
	*(volatile unsigned int *) 0xf1018014 = A38x_CUSTOMER_BOARD_0_MPP40_47;
	*(volatile unsigned int *) 0xf1018018 = A38x_CUSTOMER_BOARD_0_MPP48_55;
	*(volatile unsigned int *) 0xf101801c = A38x_CUSTOMER_BOARD_0_MPP56_63;
}

static void setup_win_regs(void)
{
	*(volatile unsigned int *) 0xf1020250 = 0x1ff00001;
	*(volatile unsigned int *) 0xf1020254 = 0xe0000000;
	*(volatile unsigned int *) 0xf1020200 = 0x00000000;
}

void main(void)
{
	if (CONFIG_BOOTBLOCK_CONSOLE) {
		console_init();
		exception_init();
	}

	init_timer();

	//enable mmu
	mmu_init();
	mmu_config_range(0, 4096, DCACHE_OFF);
	mmu_config_range(DRAM_START, DRAM_SIZE, DCACHE_WRITEBACK);
	mmu_config_range(DMA_START, DMA_SIZE, DCACHE_OFF);
	dcache_mmu_enable();
	
	bootblock_mainboard_init();
	
	setup_pinmux();
	setup_win_regs();
	
	run_romstage();
}
