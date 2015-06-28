/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013 Google, Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <arch/stages.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <cbfs.h>
#include <cbmem.h>
#include <console/console.h>
#include <console/vtxprintf.h>
#include <tpm.h>
#include <reset.h>
#include <romstage_handoff.h>
#include <rmodule.h>
#include <string.h>
#include <stdlib.h>
#include <timestamp.h>
#include <arch/stages.h>
#include "../chromeos.h"
#include "../vboot_context.h"
#include "../vboot_handoff.h"

#define TEMP_CBMEM_ID_VBOOT	0xffffffff
#define TEMP_CBMEM_ID_VBLOCKS	0xfffffffe

static void vboot_run_stub(struct vboot_context *context)
{
	struct rmod_stage_load rmod_stage = {
		.cbmem_id = TEMP_CBMEM_ID_VBOOT,
		.name = CONFIG_CBFS_PREFIX "/vboot",
	};
	void (*entry)(struct vboot_context *context);

	if (rmodule_stage_load_from_cbfs(&rmod_stage)) {
		printk(BIOS_DEBUG, "Could not load vboot stub.\n");
		goto out;
	}

	entry = rmod_stage.entry;

	/* Call stub. */
	entry(context);

out:
	/* Tear down the region no longer needed. */
	if (rmod_stage.cbmem_entry != NULL)
		cbmem_entry_remove(rmod_stage.cbmem_entry);
}

/* Helper routines for the vboot stub. */
static void log_msg(const char *fmt, va_list args)
{
	vtxprintf(console_tx_byte, fmt, args);
	console_tx_flush();
}

static void fatal_error(void)
{
	printk(BIOS_ERR, "vboot encountered fatal error. Reseting.\n");
	vboot_reboot();
}

static int fw_region_size(struct vboot_region *r)
{
	struct vboot_components *fw_info;
	int32_t size;
	int i;

	fw_info = vboot_locate_components(r);
	if (fw_info == NULL)
		return -1;

	if (fw_info->num_components > MAX_PARSED_FW_COMPONENTS)
		return -1;

	size = sizeof(*fw_info);
	size += sizeof(struct vboot_component_entry) * fw_info->num_components;

	for (i = 0; i < fw_info->num_components; i++)
		size += ALIGN(fw_info->entries[i].size, sizeof(uint32_t));

	/* Check that size of comopnents does not exceed the region's size. */
	if (size > r->size)
		return -1;

	/* Update region with the correct size. */
	r->size = size;

	return 0;
}

static int vboot_fill_params(struct vboot_context *ctx)
{
	VbCommonParams *cparams;
	VbSelectFirmwareParams *fparams;

	if (fw_region_size(&ctx->fw_a))
		return -1;

	if (fw_region_size(&ctx->fw_b))
		return -1;

	cparams = ctx->cparams;
	fparams = ctx->fparams;

	cparams->gbb_size = ctx->gbb.size;
	fparams->verification_size_A = ctx->vblock_a.size;
	fparams->verification_size_B = ctx->vblock_b.size;

	if (IS_ENABLED(CONFIG_SPI_FLASH_MEMORY_MAPPED)) {
		/* Get memory-mapped pointers to the regions. */
		cparams->gbb_data = vboot_get_region(ctx->gbb.offset_addr,
		                                     ctx->gbb.size, NULL);
		fparams->verification_block_A =
			vboot_get_region(ctx->vblock_a.offset_addr,
			                 ctx->vblock_a.size, NULL);
		fparams->verification_block_B =
			vboot_get_region(ctx->vblock_b.offset_addr,
			                 ctx->vblock_b.size, NULL);
	} else {
		/*
		 * Copy the vblock info into a buffer in cbmem. The gbb will
		 * be read using VbExRegionRead().
		 */
		char *dest;
		size_t vblck_sz;

		vblck_sz = ctx->vblock_a.size + ctx->vblock_b.size;
		ctx->vblocks = cbmem_entry_add(TEMP_CBMEM_ID_VBLOCKS, vblck_sz);
		if (ctx->vblocks == NULL)
			return -1;
		dest = cbmem_entry_start(ctx->vblocks);
		if (vboot_get_region(ctx->vblock_a.offset_addr,
		                     ctx->vblock_a.size, dest) == NULL)
			return -1;
		fparams->verification_block_A = (void *)dest;
		dest += ctx->vblock_a.size;
		if (vboot_get_region(ctx->vblock_b.offset_addr,
		                     ctx->vblock_b.size, dest) == NULL)
			return -1;
		fparams->verification_block_B = (void *)dest;
	}

	return 0;
}

