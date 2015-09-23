/*
 * cbfstool, CLI utility for CBFS file manipulation
 *
 * Copyright (C) 2009 coresystems GmbH
 *                 written by Patrick Georgi <patrick.georgi@coresystems.de>
 * Copyright (C) 2012 Google, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include "common.h"
#include "cbfs.h"
#include "cbfs_image.h"
#include "cbfs_sections.h"
#include "fit.h"
#include "partitioned_file.h"

#define SECTION_WITH_FIT_TABLE	"BOOTBLOCK"

struct command {
	const char *name;
	const char *optstring;
	int (*function) (void);
	// Whether to populate param.image_region before invoking function
	bool accesses_region;
	// Whether to write that region's contents back to image_file at the end
	bool modifies_region;
};

static struct param {
	partitioned_file_t *image_file;
	struct buffer *image_region;
	const char *name;
	const char *filename;
	const char *fmap;
	const char *region_name;
	const char *bootblock;
	const char *ignore_section;
	uint64_t u64val;
	uint32_t type;
	uint32_t baseaddress;
	uint32_t baseaddress_assigned;
	uint32_t loadaddress;
	uint32_t copyoffset;
	uint32_t copyoffset_assigned;
	uint32_t headeroffset;
	uint32_t headeroffset_assigned;
	uint32_t entrypoint;
	uint32_t size;
	uint32_t alignment;
	uint32_t pagesize;
	uint32_t cbfsoffset;
	uint32_t cbfsoffset_assigned;
	uint32_t arch;
	bool fill_partial_upward;
	bool fill_partial_downward;
	bool show_immutable;
	bool stage_xip;
	int fit_empty_entries;
	enum comp_algo compression;
	/* for linux payloads */
	char *initrd;
	char *cmdline;
} param = {
	/* All variables not listed are initialized as zero. */
	.arch = CBFS_ARCHITECTURE_UNKNOWN,
	.compression = CBFS_COMPRESS_NONE,
	.headeroffset = ~0,
	.region_name = SECTION_NAME_PRIMARY_CBFS,
};

static bool region_is_flashmap(const char *region)
{
	return partitioned_file_region_check_magic(param.image_file, region,
					FMAP_SIGNATURE, strlen(FMAP_SIGNATURE));
}

/* @return Same as cbfs_is_valid_cbfs(), but for a named region. */
static bool region_is_modern_cbfs(const char *region)
{
	return partitioned_file_region_check_magic(param.image_file, region,
				CBFS_FILE_MAGIC, strlen(CBFS_FILE_MAGIC));
}

/*
 * Converts between offsets from the start of the specified image region and
 * "top-aligned" offsets from the top of the entire flash image. Works in either
 * direction: pass in one type of offset and receive the other type.
 * N.B. A top-aligned offset is always a positive number, and should not be
 * confused with a top-aliged *address*, which is its arithmetic inverse. */
static unsigned convert_to_from_top_aligned(const struct buffer *region,
								unsigned offset)
{
	assert(region);

	size_t image_size = partitioned_file_total_size(param.image_file);
	return image_size - region->offset - offset;
}

static int do_cbfs_locate(int32_t *cbfs_addr, size_t metadata_size)
{
	if (!param.filename) {
		ERROR("You need to specify -f/--filename.\n");
		return 1;
	}

	if (!param.name) {
		ERROR("You need to specify -n/--name.\n");
		return 1;
	}

	struct cbfs_image image;
	if (cbfs_image_from_buffer(&image, param.image_region,
							param.headeroffset))
		return 1;

	if (cbfs_get_entry(&image, param.name))
		WARN("'%s' already in CBFS.\n", param.name);

	struct buffer buffer;
	if (buffer_from_file(&buffer, param.filename) != 0) {
		ERROR("Cannot load %s.\n", param.filename);
		return 1;
	}

	/* Include cbfs_file size along with space for with name. */
	metadata_size += cbfs_calculate_file_header_size(param.name);

	int32_t address = cbfs_locate_entry(&image, buffer.size, param.pagesize,
						param.alignment, metadata_size);
	buffer_delete(&buffer);

	if (address == -1) {
		ERROR("'%s' can't fit in CBFS for page-size %#x, align %#x.\n",
		      param.name, param.pagesize, param.alignment);
		return 1;
	}

	*cbfs_addr = address;
	return 0;
}

typedef int (*convert_buffer_t)(struct buffer *buffer, uint32_t *offset,
	struct cbfs_file *header);

