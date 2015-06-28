/*
 * This file is part of the coreboot project.
 *
 * Copyright 2015 Google Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA, 02110-1301 USA
 */

#ifndef __ARCH_ARM64_CORTEX_A57_H__
#define __ARCH_ARM64_CORTEX_A57_H__

#define CPUECTLR_EL1	S3_1_c15_c2_1
#define SMPEN_SHIFT	6

/* Cortex MIDR[15:4] PN */
#define CORTEX_A53_PN	0xd03

/* Double lock control bit */
#define OSDLR_DBL_LOCK_BIT	1

#endif /* __ARCH_ARM64_CORTEX_A57_H__ */
