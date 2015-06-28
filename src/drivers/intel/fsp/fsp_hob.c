/******************************************************************************

Copyright (C) 2013, Intel Corporation

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.
* Neither the name of Intel Corporation nor the names of its contributors may
  be used to endorse or promote products derived from this software without
  specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
  THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************/

/***********************************************************************
 *
 * fsp_hob.c
 *
 * HOB infrastructure code.
 *
 **********************************************************************/

#include <arch/early_variables.h>
#include <arch/hlt.h>
#include <bootstate.h>
#include <cbmem.h>
#include <console/console.h>
#include "fsp_util.h"
#include <ip_checksum.h>
#include <lib.h> // hexdump
#include <string.h>

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

#if IS_ENABLED(CONFIG_ENABLE_MRC_CACHE)
/*
 *  Save the FSP memory HOB (mrc data) to the MRC area in CBMEM
 */
int save_mrc_data(void *hob_start)
{
	u32 *mrc_hob;
	u32 *mrc_hob_data;
	u32 mrc_hob_size;
	struct mrc_data_container *mrc_data;
	int output_len;
	const EFI_GUID mrc_guid = FSP_NON_VOLATILE_STORAGE_HOB_GUID;

	mrc_hob = get_next_guid_hob(&mrc_guid, hob_start);
	if (mrc_hob == NULL) {
		printk(BIOS_DEBUG,
			"Memory Configure Data Hob is not present\n");
		return 0;
	}

	mrc_hob_data = GET_GUID_HOB_DATA(mrc_hob);
	mrc_hob_size = (u32) GET_HOB_LENGTH(mrc_hob);

	printk(BIOS_DEBUG, "Memory Configure Data Hob at %p (size = 0x%x).\n",
			(void *)mrc_hob_data, mrc_hob_size);

	output_len = ALIGN(mrc_hob_size, 16);

	/* Save the MRC S3/fast boot/ADR restore data to cbmem */
	mrc_data = cbmem_add(CBMEM_ID_MRCDATA,
			output_len + sizeof(struct mrc_data_container));

	/* Just return if there was a problem with getting CBMEM */
	if (mrc_data == NULL) {
		printk(BIOS_WARNING,
			"CBMEM was not available to save the fast boot cache data.\n");
		return 0;
	}

	printk(BIOS_DEBUG,
		"Copy FSP MRC DATA to HOB (source addr %p, dest addr %p, %u bytes)\n",
		(void *)mrc_hob_data, mrc_data, output_len);

	mrc_data->mrc_signature = MRC_DATA_SIGNATURE;
	mrc_data->mrc_data_size = output_len;
	mrc_data->reserved = 0;
	memcpy(mrc_data->mrc_data, (const void *)mrc_hob_data, mrc_hob_size);

	/* Zero the unused space in aligned buffer. */
	if (output_len > mrc_hob_size)
		memset((mrc_data->mrc_data + mrc_hob_size), 0,
				output_len - mrc_hob_size);

	mrc_data->mrc_checksum = compute_ip_checksum(mrc_data->mrc_data,
			mrc_data->mrc_data_size);

#if IS_ENABLED(CONFIG_DISPLAY_FAST_BOOT_DATA)
	printk(BIOS_SPEW, "Fast boot data (includes align and checksum):\n");
	hexdump32(BIOS_SPEW, (void *)mrc_data->mrc_data, output_len);
#endif
	return 1;
}

void __attribute__ ((weak)) update_mrc_cache(void *unused)
{
	printk(BIOS_ERR, "Add routine %s to save the MRC data.\n", __func__);
}
#endif /* CONFIG_ENABLE_MRC_CACHE */

static void find_fsp_hob_update_mrc(void *unused)
{
	void *hob_list_ptr;

	/* 0x0000: Print all types */
	hob_list_ptr = get_hob_list();
#if IS_ENABLED(CONFIG_DISPLAY_HOBS)
	print_hob_type_structure(0x000, hob_list_ptr);
#endif

	#if IS_ENABLED(CONFIG_ENABLE_MRC_CACHE)
	if (save_mrc_data(hob_list_ptr))
		update_mrc_cache(NULL);
	else
		printk(BIOS_DEBUG, "Not updating MRC data in flash.\n");
	#endif
}

/* Update the MRC/fast boot cache as part of the late table writing stage */
BOOT_STATE_INIT_ENTRIES(fsp_hob_find) = {
	BOOT_STATE_INIT_ENTRY(BS_WRITE_TABLES, BS_ON_ENTRY,
				find_fsp_hob_update_mrc, NULL),
};
