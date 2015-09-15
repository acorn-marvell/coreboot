/*
 * This file is part of the coreboot project.
 *
 * Copyright 2015 Marvell ltd.
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

#include <gpio.h>
#include <boot/coreboot_tables.h>
#include <console/console.h>
#include <ec/google/chromeec/ec.h>
#include <ec/google/chromeec/ec_commands.h>
#include <string.h>
#include <vendorcode/google/chromeos/chromeos.h>

#define DEV_SW 5
#define DEV_POL ACTIVE_LOW
#define REC_SW 11
#define REC_POL ACTIVE_LOW
#define WP_SW  43
#define WP_POL ACTIVE_LOW

void fill_lb_gpios(struct lb_gpios *gpios)
{
        struct lb_gpio *gpio;
        const int GPIO_COUNT = 5;

        gpios->size = sizeof(*gpios) + (GPIO_COUNT * sizeof(struct lb_gpio));
        gpios->count = GPIO_COUNT;

        gpio = gpios->gpios;
        fill_lb_gpio(gpio++, DEV_SW, ACTIVE_LOW, "developer", gpio_get(DEV_SW));
        fill_lb_gpio(gpio++, REC_SW, ACTIVE_LOW, "recovery", gpio_get(REC_SW));
        fill_lb_gpio(gpio++, WP_SW, ACTIVE_LOW, "write protect", gpio_get(WP_SW));
        fill_lb_gpio(gpio++, -1, ACTIVE_LOW, "power", 1);
        fill_lb_gpio(gpio++, -1, ACTIVE_LOW, "lid", 0);
}

int get_developer_mode_switch(void)
{
	return gpio_get(DEV_SW) ^ !WP_POL;
}

int get_recovery_mode_switch(void)
{
	return gpio_get(REC_SW) ^ !WP_POL;
}

int get_write_protect_state(void)
{
	return gpio_get(WP_SW) ^ !WP_POL;
}