static int cbfs_add_integer_component(const char *name,
			      uint64_t u64val,
			      uint32_t offset,
			      uint32_t headeroffset) {
	struct cbfs_image image;
	struct cbfs_file *header = NULL;
	struct buffer buffer;
	int i, ret = 1;

	if (!name) {
		ERROR("You need to specify -n/--name.\n");
		return 1;
	}

	if (buffer_create(&buffer, 8, name) != 0)
		return 1;

	for (i = 0; i < 8; i++)
		buffer.data[i] = (u64val >> i*8) & 0xff;

	if (cbfs_image_from_buffer(&image, param.image_region, headeroffset)) {
		ERROR("Selected image region is not a CBFS.\n");
		goto done;
	}

	if (cbfs_get_entry(&image, name)) {
		ERROR("'%s' already in ROM image.\n", name);
		goto done;
	}

	if (IS_TOP_ALIGNED_ADDRESS(offset))
		offset = convert_to_from_top_aligned(param.image_region,
								-offset);

	header = cbfs_create_file_header(CBFS_COMPONENT_RAW,
		buffer.size, name);
	if (cbfs_add_entry(&image, &buffer, offset, header) != 0) {
		ERROR("Failed to add %llu into ROM image as '%s'.\n",
					(long long unsigned)u64val, name);
		goto done;
	}

	ret = 0;

done:
	free(header);
	buffer_delete(&buffer);
	return ret;
}

static int cbfs_add_component(const char *filename,
			      const char *name,
			      uint32_t type,
			      uint32_t offset,
			      uint32_t headeroffset,
			      convert_buffer_t convert)
{
	if (!filename) {
		ERROR("You need to specify -f/--filename.\n");
		return 1;
	}

	if (!name) {
		ERROR("You need to specify -n/--name.\n");
		return 1;
	}

	if (type == 0) {
		ERROR("You need to specify a valid -t/--type.\n");
		return 1;
	}

	struct cbfs_image image;
	if (cbfs_image_from_buffer(&image, param.image_region, headeroffset))
		return 1;

	if (cbfs_get_entry(&image, name)) {
		ERROR("'%s' already in ROM image.\n", name);
		return 1;
	}

	struct buffer buffer;
	if (buffer_from_file(&buffer, filename) != 0) {
		ERROR("Could not load file '%s'.\n", filename);
		return 1;
	}

	struct cbfs_file *header =
		cbfs_create_file_header(type, buffer.size, name);

	if (convert && convert(&buffer, &offset, header) != 0) {
		ERROR("Failed to parse file '%s'.\n", filename);
		buffer_delete(&buffer);
		return 1;
	}

	if (IS_TOP_ALIGNED_ADDRESS(offset))
		offset = convert_to_from_top_aligned(param.image_region,
								-offset);

	if (cbfs_add_entry(&image, &buffer, offset, header) != 0) {
		ERROR("Failed to add '%s' into ROM image.\n", filename);
		free(header);
		buffer_delete(&buffer);
		return 1;
	}

	free(header);
	buffer_delete(&buffer);
	return 0;
}

static int cbfstool_convert_raw(struct buffer *buffer,
	unused uint32_t *offset, struct cbfs_file *header)
{
	char *compressed;
	int compressed_size;

	comp_func_ptr compress = compression_function(param.compression);
	if (!compress)
		return -1;
	compressed = calloc(buffer->size, 1);

	if (compress(buffer->data, buffer->size,
		     compressed, &compressed_size)) {
		WARN("Compression failed - disabled\n");
	} else {
		struct cbfs_file_attr_compression *attrs =
			(struct cbfs_file_attr_compression *)
			cbfs_add_file_attr(header,
				CBFS_FILE_ATTR_TAG_COMPRESSION,
				sizeof(struct cbfs_file_attr_compression));
		if (attrs == NULL)
			return -1;
		attrs->compression = htonl(param.compression);
		attrs->decompressed_size = htonl(buffer->size);

		free(buffer->data);
		buffer->data = compressed;
		buffer->size = compressed_size;

		header->len = htonl(buffer->size);
	}
	return 0;
}

static int cbfstool_convert_mkstage(struct buffer *buffer, uint32_t *offset,
	struct cbfs_file *header)
{
	struct buffer output;
	int ret;

	if (param.stage_xip) {
		int32_t address;

		if (do_cbfs_locate(&address, sizeof(struct cbfs_stage)))  {
			ERROR("Could not find location for XIP stage.\n");
			return 1;
		}

		/* Pass in a top aligned address. */
		address = -convert_to_from_top_aligned(param.image_region,
								address);
		*offset = address;

		ret = parse_elf_to_xip_stage(buffer, &output, offset,
						param.ignore_section);
	} else
		ret = parse_elf_to_stage(buffer, &output, param.compression,
					 offset, param.ignore_section);

	if (ret != 0)
		return -1;
	buffer_delete(buffer);
	// direct assign, no dupe.
	memcpy(buffer, &output, sizeof(*buffer));
	header->len = htonl(output.size);
	return 0;
}

static int cbfstool_convert_mkpayload(struct buffer *buffer,
	unused uint32_t *offset, struct cbfs_file *header)
{
	struct buffer output;
	int ret;
	/* per default, try and see if payload is an ELF binary */
	ret = parse_elf_to_payload(buffer, &output, param.compression);

	/* If it's not an ELF, see if it's a UEFI FV */
	if (ret != 0)
		ret = parse_fv_to_payload(buffer, &output, param.compression);

	/* If it's neither ELF nor UEFI Fv, try bzImage */
	if (ret != 0)
		ret = parse_bzImage_to_payload(buffer, &output,
				param.initrd, param.cmdline, param.compression);

	/* Not a supported payload type */
	if (ret != 0) {
		ERROR("Not a supported payload type (ELF / FV).\n");
		buffer_delete(buffer);
		return -1;
	}

	buffer_delete(buffer);
	// direct assign, no dupe.
	memcpy(buffer, &output, sizeof(*buffer));
	header->len = htonl(output.size);
	return 0;
}

