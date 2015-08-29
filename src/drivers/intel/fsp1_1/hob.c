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

#include <arch/early_variables.h>
#include <arch/hlt.h>
#include <bootstate.h>
#include <cbmem.h>
#include <console/console.h>
#include "fsp_util.h"
#include <ip_checksum.h>
#include <lib.h> // hexdump
#include <string.h>
#include <soc/intel/common/mrc_cache.h>

/*
 * Reads a 64-bit value from memory that may be unaligned.
 *
 * This function returns the 64-bit value pointed to by buffer. The
 * function guarantees that the read operation does not produce an
 * alignment fault.
 *
 * If buffer is NULL, then ASSERT().
 *
 * buffer: Pointer to a 64-bit value that may be unaligned.
 *
 * Returns the 64-bit value read from buffer.
 *
 */
static
uint64_t
read_unaligned_64(
	const uint64_t *buffer
	)
{
	ASSERT(buffer != NULL);

	return *buffer;
}

/*
 * Compares two GUIDs.
 *
 * This function compares guid1 to guid2.  If the GUIDs are identical then
 * TRUE is returned.  If there are any bit differences in the two GUIDs,
 * then FALSE is returned.
 *
 * If guid1 is NULL, then ASSERT().
 * If guid2 is NULL, then ASSERT().
 *
 * guid1: A pointer to a 128 bit GUID.
 * guid2: A pointer to a 128 bit GUID.
 *
 * Returns non-zero if guid1 and guid2 are identical, otherwise returns 0.
 *
 */
static
long
compare_guid(
	const EFI_GUID * guid1,
	const EFI_GUID * guid2
	)
{
	uint64_t low_part_of_guid1;
	uint64_t low_part_of_guid2;
	uint64_t high_part_of_guid1;
	uint64_t high_part_of_guid2;

	low_part_of_guid1  = read_unaligned_64((const uint64_t *) guid1);
	low_part_of_guid2  = read_unaligned_64((const uint64_t *) guid2);
	high_part_of_guid1 = read_unaligned_64((const uint64_t *) guid1 + 1);
	high_part_of_guid2 = read_unaligned_64((const uint64_t *) guid2 + 1);

	return ((low_part_of_guid1 == low_part_of_guid2)
		&& (high_part_of_guid1 == high_part_of_guid2));
}

/* Returns the pointer to the HOB list. */
VOID *
EFIAPI
get_hob_list(
	VOID
	)
{
	void *hob_list;

	hob_list = fsp_get_hob_list();
	if (hob_list == NULL)
		die("Call fsp_set_runtime() before this call!\n");
	return hob_list;
}

/* Returns the next instance of a HOB type from the starting HOB. */
VOID *
EFIAPI
get_next_hob(
	UINT16 type,
	CONST VOID *hob_start
	)
{
	EFI_PEI_HOB_POINTERS hob;

	ASSERT(hob_start != NULL);

	hob.Raw = (UINT8 *)hob_start;

	/* Parse the HOB list until end of list or matching type is found. */
	while (!END_OF_HOB_LIST(hob.Raw)) {
		if (hob.Header->HobType == type)
			return hob.Raw;
		if (GET_HOB_LENGTH(hob.Raw) < sizeof(*hob.Header))
			break;
		hob.Raw = GET_NEXT_HOB(hob.Raw);
	}
	return NULL;
}

/* Returns the first instance of a HOB type among the whole HOB list. */
VOID *
EFIAPI
get_first_hob(
	UINT16 type
	)
{
	VOID *hob_list;

	hob_list = get_hob_list();
	return get_next_hob(type, hob_list);
}

/* Returns the next instance of the matched GUID HOB from the starting HOB. */
VOID *
EFIAPI
get_next_guid_hob(
	CONST EFI_GUID * guid,
	CONST VOID *hob_start
	)
{
	EFI_PEI_HOB_POINTERS hob;

	hob.Raw = (UINT8 *)hob_start;
	while ((hob.Raw = get_next_hob(EFI_HOB_TYPE_GUID_EXTENSION, hob.Raw))
					!= NULL) {
		if (compare_guid(guid, &hob.Guid->Name))
			break;
		hob.Raw = GET_NEXT_HOB(hob.Raw);
	}
	return hob.Raw;
}

