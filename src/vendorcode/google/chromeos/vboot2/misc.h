/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2014 The ChromiumOS Authors.  All rights reserved.
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

#ifndef __CHROMEOS_VBOOT2_MISC_H__
#define __CHROMEOS_VBOOT2_MISC_H__

#include "../vboot_common.h"

struct vb2_context;
struct vb2_shared_data;

void vboot_fill_handoff(void);
void *vboot_load_stage(int stage_index,
		       struct region *fw_main,
		       struct vboot_components *fw_info);

void vb2_init_work_context(struct vb2_context *ctx);
struct vb2_shared_data *vb2_get_shared_data(void);

/* Returns 0 on success. < 0 on failure. */
int vb2_get_selected_region(struct region_device *rdev);
void vb2_set_selected_region(struct region_device *rdev);
int vboot_is_slot_selected(void);
int vboot_is_readonly_path(void);

#endif /* __CHROMEOS_VBOOT2_MISC_H__ */