static int cbfstool_convert_mkflatpayload(struct buffer *buffer,
	unused uint32_t *offset, struct cbfs_file *header)
{
	struct buffer output;
	if (parse_flat_binary_to_payload(buffer, &output,
					 param.loadaddress,
					 param.entrypoint,
					 param.compression) != 0) {
		return -1;
	}
	buffer_delete(buffer);
	// direct assign, no dupe.
	memcpy(buffer, &output, sizeof(*buffer));
	header->len = htonl(output.size);
	return 0;
}

static int cbfs_add(void)
{
	int32_t address;

	if (param.alignment && param.baseaddress) {
		ERROR("Cannot specify both alignment and base address\n");
		return 1;
	}

	if (param.alignment) {
		/* CBFS compression file attribute is unconditionally added. */
		size_t metadata_sz = sizeof(struct cbfs_file_attr_compression);
		if (do_cbfs_locate(&address, metadata_sz))
			return 1;
		param.baseaddress = address;
	}

	return cbfs_add_component(param.filename,
				  param.name,
				  param.type,
				  param.baseaddress,
				  param.headeroffset,
				  cbfstool_convert_raw);
}

static int cbfs_add_stage(void)
{
	if (param.stage_xip) {
		if (param.baseaddress_assigned) {
			ERROR("Cannot specify base address for XIP.\n");
			return 1;
		}

		if (param.compression != CBFS_COMPRESS_NONE) {
			ERROR("Cannot specify compression for XIP.\n");
			return 1;
		}
	}

	return cbfs_add_component(param.filename,
				  param.name,
				  CBFS_COMPONENT_STAGE,
				  param.baseaddress,
				  param.headeroffset,
				  cbfstool_convert_mkstage);
}

static int cbfs_add_payload(void)
{
	return cbfs_add_component(param.filename,
				  param.name,
				  CBFS_COMPONENT_PAYLOAD,
				  param.baseaddress,
				  param.headeroffset,
				  cbfstool_convert_mkpayload);
}

static int cbfs_add_flat_binary(void)
{
	if (param.loadaddress == 0) {
		ERROR("You need to specify a valid "
			"-l/--load-address.\n");
		return 1;
	}
	if (param.entrypoint == 0) {
		ERROR("You need to specify a valid "
			"-e/--entry-point.\n");
		return 1;
	}
	return cbfs_add_component(param.filename,
				  param.name,
				  CBFS_COMPONENT_PAYLOAD,
				  param.baseaddress,
				  param.headeroffset,
				  cbfstool_convert_mkflatpayload);
}

static int cbfs_add_integer(void)
{
	return cbfs_add_integer_component(param.name,
				  param.u64val,
				  param.baseaddress,
				  param.headeroffset);
}

static int cbfs_remove(void)
{
	if (!param.name) {
		ERROR("You need to specify -n/--name.\n");
		return 1;
	}

	struct cbfs_image image;
	if (cbfs_image_from_buffer(&image, param.image_region,
							param.headeroffset))
		return 1;

	if (cbfs_remove_entry(&image, param.name) != 0) {
		ERROR("Removing file '%s' failed.\n",
		      param.name);
		return 1;
	}

	return 0;
}