/*
 * Returns the first instance of the matched GUID HOB among the whole HOB list.
 */
VOID *
EFIAPI
get_first_guid_hob(
	CONST EFI_GUID * guid
	)
{
	return get_next_guid_hob(guid, get_hob_list());
}

/*
 * Returns the next instance of the matching resource HOB from the starting HOB.
 */
void *get_next_resource_hob(const EFI_GUID *guid, const void *hob_start)
{
	EFI_PEI_HOB_POINTERS hob;

	hob.Raw = (UINT8 *)hob_start;
	while ((hob.Raw = get_next_hob(EFI_HOB_TYPE_RESOURCE_DESCRIPTOR,
					    hob.Raw)) != NULL) {
		if (compare_guid(guid, &hob.ResourceDescriptor->Owner))
			break;
		hob.Raw = GET_NEXT_HOB(hob.Raw);
	}
	return hob.Raw;
}

/*
 * Returns the first instance of the matching resource HOB among the whole HOB
 * list.
 */
void *get_first_resource_hob(const EFI_GUID *guid)
{
	return get_next_resource_hob(guid, get_hob_list());
}

static void print_hob_mem_attributes(void *hob_ptr)
{
	EFI_HOB_MEMORY_ALLOCATION *hob_memory_ptr =
		(EFI_HOB_MEMORY_ALLOCATION *)hob_ptr;
	EFI_MEMORY_TYPE hob_mem_type =
		hob_memory_ptr->AllocDescriptor.MemoryType;
	u64 hob_mem_addr = hob_memory_ptr->AllocDescriptor.MemoryBaseAddress;
	u64 hob_mem_length = hob_memory_ptr->AllocDescriptor.MemoryLength;
	const char *hob_mem_type_names[15];

	hob_mem_type_names[0] = "EfiReservedMemoryType";
	hob_mem_type_names[1] = "EfiLoaderCode";
	hob_mem_type_names[2] = "EfiLoaderData";
	hob_mem_type_names[3] = "EfiBootServicesCode";
	hob_mem_type_names[4] = "EfiBootServicesData";
	hob_mem_type_names[5] = "EfiRuntimeServicesCode";
	hob_mem_type_names[6] = "EfiRuntimeServicesData";
	hob_mem_type_names[7] = "EfiConventionalMemory";
	hob_mem_type_names[8] = "EfiUnusableMemory";
	hob_mem_type_names[9] = "EfiACPIReclaimMemory";
	hob_mem_type_names[10] = "EfiACPIMemoryNVS";
	hob_mem_type_names[11] = "EfiMemoryMappedIO";
	hob_mem_type_names[12] = "EfiMemoryMappedIOPortSpace";
	hob_mem_type_names[13] = "EfiPalCode";
	hob_mem_type_names[14] = "EfiMaxMemoryType";

	printk(BIOS_SPEW, "  Memory type %s (0x%x)\n",
			hob_mem_type_names[(u32)hob_mem_type],
			(u32)hob_mem_type);
	printk(BIOS_SPEW, "  at location 0x%0lx with length 0x%0lx\n",
			(unsigned long)hob_mem_addr,
			(unsigned long)hob_mem_length);
}

