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

#include <arch/io.h>
#include <soc/pci_devs.h>
#include <soc/ramstage.h>
#include <soc/sata.h>
#include <console/console.h>
#include <delay.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>

#include "chip.h"

typedef struct soc_intel_braswell_config config_t;

static void sata_init(struct device *dev)
{
	printk(BIOS_SPEW, "%s/%s ( %s )\n",
			__FILE__, __func__, dev_name(dev));
}

static void sata_enable(device_t dev)
{
	southcluster_enable_dev(dev);
}

static struct device_operations sata_ops = {
	.read_resources		= pci_dev_read_resources,
	.set_resources		= pci_dev_set_resources,
	.enable_resources	= pci_dev_enable_resources,
	.init			= sata_init,
	.enable			= sata_enable,
	.ops_pci		= &soc_pci_ops,
};

static const unsigned short pci_device_ids[] = {
	AHCI1_DEVID,	/* AHCI */
	0,
};

static const struct pci_driver braswell_sata __pci_driver = {
	.ops     = &sata_ops,
	.vendor  = PCI_VENDOR_ID_INTEL,
	.devices = pci_device_ids,
};