static int cbfs_create(void)
{
	struct cbfs_image image;
	memset(&image, 0, sizeof(image));
	buffer_clone(&image.buffer, param.image_region);

	if (param.fmap) {
		if (param.arch != CBFS_ARCHITECTURE_UNKNOWN || param.size ||
						param.baseaddress_assigned ||
						param.headeroffset_assigned ||
						param.cbfsoffset_assigned ||
							param.bootblock) {
			ERROR("Since -M was provided, -m, -s, -b, -o, -H, and -B should be omitted\n");
			return 1;
		}

		return cbfs_image_create(&image, image.buffer.size);
	}

	if (param.arch == CBFS_ARCHITECTURE_UNKNOWN) {
		ERROR("You need to specify -m/--machine arch.\n");
		return 1;
	}

	struct buffer bootblock;
	if (!param.bootblock) {
		DEBUG("-B not given, creating image without bootblock.\n");
		buffer_create(&bootblock, 0, "(dummy)");
	} else if (buffer_from_file(&bootblock, param.bootblock)) {
		return 1;
	}

	if (!param.alignment)
		param.alignment = CBFS_ALIGNMENT;

	// Set default offsets. x86, as usual, needs to be a special snowflake.
	if (!param.baseaddress_assigned) {
		if (param.arch == CBFS_ARCHITECTURE_X86) {
			// Make sure there's at least enough room for rel_offset
			param.baseaddress = param.size -
					MAX(bootblock.size, sizeof(int32_t));
			DEBUG("x86 -> bootblock lies at end of ROM (%#x).\n",
			      param.baseaddress);
		} else {
			param.baseaddress = 0;
			DEBUG("bootblock starts at address 0x0.\n");
		}
	}
	if (!param.headeroffset_assigned) {
		if (param.arch == CBFS_ARCHITECTURE_X86) {
			param.headeroffset = param.baseaddress -
					     sizeof(struct cbfs_header);
			DEBUG("x86 -> CBFS header before bootblock (%#x).\n",
				param.headeroffset);
		} else {
			param.headeroffset = align_up(param.baseaddress +
				bootblock.size, sizeof(uint32_t));
			DEBUG("CBFS header placed behind bootblock (%#x).\n",
				param.headeroffset);
		}
	}
	if (!param.cbfsoffset_assigned) {
		if (param.arch == CBFS_ARCHITECTURE_X86) {
			param.cbfsoffset = 0;
			DEBUG("x86 -> CBFS entries start at address 0x0.\n");
		} else {
			param.cbfsoffset = align_up(param.headeroffset +
						    sizeof(struct cbfs_header),
						    CBFS_ALIGNMENT);
			DEBUG("CBFS entries start beind master header (%#x).\n",
			      param.cbfsoffset);
		}
	}

	int ret = cbfs_legacy_image_create(&image,
					   param.arch,
					   CBFS_ALIGNMENT,
					   &bootblock,
					   param.baseaddress,
					   param.headeroffset,
					   param.cbfsoffset);
	buffer_delete(&bootblock);
	return ret;
}

static int cbfs_layout(void)
{
	const struct fmap *fmap = partitioned_file_get_fmap(param.image_file);
	if (!fmap) {
		LOG("This is a legacy image composed entirely of a single CBFS.\n");
		return 1;
	}

	printf("This image contains the following sections that can be %s with this tool:\n",
			param.show_immutable ? "accessed" : "manipulated");
	puts("");
	for (unsigned i = 0; i < fmap->nareas; ++i) {
		const struct fmap_area *current = fmap->areas + i;

		bool readonly = partitioned_file_fmap_count(param.image_file,
			partitioned_file_fmap_select_children_of, current) ||
				region_is_flashmap((const char *)current->name);
		if (!param.show_immutable && readonly)
			continue;

		printf("'%s'", current->name);

		// Detect consecutive sections that describe the same region and
		// show them as aliases. This cannot find equivalent entries
		// that aren't adjacent; however, fmaptool doesn't generate
		// FMAPs with such sections, so this convenience feature works
		// for all but the strangest manually created FMAP binaries.
		// TODO: This could be done by parsing the FMAP into some kind
		// of tree that had duplicate lists in addition to child lists,
		// which would allow covering that weird, unlikely case as well.
		unsigned lookahead;
		for (lookahead = 1; i + lookahead < fmap->nareas;
								++lookahead) {
			const struct fmap_area *consecutive =
					fmap->areas + i + lookahead;
			if (consecutive->offset != current->offset ||
					consecutive->size != current->size)
				break;
			printf(", '%s'", consecutive->name);
		}
		if (lookahead > 1)
			fputs(" are aliases for the same region", stdout);

		const char *qualifier = "";
		if (readonly)
			qualifier = "read-only, ";
		else if (region_is_modern_cbfs((const char *)current->name))
			qualifier = "CBFS, ";
		printf(" (%ssize %u)\n", qualifier, current->size);

		i += lookahead - 1;
	}
	puts("");

	if (param.show_immutable) {
		puts("It is at least possible to perform the read action on every section listed above.");
	} else {
		puts("It is possible to perform either the write action or the CBFS add/remove actions on every section listed above.");
		puts("To see the image's read-only sections as well, rerun with the -w option.");
	}

	return 0;
}

static int cbfs_print(void)
{
	struct cbfs_image image;
	if (cbfs_image_from_buffer(&image, param.image_region,
							param.headeroffset))
		return 1;
	cbfs_print_directory(&image);
	return 0;
}

static int cbfs_extract(void)
{
	if (!param.filename) {
		ERROR("You need to specify -f/--filename.\n");
		return 1;
	}

	if (!param.name) {
		ERROR("You need to specify -n/--name.\n");
		return 1;
	}

	struct cbfs_image image;
	if (cbfs_image_from_buffer(&image, param.image_region,
							param.headeroffset))
		return 1;

	return cbfs_export_entry(&image, param.name, param.filename);
}

