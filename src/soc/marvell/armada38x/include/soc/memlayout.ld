/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 Google Inc.
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

#include <memlayout.h>
#include <vendorcode/google/chromeos/memlayout.h>

#include <arch/header.ld>

SECTIONS
{
        DRAM_START(0x00000000)
	BOOTBLOCK(0x00000000, 24K)
        RAMSTAGE(0x00800000, 128K)
	CBFS_CACHE(0x12006000, 80K)
	CBFS_HEADER_OFFSET(0x1201a000)
	STACK(0x1201c000, 16K)

	VERSTAGE(0x1202c000, 96K)
	ROMSTAGE(0x12044000, 96K)
	VBOOT2_WORK(0x1205c000, 16K)
	DMA_COHERENT(0x12100000, 2M)
	SYMBOL(memlayout_cbmem_top, 0x1F400000)
	TTB(0x1FF00000, 16K)
        TTB_SUBTABLES(0x1FF04000, 2K)
}