static void print_hob_resource_attributes(void *hob_ptr)
{
	EFI_HOB_RESOURCE_DESCRIPTOR *hob_resource_ptr =
		(EFI_HOB_RESOURCE_DESCRIPTOR *)hob_ptr;
	u32 hob_res_type   = hob_resource_ptr->ResourceType;
	u32 hob_res_attr   = hob_resource_ptr->ResourceAttribute;
	u64 hob_res_addr   = hob_resource_ptr->PhysicalStart;
	u64 hob_res_length = hob_resource_ptr->ResourceLength;
	const char *hob_res_type_str = NULL;

	/* HOB Resource Types */
	switch (hob_res_type) {
	case EFI_RESOURCE_SYSTEM_MEMORY:
		hob_res_type_str = "EFI_RESOURCE_SYSTEM_MEMORY";
		break;
	case EFI_RESOURCE_MEMORY_MAPPED_IO:
		hob_res_type_str = "EFI_RESOURCE_MEMORY_MAPPED_IO";
		break;
	case EFI_RESOURCE_IO:
		hob_res_type_str = "EFI_RESOURCE_IO";
		break;
	case EFI_RESOURCE_FIRMWARE_DEVICE:
		hob_res_type_str = "EFI_RESOURCE_FIRMWARE_DEVICE";
		break;
	case EFI_RESOURCE_MEMORY_MAPPED_IO_PORT:
		hob_res_type_str = "EFI_RESOURCE_MEMORY_MAPPED_IO_PORT";
		break;
	case EFI_RESOURCE_MEMORY_RESERVED:
		hob_res_type_str = "EFI_RESOURCE_MEMORY_RESERVED";
		break;
	case EFI_RESOURCE_IO_RESERVED:
		hob_res_type_str = "EFI_RESOURCE_IO_RESERVED";
		break;
	case EFI_RESOURCE_MAX_MEMORY_TYPE:
		hob_res_type_str = "EFI_RESOURCE_MAX_MEMORY_TYPE";
		break;
	default:
		hob_res_type_str = "EFI_RESOURCE_UNKNOWN";
		break;
	}

	printk(BIOS_SPEW, "  Resource %s (0x%0x) has attributes 0x%0x\n",
			hob_res_type_str, hob_res_type, hob_res_attr);
	printk(BIOS_SPEW, "  at location 0x%0lx with length 0x%0lx\n",
			(unsigned long)hob_res_addr,
			(unsigned long)hob_res_length);
}

static const char *get_hob_type_string(void *hob_ptr)
{
	EFI_PEI_HOB_POINTERS hob;
	const char *hob_type_string = NULL;
	const EFI_GUID fsp_reserved_guid =
		FSP_RESERVED_MEMORY_RESOURCE_HOB_GUID;
	const EFI_GUID mrc_guid = FSP_NON_VOLATILE_STORAGE_HOB_GUID;
	const EFI_GUID bootldr_tmp_mem_guid =
		FSP_BOOTLOADER_TEMP_MEMORY_HOB_GUID;
	const EFI_GUID bootldr_tolum_guid = FSP_BOOTLOADER_TOLUM_HOB_GUID;
	const EFI_GUID graphics_info_guid = EFI_PEI_GRAPHICS_INFO_HOB_GUID;
	const EFI_GUID memory_info_hob_guid = FSP_SMBIOS_MEMORY_INFO_GUID;

	hob.Header = (EFI_HOB_GENERIC_HEADER *)hob_ptr;
	switch (hob.Header->HobType) {
	case EFI_HOB_TYPE_HANDOFF:
		hob_type_string = "EFI_HOB_TYPE_HANDOFF";
		break;
	case EFI_HOB_TYPE_MEMORY_ALLOCATION:
		hob_type_string = "EFI_HOB_TYPE_MEMORY_ALLOCATION";
		break;
	case EFI_HOB_TYPE_RESOURCE_DESCRIPTOR:
		hob_type_string = "EFI_HOB_TYPE_RESOURCE_DESCRIPTOR";
		if (compare_guid(&fsp_reserved_guid, &hob.Guid->Name))
			hob_type_string = "FSP_RESERVED_MEMORY_RESOURCE_HOB";
		else if (compare_guid(&bootldr_tolum_guid, &hob.Guid->Name))
			hob_type_string = "FSP_BOOTLOADER_TOLUM_HOB_GUID";
		break;
	case EFI_HOB_TYPE_GUID_EXTENSION:
		hob_type_string = "EFI_HOB_TYPE_GUID_EXTENSION";
		if (compare_guid(&bootldr_tmp_mem_guid, &hob.Guid->Name))
			hob_type_string = "FSP_BOOTLOADER_TEMP_MEMORY_HOB";
		else if (compare_guid(&mrc_guid, &hob.Guid->Name))
			hob_type_string = "FSP_NON_VOLATILE_STORAGE_HOB";
		else if (compare_guid(&graphics_info_guid, &hob.Guid->Name))
			hob_type_string = "EFI_PEI_GRAPHICS_INFO_HOB_GUID";
		else if (compare_guid(&memory_info_hob_guid, &hob.Guid->Name))
			hob_type_string = "FSP_SMBIOS_MEMORY_INFO_GUID";
		break;
	case EFI_HOB_TYPE_MEMORY_POOL:
		hob_type_string = "EFI_HOB_TYPE_MEMORY_POOL";
		break;
	case EFI_HOB_TYPE_UNUSED:
		hob_type_string = "EFI_HOB_TYPE_UNUSED";
		break;
	case EFI_HOB_TYPE_END_OF_HOB_LIST:
		hob_type_string = "EFI_HOB_TYPE_END_OF_HOB_LIST";
		break;
	default:
		hob_type_string = "EFI_HOB_TYPE_UNRECOGNIZED";
		break;
	}

	return hob_type_string;
}