static int cbfs_write(void)
{
	if (!param.filename) {
		ERROR("You need to specify a valid input -f/--file.\n");
		return 1;
	}
	if (!partitioned_file_is_partitioned(param.image_file)) {
		ERROR("This operation isn't valid on legacy images having CBFS master headers\n");
		return 1;
	}

	if (region_is_modern_cbfs(param.region_name)) {
		ERROR("Target image region '%s' is a CBFS and must be manipulated using add and remove\n",
							param.region_name);
		return 1;
	}

	struct buffer new_content;
	if (buffer_from_file(&new_content, param.filename))
		return 1;

	if (buffer_check_magic(&new_content, FMAP_SIGNATURE,
						strlen(FMAP_SIGNATURE))) {
		ERROR("File '%s' appears to be an FMAP and cannot be added to an existing image\n",
								param.filename);
		buffer_delete(&new_content);
		return 1;
	}
	if (buffer_check_magic(&new_content, CBFS_FILE_MAGIC,
						strlen(CBFS_FILE_MAGIC))) {
		ERROR("File '%s' appears to be a CBFS and cannot be inserted into a raw region\n",
								param.filename);
		buffer_delete(&new_content);
		return 1;
	}

	unsigned offset = 0;
	if (param.fill_partial_upward && param.fill_partial_downward) {
		ERROR("You may only specify one of -u and -d.\n");
		buffer_delete(&new_content);
		return 1;
	} else if (!param.fill_partial_upward && !param.fill_partial_downward) {
		if (new_content.size != param.image_region->size) {
			ERROR("File to add is %zu bytes and would not fill %zu-byte target region (did you mean to pass either -u or -d?)\n",
				new_content.size, param.image_region->size);
			buffer_delete(&new_content);
			return 1;
		}
	} else {
		if (new_content.size > param.image_region->size) {
			ERROR("File to add is %zu bytes and would overflow %zu-byte target region\n",
				new_content.size, param.image_region->size);
			buffer_delete(&new_content);
			return 1;
		}
		WARN("Written area will abut %s of target region: any unused space will keep its current contents\n",
				param.fill_partial_upward ? "bottom" : "top");
		if (param.fill_partial_downward)
			offset = param.image_region->size - new_content.size;
	}

	memcpy(param.image_region->data + offset, new_content.data,
							new_content.size);
	buffer_delete(&new_content);
	return 0;
}

static int cbfs_read(void)
{
	if (!param.filename) {
		ERROR("You need to specify a valid output -f/--file.\n");
		return 1;
	}
	if (!partitioned_file_is_partitioned(param.image_file)) {
		ERROR("This operation isn't valid on legacy images having CBFS master headers\n");
		return 1;
	}

	return buffer_write_file(param.image_region, param.filename);
}

static int cbfs_update_fit(void)
{
	if (!param.name) {
		ERROR("You need to specify -n/--name.\n");
		return 1;
	}

	if (param.fit_empty_entries <= 0) {
		ERROR("Invalid number of fit entries "
		        "(-x/--empty-fits): %d\n", param.fit_empty_entries);
		return 1;
	}

	// Decide which region to read/write the FIT table from/to.
	struct buffer bootblock;
	if (partitioned_file_is_partitioned(param.image_file)) {
		if (!partitioned_file_read_region(&bootblock, param.image_file,
							SECTION_WITH_FIT_TABLE))
			return 1;
	} else {
		// In legacy images, the bootblock is part of the CBFS.
		buffer_clone(&bootblock, param.image_region);
	}

	struct cbfs_image image;
	if (cbfs_image_from_buffer(&image, param.image_region,
							param.headeroffset))
		return 1;

	if (fit_update_table(&bootblock, &image, param.name,
			param.fit_empty_entries, convert_to_from_top_aligned))
		return 1;

	// The region to be written depends on the type of image, so we write it
	// here rather than having main() write the CBFS region back as usual.
	return !partitioned_file_write_region(param.image_file, &bootblock);
}

static int cbfs_copy(void)
{
	if (!param.copyoffset_assigned) {
		ERROR("You need to specify -D/--copy_offset.\n");
		return 1;
	}

	if (!param.size) {
		ERROR("You need to specify -s/--size.\n");
		return 1;
	}

	struct cbfs_image image;
	if (cbfs_image_from_buffer(&image, param.image_region,
							param.headeroffset))
		return 1;

	if (!cbfs_is_legacy_cbfs(&image)) {
		ERROR("This operation is only valid on legacy images having CBFS master headers\n");
		return 1;
	}

	return cbfs_copy_instance(&image, param.copyoffset, param.size);
}

static bool cbfs_is_legacy_format(struct buffer *buffer)
{
	// Legacy CBFSes are those containing the deprecated CBFS master header.
	return cbfs_find_header(buffer->data, buffer->size, -1);
}

static const struct command commands[] = {
	{"add", "H:r:f:n:t:c:b:a:vh?", cbfs_add, true, true},
	{"add-flat-binary", "H:r:f:n:l:e:c:b:vh?", cbfs_add_flat_binary, true,
									true},
	{"add-payload", "H:r:f:n:t:c:b:C:I:vh?", cbfs_add_payload, true, true},
	{"add-stage", "a:H:r:f:n:t:c:b:P:S:yvh?", cbfs_add_stage, true, true},
	{"add-int", "H:r:i:n:b:vh?", cbfs_add_integer, true, true},
	{"copy", "H:D:s:h?", cbfs_copy, true, true},
	{"create", "M:r:s:B:b:H:o:m:vh?", cbfs_create, true, true},
	{"extract", "H:r:n:f:vh?", cbfs_extract, true, false},
	{"layout", "wvh?", cbfs_layout, false, false},
	{"print", "H:r:vh?", cbfs_print, true, false},
	{"read", "r:f:vh?", cbfs_read, true, false},
	{"remove", "H:r:n:vh?", cbfs_remove, true, true},
	{"update-fit", "H:r:n:x:vh?", cbfs_update_fit, true, false},
	{"write", "r:f:udvh?", cbfs_write, true, true},
};

