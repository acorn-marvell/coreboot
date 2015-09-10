/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013-2014 Sage Electronic Engineering, LLC.
 * Copyright (C) 2015 Intel Corp.
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

#include <bootstate.h>
#include <cbmem.h>
#include <console/console.h>
#include "fsp_util.h"
#include <timestamp.h>

/* Locate the FSP binary in the coreboot filesystem */
FSP_INFO_HEADER *find_fsp(uintptr_t fsp_base_address)
{
	union {
		EFI_FFS_FILE_HEADER *ffh;
		FSP_INFO_HEADER *fih;
		EFI_FIRMWARE_VOLUME_EXT_HEADER *fveh;
		EFI_FIRMWARE_VOLUME_HEADER *fvh;
		EFI_RAW_SECTION *rs;
		u8 *u8;
		u32 u32;
	} fsp_ptr;
	u32 *image_id;

#ifndef CONFIG_FSP_LOC
#error "CONFIG_FSP_LOC must be set."
#endif

	for (;;) {
		/* Get the FSP binary base address in CBFS */
		fsp_ptr.u32 = fsp_base_address;

		/* Check the FV signature, _FVH */
		if (fsp_ptr.fvh->Signature != 0x4856465F) {
			fsp_ptr.u8 = (u8 *)ERROR_NO_FV_SIG;
			break;
		}

		/* Locate the file header which follows the FV header. */
		fsp_ptr.u8 += fsp_ptr.fvh->ExtHeaderOffset;
		fsp_ptr.u8 += fsp_ptr.fveh->ExtHeaderSize;
		fsp_ptr.u8 = (u8 *)((fsp_ptr.u32 + 7) & 0xFFFFFFF8);

		/* Check the FFS GUID */
		if ((((u32 *)&fsp_ptr.ffh->Name)[0] != 0x912740BE)
			|| (((u32 *)&fsp_ptr.ffh->Name)[1] != 0x47342284)
			|| (((u32 *)&fsp_ptr.ffh->Name)[2] != 0xB08471B9)
			|| (((u32 *)&fsp_ptr.ffh->Name)[3] != 0x0C3F3527)) {
			fsp_ptr.u8 = (u8 *)ERROR_NO_FFS_GUID;
			break;
		}

		/* Locate the Raw Section Header */
		fsp_ptr.u8 += sizeof(EFI_FFS_FILE_HEADER);

		if (fsp_ptr.rs->Type != EFI_SECTION_RAW) {
			fsp_ptr.u8 = (u8 *)ERROR_NO_INFO_HEADER;
			break;
		}

		/* Locate the FSP INFO Header which follows the Raw Header. */
		fsp_ptr.u8 += sizeof(EFI_RAW_SECTION);

		/* Verify that the FSP base address.*/
		if (fsp_ptr.fih->ImageBase != fsp_base_address) {
			fsp_ptr.u8 = (u8 *)ERROR_IMAGEBASE_MISMATCH;
			break;
		}

		/* Verify the FSP Signature */
		if (fsp_ptr.fih->Signature != FSP_SIG) {
			fsp_ptr.u8 = (u8 *)ERROR_INFO_HEAD_SIG_MISMATCH;
			break;
		}

		/* Verify the FSP ID */
		image_id = (u32 *)&fsp_ptr.fih->ImageId[0];
		if ((image_id[0] != CONFIG_FSP_IMAGE_ID_DWORD0)
			|| (image_id[1] != CONFIG_FSP_IMAGE_ID_DWORD1))
			fsp_ptr.u8 = (u8 *)ERROR_FSP_SIG_MISMATCH;
		break;
	}

	return fsp_ptr.fih;
}

void print_fsp_info(FSP_INFO_HEADER *fsp_header)
{
	u8 *fsp_base;

	fsp_base = (u8 *)fsp_header->ImageBase;
	printk(BIOS_SPEW, "FSP_INFO_HEADER: %p\n", fsp_header);
	printk(BIOS_INFO, "FSP Signature: %c%c%c%c%c%c%c%c\n",
			fsp_header->ImageId[0], fsp_header->ImageId[1],
			fsp_header->ImageId[2], fsp_header->ImageId[3],
			fsp_header->ImageId[4], fsp_header->ImageId[5],
			fsp_header->ImageId[6], fsp_header->ImageId[7]);
	printk(BIOS_INFO, "FSP Header Version: %d\n",
			fsp_header->HeaderRevision);
	printk(BIOS_INFO, "FSP Revision: %d.%d\n",
			(u8)((fsp_header->ImageRevision >> 8) & 0xff),
			(u8)(fsp_header->ImageRevision  & 0xff));
#if IS_ENABLED(CONFIG_DISPLAY_FSP_ENTRY_POINTS)
	printk(BIOS_SPEW, "FSP Entry Points:\n");
	printk(BIOS_SPEW, "    0x%p: Image Base\n", fsp_base);
	printk(BIOS_SPEW, "    0x%p: TempRamInit\n",
		&fsp_base[fsp_header->TempRamInitEntryOffset]);
	printk(BIOS_SPEW, "    0x%p: FspInit\n",
		&fsp_base[fsp_header->FspInitEntryOffset]);
	if (fsp_header->HeaderRevision >= FSP_HEADER_REVISION_2) {
		printk(BIOS_SPEW, "    0x%p: MemoryInit\n",
			&fsp_base[fsp_header->FspMemoryInitEntryOffset]);
		printk(BIOS_SPEW, "    0x%p: TempRamExit\n",
			&fsp_base[fsp_header->TempRamExitEntryOffset]);
		printk(BIOS_SPEW, "    0x%p: SiliconInit\n",
			&fsp_base[fsp_header->FspSiliconInitEntryOffset]);
	}
	printk(BIOS_SPEW, "    0x%p: NotifyPhase\n",
		&fsp_base[fsp_header->NotifyPhaseEntryOffset]);
	printk(BIOS_SPEW, "    0x%p: Image End\n",
			&fsp_base[fsp_header->ImageSize]);
#endif
}

