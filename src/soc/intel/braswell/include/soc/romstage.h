/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013 Google Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _BRASWELL_ROMSTAGE_H_
#define _BRASWELL_ROMSTAGE_H_

#include <stdint.h>
#include <arch/cpu.h>
#include <fsp_util.h>
#include <soc/pei_data.h>
#include <soc/pm.h>
#include <soc/intel/common/romstage.h>

void gfx_init(void);
void tco_disable(void);
void punit_init(void);
int early_spi_read_wpsr(u8 *sr);
void mainboard_fill_spd_data(struct pei_data *pei_data);

/* romstage_common.c functions */
void program_base_addresses(void);
void migrate_power_state(void);
struct chipset_power_state *fill_power_state(void);
int chipset_prev_sleep_state(struct chipset_power_state *ps);

#endif /* _BRASWELL_ROMSTAGE_H_ */