static struct option long_options[] = {
	{"alignment",     required_argument, 0, 'a' },
	{"base-address",  required_argument, 0, 'b' },
	{"bootblock",     required_argument, 0, 'B' },
	{"cmdline",       required_argument, 0, 'C' },
	{"compression",   required_argument, 0, 'c' },
	{"copy-offset",   required_argument, 0, 'D' },
	{"empty-fits",    required_argument, 0, 'x' },
	{"entry-point",   required_argument, 0, 'e' },
	{"file",          required_argument, 0, 'f' },
	{"fill-downward", no_argument,       0, 'd' },
	{"fill-upward",   no_argument,       0, 'u' },
	{"flashmap",      required_argument, 0, 'M' },
	{"fmap-regions",  required_argument, 0, 'r' },
	{"header-offset", required_argument, 0, 'H' },
	{"help",          no_argument,       0, 'h' },
	{"ignore-sec",    required_argument, 0, 'S' },
	{"initrd",        required_argument, 0, 'I' },
	{"int",           required_argument, 0, 'i' },
	{"load-address",  required_argument, 0, 'l' },
	{"machine",       required_argument, 0, 'm' },
	{"name",          required_argument, 0, 'n' },
	{"offset",        required_argument, 0, 'o' },
	{"page-size",     required_argument, 0, 'P' },
	{"size",          required_argument, 0, 's' },
	{"top-aligned",   required_argument, 0, 'T' },
	{"type",          required_argument, 0, 't' },
	{"verbose",       no_argument,       0, 'v' },
	{"with-readonly", no_argument,       0, 'w' },
	{"xip",           no_argument,       0, 'y' },
	{NULL,            0,                 0,  0  }
};

static int dispatch_command(struct command command)
{
	if (command.accesses_region) {
		assert(param.image_file);

		if (partitioned_file_is_partitioned(param.image_file)) {
			LOG("Performing operation on '%s' region...\n",
					param.region_name);
		}
		if (!partitioned_file_read_region(param.image_region,
					param.image_file, param.region_name)) {
			ERROR("The image will be left unmodified.\n");
			return 1;
		}

		if (command.modifies_region) {
			// We (intentionally) don't support overwriting the FMAP
			// section. If you find yourself wanting to do this,
			// consider creating a new image rather than performing
			// whatever hacky transformation you were planning.
			if (region_is_flashmap(param.region_name)) {
				ERROR("Image region '%s' is read-only because it contains the FMAP.\n",
							param.region_name);
				ERROR("The image will be left unmodified.\n");
				return 1;
			}
			// We don't allow writing raw data to regions that
			// contain nested regions, since doing so would
			// overwrite all such subregions.
			if (partitioned_file_region_contains_nested(
					param.image_file, param.region_name)) {
				ERROR("Image region '%s' is read-only because it contains nested regions.\n",
							param.region_name);
				ERROR("The image will be left unmodified.\n");
				return 1;
			}
		}
	}

	if (command.function()) {
		if (partitioned_file_is_partitioned(param.image_file)) {
			ERROR("Failed while operating on '%s' region!\n",
							param.region_name);
			ERROR("The image will be left unmodified.\n");
		}
		return 1;
	}

	return 0;
}