void fsp_notify(u32 phase)
{
	FSP_NOTIFY_PHASE notify_phase_proc;
	NOTIFY_PHASE_PARAMS notify_phase_params;
	EFI_STATUS status;
	FSP_INFO_HEADER *fsp_header_ptr;

	fsp_header_ptr = fsp_get_fih();
	if (fsp_header_ptr == NULL) {
		fsp_header_ptr = (void *)find_fsp(CONFIG_FSP_LOC);
		if ((u32)fsp_header_ptr < 0xff) {
			/* output something in case there is no serial */
			post_code(0x4F);
			die("Can't find the FSP!\n");
		}
	}

	/* call FSP PEI to Notify PostPciEnumeration */
	notify_phase_proc = (FSP_NOTIFY_PHASE)(fsp_header_ptr->ImageBase +
		fsp_header_ptr->NotifyPhaseEntryOffset);
	notify_phase_params.Phase = phase;

	timestamp_add_now(phase == EnumInitPhaseReadyToBoot ?
		TS_FSP_BEFORE_FINALIZE : TS_FSP_BEFORE_ENUMERATE);

	status = notify_phase_proc(&notify_phase_params);

	timestamp_add_now(phase == EnumInitPhaseReadyToBoot ?
		TS_FSP_AFTER_FINALIZE : TS_FSP_AFTER_ENUMERATE);

	if (status != 0)
		printk(BIOS_ERR, "FSP API NotifyPhase failed for phase 0x%x with status: 0x%x\n",
			phase, status);
}

static void fsp_notify_boot_state_callback(void *arg)
{
	u32 phase = (u32)arg;

	printk(BIOS_SPEW, "Calling FspNotify(0x%08x)\n", phase);
	fsp_notify(phase);
}

BOOT_STATE_INIT_ENTRY(BS_DEV_RESOURCES, BS_ON_EXIT,
	fsp_notify_boot_state_callback,
	(void *)EnumInitPhaseAfterPciEnumeration);
BOOT_STATE_INIT_ENTRY(BS_PAYLOAD_LOAD, BS_ON_EXIT,
	fsp_notify_boot_state_callback,
	(void *)EnumInitPhaseReadyToBoot);
BOOT_STATE_INIT_ENTRY(BS_OS_RESUME, BS_ON_ENTRY,
	fsp_notify_boot_state_callback,
	(void *)EnumInitPhaseReadyToBoot);

struct fsp_runtime {
	uint32_t fih;
	uint32_t hob_list;
} __attribute__((packed));


void fsp_set_runtime(FSP_INFO_HEADER *fih, void *hob_list)
{
	struct fsp_runtime *fspr;

	fspr = cbmem_add(CBMEM_ID_FSP_RUNTIME, sizeof(*fspr));

	if (fspr == NULL)
		die("Can't save FSP runtime information.\n");

	fspr->fih = (uintptr_t)fih;
	fspr->hob_list = (uintptr_t)hob_list;
}

FSP_INFO_HEADER *fsp_get_fih(void)
{
	struct fsp_runtime *fspr;

	fspr = cbmem_find(CBMEM_ID_FSP_RUNTIME);

	if (fspr == NULL)
		return NULL;

	return (void *)(uintptr_t)fspr->fih;
}

void *fsp_get_hob_list(void)
{
	struct fsp_runtime *fspr;

	fspr = cbmem_find(CBMEM_ID_FSP_RUNTIME);

	if (fspr == NULL)
		return NULL;

	return (void *)(uintptr_t)fspr->hob_list;
}

void fsp_update_fih(FSP_INFO_HEADER *fih)
{
	struct fsp_runtime *fspr;

	fspr = cbmem_find(CBMEM_ID_FSP_RUNTIME);

	if (fspr == NULL)
		die("Can't update FSP runtime information.\n");

	fspr->fih = (uintptr_t)fih;
}
