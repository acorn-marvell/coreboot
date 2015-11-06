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
 * Foundation, Inc.
 */

#include <arch/asm.h>

.arm
ENTRY(maincpu_setup)
	/*
	 * Set the cpu to System mode with IRQ and FIQ disabled. Prefetch/Data
	 * aborts may happen early and crash before the abort handlers are
	 * installed, but at least the problem will show up near the code that
	 * causes it.
	 */
	msr	cpsr_cxf, #0xdf

	ldr	sp, maincpu_stack_pointer
	eor	lr, lr
	ldr	r0, maincpu_entry_point
	bx	r0
ENDPROC(maincpu_setup)

	.align 2

	.global maincpu_stack_pointer
maincpu_stack_pointer:
	.word 0

	.global maincpu_entry_point
maincpu_entry_point:
	.word 0