static void fill_handoff(struct vboot_context *context)
{
	struct vboot_components *fw_info;
	struct vboot_region *region;
	int i;

	/* Fix up the handoff structure. */
	context->handoff->selected_firmware =
		context->fparams->selected_firmware;

	/* Parse out the components for downstream consumption. */
	if (context->handoff->selected_firmware == VB_SELECT_FIRMWARE_A)
		region = &context->fw_a;
	else if  (context->handoff->selected_firmware == VB_SELECT_FIRMWARE_B)
		region = &context->fw_b;
	else
		return;

	fw_info = vboot_locate_components(region);
	if (fw_info == NULL)
		return;

	for (i = 0; i < fw_info->num_components; i++) {
		context->handoff->components[i].address =
			region->offset_addr + fw_info->entries[i].offset;
		context->handoff->components[i].size = fw_info->entries[i].size;
	}
}

static void vboot_clean_up(struct vboot_context *context)
{
	if (context->vblocks != NULL)
		cbmem_entry_remove(context->vblocks);
}

static void vboot_invoke_wrapper(struct vboot_handoff *vboot_handoff)
{
	VbCommonParams cparams;
	VbSelectFirmwareParams fparams;
	struct vboot_context context;
	uint32_t *iflags;

	vboot_handoff->selected_firmware = VB_SELECT_FIRMWARE_READONLY;

	memset(&cparams, 0, sizeof(cparams));
	memset(&fparams, 0, sizeof(fparams));
	memset(&context, 0, sizeof(context));

	iflags = &vboot_handoff->init_params.flags;
	if (get_developer_mode_switch())
		*iflags |= VB_INIT_FLAG_DEV_SWITCH_ON;
	if (get_recovery_mode_switch()) {
		clear_recovery_mode_switch();
		*iflags |= VB_INIT_FLAG_REC_BUTTON_PRESSED;
	}
	if (get_write_protect_state())
		*iflags |= VB_INIT_FLAG_WP_ENABLED;
	if (vboot_get_sw_write_protect())
		*iflags |= VB_INIT_FLAG_SW_WP_ENABLED;
	if (CONFIG_VIRTUAL_DEV_SWITCH)
		*iflags |= VB_INIT_FLAG_VIRTUAL_DEV_SWITCH;
	if (CONFIG_EC_SOFTWARE_SYNC)
		*iflags |= VB_INIT_FLAG_EC_SOFTWARE_SYNC;
	if (!CONFIG_PHYSICAL_REC_SWITCH)
		*iflags |= VB_INIT_FLAG_VIRTUAL_REC_SWITCH;
	if (CONFIG_VBOOT_EC_SLOW_UPDATE)
		*iflags |= VB_INIT_FLAG_EC_SLOW_UPDATE;
	if (CONFIG_VBOOT_OPROM_MATTERS) {
		*iflags |= VB_INIT_FLAG_OPROM_MATTERS;
		*iflags |= VB_INIT_FLAG_BEFORE_OPROM_LOAD;
		/* Will load VGA option rom during this boot */
		if (developer_mode_enabled() || recovery_mode_enabled() ||
		    vboot_wants_oprom()) {
			*iflags |= VB_INIT_FLAG_OPROM_LOADED;
		}
	}

	context.handoff = vboot_handoff;
	context.cparams = &cparams;
	context.fparams = &fparams;

	cparams.shared_data_blob = &vboot_handoff->shared_data[0];
	cparams.shared_data_size = VB_SHARED_DATA_MIN_SIZE;
	cparams.caller_context = &context;

	vboot_locate_region("GBB", &context.gbb);
	vboot_locate_region("VBLOCK_A", &context.vblock_a);
	vboot_locate_region("VBLOCK_B", &context.vblock_b);
	vboot_locate_region("FW_MAIN_A", &context.fw_a);
	vboot_locate_region("FW_MAIN_B", &context.fw_b);

	/* Check all fmap entries. */
	if (context.fw_a.size < 0 || context.fw_b.size < 0 ||
	    context.vblock_a.size < 0 || context.vblock_b.size < 0 ||
	    context.gbb.size < 0) {
		printk(BIOS_DEBUG, "Not all fmap entries found for vboot.\n");
		return;
	}

	/* Fill in vboot parameters. */
	if (vboot_fill_params(&context)) {
		vboot_clean_up(&context);
		return;
	}

	/* Initialize callbacks. */
	context.read_vbnv = &read_vbnv;
	context.save_vbnv = &save_vbnv;
	context.tis_init = &tis_init;
	context.tis_open = &tis_open;
	context.tis_close = &tis_close;
	context.tis_sendrecv = &tis_sendrecv;
	context.log_msg = &log_msg;
	context.fatal_error = &fatal_error;
	context.get_region = &vboot_get_region;
	context.reset = &vboot_reboot;

	vboot_run_stub(&context);

	fill_handoff(&context);

	vboot_clean_up(&context);
}

#if CONFIG_RELOCATABLE_RAMSTAGE
static void *vboot_load_ramstage(struct vboot_handoff *vboot_handoff,
                                 struct romstage_handoff *handoff)
{
	struct cbfs_stage *stage;
	const struct firmware_component *fwc;
	struct rmod_stage_load rmod_load = {
		.cbmem_id = CBMEM_ID_RAMSTAGE,
		.name = CONFIG_CBFS_PREFIX "/ramstage",
	};