static void usage(char *name)
{
	printf
	    ("cbfstool: Management utility for CBFS formatted ROM images\n\n"
	     "USAGE:\n" " %s [-h]\n"
	     " %s FILE COMMAND [-v] [PARAMETERS]...\n\n" "OPTIONs:\n"
	     "  -H header_offset Do not search for header; use this offset*\n"
	     "  -T               Output top-aligned memory address\n"
	     "  -u               Accept short data; fill upward/from bottom\n"
	     "  -d               Accept short data; fill downward/from top\n"
	     "  -v               Provide verbose output\n"
	     "  -h               Display this help message\n\n"
	     "COMMANDs:\n"
	     " add [-r image,regions] -f FILE -n NAME -t TYPE \\\n"
	     "        [-c compression] [-b base-address | -a alignment]    "
			"Add a component\n"
	     " add-payload [-r image,regions] -f FILE -n NAME \\\n"
	     "        [-c compression] [-b base-address]                   "
			"Add a payload to the ROM\n"
	     "        (linux specific: [-C cmdline] [-I initrd])\n"
	     " add-stage [-r image,regions] -f FILE -n NAME \\\n"
	     "        [-c compression] [-b base] [-S section-to-ignore]    "
	     "        [-a alignment] [-y|--xip] [-P page-size]"
			"Add a stage to the ROM\n"
	     " add-flat-binary [-r image,regions] -f FILE -n NAME \\\n"
	     "        -l load-address -e entry-point [-c compression] \\\n"
	     "        [-b base]                                            "
			"Add a 32bit flat mode binary\n"
	     " add-int [-r image,regions] -i INTEGER -n NAME [-b base]     "
			"Add a raw 64-bit integer value\n"
	     " remove [-r image,regions] -n NAME                           "
			"Remove a component\n"
	     " copy -D new_header_offset -s region size \\\n"
	     "        [-H source header offset]                            "
			"Create a copy (duplicate) cbfs instance*\n"
	     " create -m ARCH -s size [-b bootblock offset] \\\n"
	     "        [-o CBFS offset] [-H header offset] [-B bootblock]   "
			"Create a legacy ROM file with CBFS master header*\n"
	     " create -M flashmap [-r list,of,regions,containing,cbfses]   "
			"Create a new-style partitioned firmware image\n"
	     " locate [-r image,regions] -f FILE -n NAME [-P page-size] \\\n"
	     "        [-a align] [-T]                                      "
			"Find a place for a file of that size\n"
	     " layout [-w]                                                 "
			"List mutable (or, with -w, readable) image regions\n"
	     " print [-r image,regions]                                    "
			"Show the contents of the ROM\n"
	     " extract [-r image,regions] -n NAME -f FILE                  "
			"Extracts a raw payload from ROM\n"
	     " write -r image,regions -f file [-u | -d]                    "
			"Write file into same-size [or larger] raw region\n"
	     " read [-r fmap-region] -f file                               "
			"Extract raw region contents into binary file\n"
	     " update-fit [-r image,regions] -n MICROCODE_BLOB_NAME \\\n"
	     "          -x EMTPY_FIT_ENTRIES                               "
			"Updates the FIT table with microcode entries\n"
	     "\n"
	     "OFFSETs:\n"
	     "  Numbers accompanying -b, -H, and -o switches* may be provided\n"
	     "  in two possible formats: if their value is greater than\n"
	     "  0x80000000, they are interpreted as a top-aligned x86 memory\n"
	     "  address; otherwise, they are treated as an offset into flash.\n"
	     "ARCHes:\n"
	     "  arm64, arm, mips, x86\n"
	     "TYPEs:\n", name, name
	    );
	print_supported_filetypes();

	printf(
	     "\n* Note that these actions and switches are only valid when\n"
	     "  working with legacy images whose structure is described\n"
	     "  primarily by a CBFS master header. New-style images, in\n"
	     "  contrast, exclusively make use of an FMAP to describe their\n"
	     "  layout: this must minimally contain an '%s' section\n"
	     "  specifying the location of this FMAP itself and a '%s'\n"
	     "  section describing the primary CBFS. It should also be noted\n"
	     "  that, when working with such images, the -F and -r switches\n"
	     "  default to '%s' for convenience, and both the -b switch to\n"
	     "  CBFS operations and the output of the locate action become\n"
	     "  relative to the selected CBFS region's lowest address.\n"
	     "  The one exception to this rule is the top-aligned address,\n"
	     "  which is always relative to the end of the entire image\n"
	     "  rather than relative to the local region; this is true for\n"
	     "  for both input (sufficiently large) and output (-T) data.\n",
	     SECTION_NAME_FMAP, SECTION_NAME_PRIMARY_CBFS,
	     SECTION_NAME_PRIMARY_CBFS
	     );
}

