/*
 * Early initialization code for ARM architecture.
 *
 * This file is based off of the OMAP3530/ARM Cortex start.S file from Das
 * U-Boot, which itself got the file from armboot.
 *
 * Copyright (c) 2004	Texas Instruments <r-woodruff2@ti.com>
 * Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 * Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
 * Copyright (c) 2002	Gary Jennejohn <garyj@denx.de>
 * Copyright (c) 2003	Richard Woodruff <r-woodruff2@ti.com>
 * Copyright (c) 2003	Kshitij <kshitij@ti.com>
 * Copyright (c) 2006-2008 Syed Mohammed Khasim <x0khasim@ti.com>
 * Copyright (c) 2013   The Chromium OS Authors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <arch/asm.h>

ENTRY(_start)
	
	/* Disable L2 Cache */
        ldr r1, =0x0
        ldr r0, =0xD0008100
        str r1, [r0]

	/* Disable windows that overlap with 0xF1000000 */
        /* By default, window #12 overlaps with 0xF1000000 */
        mov  r1, #0
        ldr  r0, =0xD00200B0
        str  r1, [r0]
        /* Set Registers Base address. */
        ldr  r1, =0xf1000000
        ldr  r0, =0xD0020080
        str  r1, [r0]
	 /* Update SCU (peripheral) register Base address with correct INTER_REG_BASE */
        ldr r2, = 0xC000                /* SCU offset = 0xC000 */
        add r1, r1, r2                  /* r1 = INTER_REG_BASE + SCU_OFFSET */
        mcr p15, 4, r1, c15, c0, 0      /* Write SCU base register */
	/*
	 * Set the cpu to System mode with IRQ and FIQ disabled. Prefetch/Data
	 * aborts may happen early and crash before the abort handlers are
	 * installed, but at least the problem will show up near the code that
	 * causes it.
	 */
	msr	cpsr_cxf, #0xdf

	bl      cpu_init_cp15
        bl      cpu_init_crit

	/*
	 * Initialize the stack to a known value. This is used to check for
	 * stack overflow later in the boot process.
	 */
	ldr     r0, =_stack
        ldr     r1, =_estack
        ldr     r2, =0xdeadbeef
init_stack_loop:
        str     r2, [r0]
        add     r0, #4
        cmp     r0, r1
        bne     init_stack_loop

/* Set stackpointer in internal RAM to call bootblock main() */
call_bootblock:
	ldr	sp, =_estack /* Set up stack pointer */
	ldr	r0,=0x00000000
	 /*
	  * The current design of cpu_info places the
	  * struct at the top of the stack. The number of
	  * words pushed must be at least as large as that
	  * struct.
	  */
	push	{r0-r2}
	bic	sp, sp, #7 /* 8-byte alignment for ABI compliance */
	/*
	 * Use "bl" instead of "b" even though we do not intend to return.
	 * "bl" gets compiled to "blx" if we're transitioning from ARM to
	 * Thumb. However, "b" will not and GCC may attempt to create a
	 * wrapper which is currently broken.
	 */
	bl	main
ENDPROC(_start)


ENTRY(cpu_init_cp15)
        /*
         * Invalidate L1 I/D
         */
        mov     r0, #0                  @ set up for MCR
        mcr     p15, 0, r0, c8, c7, 0   @ invalidate TLBs
        mcr     p15, 0, r0, c7, c5, 0   @ invalidate icache
        mcr     p15, 0, r0, c7, c5, 6   @ invalidate BP array
        mcr     p15, 0, r0, c7, c10, 4  @ DSB
        mcr     p15, 0, r0, c7, c5, 4   @ ISB

        /*
         * disable MMU stuff and caches
         */
        mrc     p15, 0, r0, c1, c0, 0
        bic     r0, r0, #0x00002000     @ clear bits 13 (--V-)
        bic     r0, r0, #0x00000087     @ clear bits 7, 2:0 (B--- -CAM)
        bic     r0, r0, #0x00000007     @ clear bits 2:0 (-CAM)
        orr     r0, r0, #0x00000800     @ set bit 11 (Z---) BTB
        orr     r0, r0, #0x00001000     @ set bit 12 (I) I-cache
        mcr     p15, 0, r0, c1, c0, 0
        mov     pc, lr                  @ back to my caller
ENDPROC(cpu_init_cp15)

ENTRY(cpu_init_crit)
	mov     r2, lr

        /* Multicore support - read MPIDR register (Multiprocessor Affinity) */
        mrc     p15, 0, r0, c0, c0, 5

	mov     lr, r2
        mov     pc, lr
ENDPROC(cpu_init_crit)
