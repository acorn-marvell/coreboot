/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2009 Samsung Electronics
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

#include <uart.h>
#include <arch/io.h>
#include <console/console.h>	/* for __console definition */
#include <stdint.h>
#include <uart8250.h>

struct armada385_uart {
	union {
		uint32_t thr; // Transmit holding register.
		uint32_t rbr; // Receive buffer register.
		uint32_t dll; // Divisor latch lsb.
	};
	union {
		uint32_t ier; // Interrupt enable register.
		uint32_t dlm; // Divisor latch msb.
	};
	union {
		uint32_t iir; // Interrupt identification register.
		uint32_t fcr; // FIFO control register.
	};
	uint32_t lcr; // Line control register.
	uint32_t mcr; // Modem control register.
	uint32_t lsr; // Line status register.
	uint32_t msr; // Modem status register.
} __attribute__ ((packed));

static struct armada385_uart * const uart_ptr =
	(void *)CONFIG_CONSOLE_SERIAL_UART_ADDRESS;

static void armada385_uart_tx_flush(void);
static int armada385_uart_tst_byte(void);

static void armada385_uart_init(void)
{
	/* Use hard coded divisor. 108 for tclk 200mhz and 135 for tclk 250mhz */
	const unsigned divisor = 108; /*135*/
	const uint8_t line_config = UART8250_LCR_WLS_8; // 8n1
	armada385_uart_tx_flush();
	/* Disable interrupts. */
	write8(&uart_ptr->ier, 0);
	/* Set line configuration, access divisor latches. */
	write8(&uart_ptr->lcr, UART8250_LCR_DLAB);

	/* Set the divisor. */
	write8(&uart_ptr->dll, divisor & 0xff);
	write8(&uart_ptr->dlm, (divisor >> 8) & 0xff);
	/* Hide the divisor latches. */
	write8(&uart_ptr->lcr, line_config);
	/* Enable FIFOs, and clear receive and transmit. */
	write8(&uart_ptr->fcr, UART8250_FCR_FIFO_EN |
		UART8250_FCR_CLEAR_RCVR |
		UART8250_FCR_CLEAR_XMIT);

}

static void armada385_uart_tx_byte(unsigned char data)
{
	while (!(read8(&uart_ptr->lsr) & UART8250_LSR_THRE));
	write8(&uart_ptr->thr, data);
}

static void armada385_uart_tx_flush(void)
{
	while (!(read8(&uart_ptr->lsr) & UART8250_LSR_TEMT));
}

static unsigned char armada385_uart_rx_byte(void)
{
	if (!armada385_uart_tst_byte())
		return 0;
	return read8(&uart_ptr->rbr);
}

static int armada385_uart_tst_byte(void)
{
	return (read8(&uart_ptr->lsr) & UART8250_LSR_DR) == UART8250_LSR_DR;
}

#if !defined(__PRE_RAM__)

static const struct console_driver armada385_uart_console __console = {
	.init     = armada385_uart_init,
	.tx_byte  = armada385_uart_tx_byte,
	.tx_flush = armada385_uart_tx_flush,
	.rx_byte  = armada385_uart_rx_byte,
	.tst_byte = armada385_uart_tst_byte,
};

uint32_t uartmem_getbaseaddr(void)
{
	return (uintptr_t)uart_ptr;
}

#else

void uart_init(void)
{
	armada385_uart_init();
}

void uart_tx_byte(unsigned char data)
{
	armada385_uart_tx_byte(data);
}

void uart_tx_flush(void)
{
	armada385_uart_tx_flush();
}

unsigned char uart_rx_byte(void)
{
	return armada385_uart_rx_byte();
}

#endif
