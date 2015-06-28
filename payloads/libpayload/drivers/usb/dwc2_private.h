/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2014 Rockchip Electronics
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

#ifndef __DWC2_REGS_H__
#define __DWC2_REGS_H__
#include <usb/dwc2_registers.h>

typedef struct dwc_ctrl {
#define DMA_SIZE (64 * 1024)
	void *dma_buffer;
	uint32_t *hprt0;
} dwc_ctrl_t;

#define DWC2_INST(controller) ((dwc_ctrl_t *)((controller)->instance))
#define DWC2_REG(controller) ((dwc2_reg_t *)((controller)->reg_base))

typedef enum {
	HCSTAT_DONE = 0,
	HCSTAT_XFERERR,
	HCSTAT_BABBLE,
	HCSTAT_STALL,
	HCSTAT_UNKNOW,
	HCSTAT_TIMEOUT,
} hcstat_t;
#endif