/*
 * Print out a structure of all the HOBs
 * that match a certain type:
 * Print all types			(0x0000)
 * EFI_HOB_TYPE_HANDOFF		(0x0001)
 * EFI_HOB_TYPE_MEMORY_ALLOCATION	(0x0002)
 * EFI_HOB_TYPE_RESOURCE_DESCRIPTOR	(0x0003)
 * EFI_HOB_TYPE_GUID_EXTENSION		(0x0004)
 * EFI_HOB_TYPE_MEMORY_POOL		(0x0007)
 * EFI_HOB_TYPE_UNUSED			(0xFFFE)
 * EFI_HOB_TYPE_END_OF_HOB_LIST	(0xFFFF)
 */
void print_hob_type_structure(u16 hob_type, void *hob_list_ptr)
{
	u32 *current_hob;
	u32 *next_hob = 0;
	u8  last_hob = 0;
	u32 current_type;
	const char *current_type_str;

	current_hob = hob_list_ptr;

	/*
	 * Print out HOBs of our desired type until
	 * the end of the HOB list
	 */
	printk(BIOS_DEBUG, "\n=== FSP HOB Data Structure ===\n");
	printk(BIOS_DEBUG, "0x%p: hob_list_ptr\n", hob_list_ptr);
	do {
		EFI_HOB_GENERIC_HEADER *current_header_ptr =
			(EFI_HOB_GENERIC_HEADER *)current_hob;

		/* Get the type of this HOB */
		current_type = current_header_ptr->HobType;
		current_type_str = get_hob_type_string(current_hob);

		if (current_type == hob_type || hob_type == 0x0000) {
			printk(BIOS_DEBUG, "HOB 0x%0x is an %s (type 0x%0x)\n",
					(u32)current_hob, current_type_str,
					current_type);
			switch (current_type) {
			case EFI_HOB_TYPE_MEMORY_ALLOCATION:
				print_hob_mem_attributes(current_hob);
				break;
			case EFI_HOB_TYPE_RESOURCE_DESCRIPTOR:
				print_hob_resource_attributes(current_hob);
				break;
			}
		}

		/* Check for end of HOB list */
		last_hob = END_OF_HOB_LIST(current_hob);
		if (!last_hob) {
			/* Get next HOB pointer */
			next_hob = GET_NEXT_HOB(current_hob);

			/* Start on next HOB */
			current_hob = next_hob;
		}
	} while (!last_hob);
	printk(BIOS_DEBUG, "=== End of FSP HOB Data Structure ===\n\n");
}
