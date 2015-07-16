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
	TTB(0x12000000, 16K + 32)
	CBFS_CACHE(0x12006000, 80K)
	STACK(0x1201c000, 16K)
	OVERLAP_VERSTAGE_ROMSTAGE(0x1202c000, 96K)
	VBOOT2_WORK(0x12044000, 16K)
}