	if (CONFIG_VBOOT_RAMSTAGE_INDEX >= MAX_PARSED_FW_COMPONENTS) {
		printk(BIOS_ERR, "Invalid ramstage index: %d\n",
		       CONFIG_VBOOT_RAMSTAGE_INDEX);
		return NULL;
	}

	/* Check for invalid address. */
	fwc = &vboot_handoff->components[CONFIG_VBOOT_RAMSTAGE_INDEX];
	if (fwc->address == 0) {
		printk(BIOS_DEBUG, "RW ramstage image address invalid.\n");
		return NULL;
	}

	printk(BIOS_DEBUG, "RW ramstage image at 0x%08x, 0x%08x bytes.\n",
	       fwc->address, fwc->size);

	stage = (void *)fwc->address;

	timestamp_add_now(TS_START_COPYRAM);

	if (rmodule_stage_load(&rmod_load, stage)) {
		vboot_handoff->selected_firmware = VB_SELECT_FIRMWARE_READONLY;
		printk(BIOS_DEBUG, "Could not load ramstage region.\n");
		return NULL;
	}

	cache_loaded_ramstage(handoff, rmod_load.cbmem_entry, rmod_load.entry);

	timestamp_add_now(TS_END_COPYRAM);

	return rmod_load.entry;
}
#else /* CONFIG_RELOCATABLE_RAMSTAGE */
static void *vboot_load_ramstage(struct vboot_handoff *vboot_handoff,
                                 struct romstage_handoff *handoff)
{
	struct cbfs_stage *stage;
	const struct firmware_component *fwc;
	void *entry;
	void *load;

	if (CONFIG_VBOOT_RAMSTAGE_INDEX >= MAX_PARSED_FW_COMPONENTS) {
		printk(BIOS_ERR, "Invalid ramstage index: %d\n",
		       CONFIG_VBOOT_RAMSTAGE_INDEX);
		return NULL;
	}

	/* Check for invalid address. */
	fwc = &vboot_handoff->components[CONFIG_VBOOT_RAMSTAGE_INDEX];
	if (fwc->address == 0) {
		printk(BIOS_DEBUG, "RW ramstage image address invalid.\n");
		return NULL;
	}

	printk(BIOS_DEBUG, "RW ramstage image at 0x%08x, 0x%08x bytes.\n",
	       fwc->address, fwc->size);

	/* This will leak a mapping. */
	stage = vboot_get_region(fwc->address, fwc->size, NULL);

	if (stage == NULL) {
		printk(BIOS_DEBUG, "Unable to get RW ramstage region.\n");
		return NULL;
	}

	load = (void *)(uintptr_t)stage->load;
	entry = (void *)(uintptr_t)stage->entry;

	timestamp_add_now(TS_START_COPYRAM);

	/* Stages rely the below clearing so that the bss is initialized. */
	memset(load, 0, stage->memlen);

	if (cbfs_decompress(stage->compression,
			     ((unsigned char *) stage) +
			     sizeof(struct cbfs_stage),
			     load, stage->len))
		return NULL;

	timestamp_add_now(TS_END_COPYRAM);

	return entry;
}
#endif /* CONFIG_RELOCATABLE_RAMSTAGE */


void *vboot_verify_firmware_get_entry(struct romstage_handoff *handoff)
{
	struct vboot_handoff *vboot_handoff;

	/* Don't go down verified boot path on S3 resume. */
	if (handoff != NULL && handoff->s3_resume)
		return NULL;

	timestamp_add_now(TS_START_VBOOT);

	vboot_handoff = cbmem_add(CBMEM_ID_VBOOT_HANDOFF,
	                          sizeof(*vboot_handoff));

	if (vboot_handoff == NULL) {
		printk(BIOS_DEBUG, "Could not add vboot_handoff structure.\n");
		return NULL;
	}

	memset(vboot_handoff, 0, sizeof(*vboot_handoff));

	vboot_invoke_wrapper(vboot_handoff);

	timestamp_add_now(TS_END_VBOOT);

	/* Take RO firmware path since no RW area was selected. */
	if (vboot_handoff->selected_firmware != VB_SELECT_FIRMWARE_A &&
	    vboot_handoff->selected_firmware != VB_SELECT_FIRMWARE_B) {
		printk(BIOS_DEBUG, "No RW firmware selected: 0x%08x\n",
		       vboot_handoff->selected_firmware);
		return NULL;
	}

	/* Load ramstage from the vboot_handoff structure. */
	return vboot_load_ramstage(vboot_handoff, handoff);
}

void vboot_verify_firmware(struct romstage_handoff *handoff)
{
	void *entry = vboot_verify_firmware_get_entry(handoff);

	if (entry != NULL)
		stage_exit(entry);
}