int main(int argc, char **argv)
{
	size_t i;
	int c;

	if (argc < 3) {
		usage(argv[0]);
		return 1;
	}

	char *image_name = argv[1];
	char *cmd = argv[2];
	optind += 2;

	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(cmd, commands[i].name) != 0)
			continue;

		while (1) {
			char *suffix = NULL;
			int option_index = 0;

			c = getopt_long(argc, argv, commands[i].optstring,
						long_options, &option_index);
			if (c == -1)
				break;

			/* filter out illegal long options */
			if (strchr(commands[i].optstring, c) == NULL) {
				/* TODO maybe print actual long option instead */
				ERROR("%s: invalid option -- '%c'\n",
				      argv[0], c);
				c = '?';
			}

			switch(c) {
			case 'n':
				param.name = optarg;
				break;
			case 't':
				if (intfiletype(optarg) != ((uint64_t) - 1))
					param.type = intfiletype(optarg);
				else
					param.type = strtoul(optarg, NULL, 0);
				if (param.type == 0)
					WARN("Unknown type '%s' ignored\n",
							optarg);
				break;
			case 'c': {
				int algo = cbfs_parse_comp_algo(optarg);
				if (algo >= 0)
					param.compression = algo;
				else
					WARN("Unknown compression '%s' ignored.\n",
									optarg);
				break;
			}
			case 'M':
				param.fmap = optarg;
				break;
			case 'r':
				param.region_name = optarg;
				break;
			case 'b':
				param.baseaddress = strtoul(optarg, NULL, 0);
				// baseaddress may be zero on non-x86, so we
				// need an explicit "baseaddress_assigned".
				param.baseaddress = strtoul(optarg, NULL, 0);
				param.baseaddress_assigned = 1;
				break;
			case 'l':
				param.loadaddress = strtoul(optarg, NULL, 0);
				break;
			case 'e':
				param.entrypoint = strtoul(optarg, NULL, 0);
				break;
			case 's':
				param.size = strtoul(optarg, &suffix, 0);
				if (tolower(suffix[0])=='k') {
					param.size *= 1024;
				}
				if (tolower(suffix[0])=='m') {
					param.size *= 1024 * 1024;
				}
				break;
			case 'B':
				param.bootblock = optarg;
				break;
			case 'H':
				param.headeroffset = strtoul(
						optarg, NULL, 0);
				param.headeroffset_assigned = 1;
				break;
			case 'D':
				param.copyoffset = strtoul(optarg, NULL, 0);
				param.copyoffset_assigned = 1;
				break;
			case 'a':
				param.alignment = strtoul(optarg, NULL, 0);
				break;
			case 'P':
				param.pagesize = strtoul(optarg, NULL, 0);
				break;
			case 'o':
				param.cbfsoffset = strtoul(optarg, NULL, 0);
				param.cbfsoffset_assigned = 1;
				break;
			case 'f':
				param.filename = optarg;
				break;
			case 'i':
				param.u64val = strtoull(optarg, NULL, 0);
				break;
			case 'u':
				param.fill_partial_upward = true;
				break;
			case 'd':
				param.fill_partial_downward = true;
				break;
			case 'w':
				param.show_immutable = true;
				break;
			case 'x':
				param.fit_empty_entries = strtol(optarg, NULL, 0);
				break;
			case 'v':
				verbose++;
				break;
			case 'm':
				param.arch = string_to_arch(optarg);
				break;
			case 'I':
				param.initrd = optarg;
				break;
			case 'C':
				param.cmdline = optarg;
				break;
			case 'S':
				param.ignore_section = optarg;
				break;
			case 'y':
				param.stage_xip = true;
				break;
			case 'h':
			case '?':
				usage(argv[0]);
				return 1;
			default:
				break;
			}
		}

		if (commands[i].function == cbfs_create) {
			if (param.fmap) {
				struct buffer flashmap;
				if (buffer_from_file(&flashmap, param.fmap))
					return 1;
				param.image_file = partitioned_file_create(
							image_name, &flashmap);
				buffer_delete(&flashmap);
			} else if (param.size) {
				param.image_file = partitioned_file_create_flat(
							image_name, param.size);
			} else {
				ERROR("You need to specify a valid -M/--flashmap or -s/--size.\n");
				return 1;
			}
		} else {
			param.image_file =
				partitioned_file_reopen(image_name,
							cbfs_is_legacy_format);
		}
		if (!param.image_file)
			return 1;

		unsigned num_regions = 1;
		for (const char *list = strchr(param.region_name, ','); list;
						list = strchr(list + 1, ','))
			++num_regions;

		// If the action needs to read an image region, as indicated by
		// having accesses_region set in its command struct, that
		// region's buffer struct will be stored here and the client
		// will receive a pointer to it via param.image_region. It
		// need not write the buffer back to the image file itself,
		// since this behavior can be requested via its modifies_region
		// field. Additionally, it should never free the region buffer,
		// as that is performed automatically once it completes.
		struct buffer image_regions[num_regions];
		memset(image_regions, 0, sizeof(image_regions));

		bool seen_primary_cbfs = false;
		char region_name_scratch[strlen(param.region_name) + 1];
		strcpy(region_name_scratch, param.region_name);
		param.region_name = strtok(region_name_scratch, ",");
		for (unsigned region = 0; region < num_regions; ++region) {
			if (!param.region_name) {
				ERROR("Encountered illegal degenerate region name in -r list\n");
				ERROR("The image will be left unmodified.\n");
				partitioned_file_close(param.image_file);
				return 1;
			}

			if (strcmp(param.region_name, SECTION_NAME_PRIMARY_CBFS)
									== 0)
				seen_primary_cbfs = true;

			param.image_region = image_regions + region;
			if (dispatch_command(commands[i])) {
				partitioned_file_close(param.image_file);
				return 1;
			}

			param.region_name = strtok(NULL, ",");
		}

		if (commands[i].function == cbfs_create && !seen_primary_cbfs) {
			ERROR("The creation -r list must include the mandatory '%s' section.\n",
						SECTION_NAME_PRIMARY_CBFS);
			ERROR("The image will be left unmodified.\n");
			partitioned_file_close(param.image_file);
			return 1;
		}

		if (commands[i].modifies_region) {
			assert(param.image_file);
			assert(commands[i].accesses_region);
			for (unsigned region = 0; region < num_regions;
								++region) {

				if (!partitioned_file_write_region(
							param.image_file,
						image_regions + region)) {
					partitioned_file_close(
							param.image_file);
					return 1;
				}
			}
		}

		partitioned_file_close(param.image_file);
		return 0;
	}

	ERROR("Unknown command '%s'.\n", cmd);
	usage(argv[0]);
	return 1;
}
