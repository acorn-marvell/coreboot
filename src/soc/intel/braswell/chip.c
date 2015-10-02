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
 * Foundation, Inc.
 */

#include <chip.h>
#include <console/console.h>
#include <device/device.h>
#include <device/pci.h>
#include <fsp/util.h>
#include <soc/pci_devs.h>
#include <soc/ramstage.h>

static void pci_domain_set_resources(device_t dev)
{
	printk(BIOS_SPEW, "%s/%s ( %s )\n",
			__FILE__, __func__, dev_name(dev));
	assign_resources(dev->link_list);
}

static struct device_operations pci_domain_ops = {
	.read_resources   = pci_domain_read_resources,
	.set_resources    = pci_domain_set_resources,
	.enable_resources = NULL,
	.init             = NULL,
	.scan_bus         = pci_domain_scan_bus,
	.ops_pci_bus      = pci_bus_default_ops,
};

static void cpu_bus_noop(device_t dev) { }

static struct device_operations cpu_bus_ops = {
	.read_resources   = cpu_bus_noop,
	.set_resources    = cpu_bus_noop,
	.enable_resources = cpu_bus_noop,
	.init             = soc_init_cpus
};


static void enable_dev(device_t dev)
{
	printk(BIOS_SPEW, "----------\n%s/%s ( %s ), type: %d\n",
			__FILE__, __func__,
			dev_name(dev), dev->path.type);
	printk(BIOS_SPEW, "vendor: 0x%04x. device: 0x%04x\n",
			pci_read_config16(dev, PCI_VENDOR_ID),
			pci_read_config16(dev, PCI_DEVICE_ID));
	printk(BIOS_SPEW, "class: 0x%02x %s\n"
			"subclass: 0x%02x %s\n"
			"prog: 0x%02x\n"
			"revision: 0x%02x\n",
			pci_read_config16(dev, PCI_CLASS_DEVICE) >> 8,
			get_pci_class_name(dev),
			pci_read_config16(dev, PCI_CLASS_DEVICE) & 0xff,
			get_pci_subclass_name(dev),
			pci_read_config8(dev, PCI_CLASS_PROG),
			pci_read_config8(dev, PCI_REVISION_ID));

	/* Set the operations if it is a special bus type */
	if (dev->path.type == DEVICE_PATH_DOMAIN) {
		dev->ops = &pci_domain_ops;
	} else if (dev->path.type == DEVICE_PATH_CPU_CLUSTER) {
		dev->ops = &cpu_bus_ops;
	} else if (dev->path.type == DEVICE_PATH_PCI) {
		/* Handle south cluster enablement. */
		if (PCI_SLOT(dev->path.pci.devfn) > GFX_DEV &&
		    (dev->ops == NULL || dev->ops->enable == NULL)) {
			southcluster_enable_dev(dev);
		}
	}
}

void soc_silicon_init_params(SILICON_INIT_UPD *params)
{
	device_t dev = dev_find_slot(0, PCI_DEVFN(LPC_DEV, LPC_FUNC));
	struct soc_intel_braswell_config *config = dev->chip_info;

	/* Set the parameters for SiliconInit */
	printk(BIOS_DEBUG, "Updating UPD values for SiliconInit\n");
	params->PcdSdcardMode = config->PcdSdcardMode;
	params->PcdEnableHsuart0 = config->PcdEnableHsuart0;
	params->PcdEnableHsuart1 = config->PcdEnableHsuart1;
	params->PcdEnableAzalia = config->PcdEnableAzalia;
	params->PcdEnableSata = config->PcdEnableSata;
	params->PcdEnableXhci = config->PcdEnableXhci;
	params->PcdEnableLpe = config->PcdEnableLpe;
	params->PcdEnableDma0 = config->PcdEnableDma0;
	params->PcdEnableDma1 = config->PcdEnableDma1;
	params->PcdEnableI2C0 = config->PcdEnableI2C0;
	params->PcdEnableI2C1 = config->PcdEnableI2C1;
	params->PcdEnableI2C2 = config->PcdEnableI2C2;
	params->PcdEnableI2C3 = config->PcdEnableI2C3;
	params->PcdEnableI2C4 = config->PcdEnableI2C4;
	params->PcdEnableI2C5 = config->PcdEnableI2C5;
	params->PcdEnableI2C6 = config->PcdEnableI2C6;
	params->GraphicsConfigPtr = 0;
	params->AzaliaConfigPtr = 0;
	params->PunitPwrConfigDisable = config->PunitPwrConfigDisable;
	params->ChvSvidConfig = config->ChvSvidConfig;
	params->DptfDisable = config->DptfDisable;
	params->PcdEmmcMode = config->PcdEmmcMode;
	params->PcdUsb3ClkSsc = config->PcdUsb3ClkSsc;
	params->PcdDispClkSsc = config->PcdDispClkSsc;
	params->PcdSataClkSsc = config->PcdSataClkSsc;
	params->Usb2Port0PerPortPeTxiSet = config->Usb2Port0PerPortPeTxiSet;
	params->Usb2Port0PerPortTxiSet = config->Usb2Port0PerPortTxiSet;
	params->Usb2Port0IUsbTxEmphasisEn = config->Usb2Port0IUsbTxEmphasisEn;
	params->Usb2Port0PerPortTxPeHalf = config->Usb2Port0PerPortTxPeHalf;
	params->Usb2Port1PerPortPeTxiSet = config->Usb2Port1PerPortPeTxiSet;
	params->Usb2Port1PerPortTxiSet = config->Usb2Port1PerPortTxiSet;
	params->Usb2Port1IUsbTxEmphasisEn = config->Usb2Port1IUsbTxEmphasisEn;
	params->Usb2Port1PerPortTxPeHalf = config->Usb2Port1PerPortTxPeHalf;
	params->Usb2Port2PerPortPeTxiSet = config->Usb2Port2PerPortPeTxiSet;
	params->Usb2Port2PerPortTxiSet = config->Usb2Port2PerPortTxiSet;
	params->Usb2Port2IUsbTxEmphasisEn = config->Usb2Port2IUsbTxEmphasisEn;
	params->Usb2Port2PerPortTxPeHalf = config->Usb2Port2PerPortTxPeHalf;
	params->Usb2Port3PerPortPeTxiSet = config->Usb2Port3PerPortPeTxiSet;
	params->Usb2Port3PerPortTxiSet = config->Usb2Port3PerPortTxiSet;
	params->Usb2Port3IUsbTxEmphasisEn = config->Usb2Port3IUsbTxEmphasisEn;
	params->Usb2Port3PerPortTxPeHalf = config->Usb2Port3PerPortTxPeHalf;
	params->Usb2Port4PerPortPeTxiSet = config->Usb2Port4PerPortPeTxiSet;
	params->Usb2Port4PerPortTxiSet = config->Usb2Port4PerPortTxiSet;
	params->Usb2Port4IUsbTxEmphasisEn = config->Usb2Port4IUsbTxEmphasisEn;
	params->Usb2Port4PerPortTxPeHalf = config->Usb2Port4PerPortTxPeHalf;
	params->Usb3Lane0Ow2tapgen2deemph3p5 =
		config->Usb3Lane0Ow2tapgen2deemph3p5;
	params->Usb3Lane1Ow2tapgen2deemph3p5 =
		config->Usb3Lane1Ow2tapgen2deemph3p5;
	params->Usb3Lane2Ow2tapgen2deemph3p5 =
		config->Usb3Lane2Ow2tapgen2deemph3p5;
	params->Usb3Lane3Ow2tapgen2deemph3p5 =
		config->Usb3Lane3Ow2tapgen2deemph3p5;
	params->PcdSataInterfaceSpeed = config->PcdSataInterfaceSpeed;
	params->PcdPchUsbSsicPort = config->PcdPchUsbSsicPort;
	params->PcdPchUsbHsicPort = config->PcdPchUsbHsicPort;
	params->PcdPcieRootPortSpeed = config->PcdPcieRootPortSpeed;
	params->PcdPchSsicEnable = config->PcdPchSsicEnable;
	params->PcdLogoPtr = config->PcdLogoPtr;
	params->PcdLogoSize = config->PcdLogoSize;
	params->PcdRtcLock = config->PcdRtcLock;
	params->PMIC_I2CBus = config->PMIC_I2CBus;
	params->ISPEnable = config->ISPEnable;
	params->ISPPciDevConfig = config->ISPPciDevConfig;
}

void soc_display_silicon_init_params(const SILICON_INIT_UPD *old,
	SILICON_INIT_UPD *new)
{
	/* Display the parameters for SiliconInit */
	printk(BIOS_SPEW, "UPD values for SiliconInit:\n");
	soc_display_upd_value("PcdSdcardMode", 1, old->PcdSdcardMode,
		new->PcdSdcardMode);
	soc_display_upd_value("PcdEnableHsuart0", 1, old->PcdEnableHsuart0,
		new->PcdEnableHsuart0);
	soc_display_upd_value("PcdEnableHsuart1", 1, old->PcdEnableHsuart1,
		new->PcdEnableHsuart1);
	soc_display_upd_value("PcdEnableAzalia", 1, old->PcdEnableAzalia,
		new->PcdEnableAzalia);
	soc_display_upd_value("AzaliaConfigPtr", 4,
			(uint32_t)old->AzaliaConfigPtr,
			(uint32_t)new->AzaliaConfigPtr);
	soc_display_upd_value("PcdEnableSata", 1, old->PcdEnableSata,
		new->PcdEnableSata);
	soc_display_upd_value("PcdEnableXhci", 1, old->PcdEnableXhci,
		new->PcdEnableXhci);
	soc_display_upd_value("PcdEnableLpe", 1, old->PcdEnableLpe,
		new->PcdEnableLpe);
	soc_display_upd_value("PcdEnableDma0", 1, old->PcdEnableDma0,
		new->PcdEnableDma0);
	soc_display_upd_value("PcdEnableDma1", 1, old->PcdEnableDma1,
		new->PcdEnableDma1);
	soc_display_upd_value("PcdEnableI2C0", 1, old->PcdEnableI2C0,
		new->PcdEnableI2C0);
	soc_display_upd_value("PcdEnableI2C1", 1, old->PcdEnableI2C1,
		new->PcdEnableI2C1);
	soc_display_upd_value("PcdEnableI2C2", 1, old->PcdEnableI2C2,
		new->PcdEnableI2C2);
	soc_display_upd_value("PcdEnableI2C3", 1, old->PcdEnableI2C3,
		new->PcdEnableI2C3);
	soc_display_upd_value("PcdEnableI2C4", 1, old->PcdEnableI2C4,
		new->PcdEnableI2C4);
	soc_display_upd_value("PcdEnableI2C5", 1, old->PcdEnableI2C5,
		new->PcdEnableI2C5);
	soc_display_upd_value("PcdEnableI2C6", 1, old->PcdEnableI2C6,
		new->PcdEnableI2C6);
	soc_display_upd_value("PcdGraphicsConfigPtr", 4,
		old->GraphicsConfigPtr, new->GraphicsConfigPtr);
	soc_display_upd_value("GpioFamilyInitTablePtr", 4,
		(uint32_t)old->GpioFamilyInitTablePtr,
		(uint32_t)new->GpioFamilyInitTablePtr);
	soc_display_upd_value("GpioPadInitTablePtr", 4,
		(uint32_t)old->GpioPadInitTablePtr,
		(uint32_t)new->GpioPadInitTablePtr);
	soc_display_upd_value("PunitPwrConfigDisable", 1,
		old->PunitPwrConfigDisable,
		new->PunitPwrConfigDisable);
	soc_display_upd_value("ChvSvidConfig", 1, old->ChvSvidConfig,
		new->ChvSvidConfig);
	soc_display_upd_value("DptfDisable", 1, old->DptfDisable,
		new->DptfDisable);
	soc_display_upd_value("PcdEmmcMode", 1, old->PcdEmmcMode,
		new->PcdEmmcMode);
	soc_display_upd_value("PcdUsb3ClkSsc", 1, old->PcdUsb3ClkSsc,
		new->PcdUsb3ClkSsc);
	soc_display_upd_value("PcdDispClkSsc", 1, old->PcdDispClkSsc,
		new->PcdDispClkSsc);
	soc_display_upd_value("PcdSataClkSsc", 1, old->PcdSataClkSsc,
		new->PcdSataClkSsc);
	soc_display_upd_value("Usb2Port0PerPortPeTxiSet", 1,
		old->Usb2Port0PerPortPeTxiSet,
		new->Usb2Port0PerPortPeTxiSet);
	soc_display_upd_value("Usb2Port0PerPortTxiSet", 1,
		old->Usb2Port0PerPortTxiSet,
		new->Usb2Port0PerPortTxiSet);
	soc_display_upd_value("Usb2Port0IUsbTxEmphasisEn", 1,
		old->Usb2Port0IUsbTxEmphasisEn,
		new->Usb2Port0IUsbTxEmphasisEn);
	soc_display_upd_value("Usb2Port0PerPortTxPeHalf", 1,
		old->Usb2Port0PerPortTxPeHalf,
		new->Usb2Port0PerPortTxPeHalf);
	soc_display_upd_value("Usb2Port1PerPortPeTxiSet", 1,
		old->Usb2Port1PerPortPeTxiSet,
		new->Usb2Port1PerPortPeTxiSet);
	soc_display_upd_value("Usb2Port1PerPortTxiSet", 1,
		old->Usb2Port1PerPortTxiSet,
		new->Usb2Port1PerPortTxiSet);
	soc_display_upd_value("Usb2Port1IUsbTxEmphasisEn", 1,
		old->Usb2Port1IUsbTxEmphasisEn,
		new->Usb2Port1IUsbTxEmphasisEn);
	soc_display_upd_value("Usb2Port1PerPortTxPeHalf", 1,
		old->Usb2Port1PerPortTxPeHalf,
		new->Usb2Port1PerPortTxPeHalf);
	soc_display_upd_value("Usb2Port2PerPortPeTxiSet", 1,
		old->Usb2Port2PerPortPeTxiSet,
		new->Usb2Port2PerPortPeTxiSet);
	soc_display_upd_value("Usb2Port2PerPortTxiSet", 1,
		old->Usb2Port2PerPortTxiSet,
		new->Usb2Port2PerPortTxiSet);
	soc_display_upd_value("Usb2Port2IUsbTxEmphasisEn", 1,
		old->Usb2Port2IUsbTxEmphasisEn,
		new->Usb2Port2IUsbTxEmphasisEn);
	soc_display_upd_value("Usb2Port2PerPortTxPeHalf", 1,
		old->Usb2Port2PerPortTxPeHalf,
		new->Usb2Port2PerPortTxPeHalf);
	soc_display_upd_value("Usb2Port3PerPortPeTxiSet", 1,
		old->Usb2Port3PerPortPeTxiSet,
		new->Usb2Port3PerPortPeTxiSet);
	soc_display_upd_value("Usb2Port3PerPortTxiSet", 1,
		old->Usb2Port3PerPortTxiSet,
		new->Usb2Port3PerPortTxiSet);
	soc_display_upd_value("Usb2Port3IUsbTxEmphasisEn", 1,
		old->Usb2Port3IUsbTxEmphasisEn,
		new->Usb2Port3IUsbTxEmphasisEn);
	soc_display_upd_value("Usb2Port3PerPortTxPeHalf", 1,
		old->Usb2Port3PerPortTxPeHalf,
		new->Usb2Port3PerPortTxPeHalf);
	soc_display_upd_value("Usb2Port4PerPortPeTxiSet", 1,
		old->Usb2Port4PerPortPeTxiSet,
		new->Usb2Port4PerPortPeTxiSet);
	soc_display_upd_value("Usb2Port4PerPortTxiSet", 1,
		old->Usb2Port4PerPortTxiSet,
		new->Usb2Port4PerPortTxiSet);
	soc_display_upd_value("Usb2Port4IUsbTxEmphasisEn", 1,
		old->Usb2Port4IUsbTxEmphasisEn,
		new->Usb2Port4IUsbTxEmphasisEn);
	soc_display_upd_value("Usb2Port4PerPortTxPeHalf", 1,
		old->Usb2Port4PerPortTxPeHalf,
		new->Usb2Port4PerPortTxPeHalf);
	soc_display_upd_value("Usb3Lane0Ow2tapgen2deemph3p5", 1,
		old->Usb3Lane0Ow2tapgen2deemph3p5,
		new->Usb3Lane0Ow2tapgen2deemph3p5);
	soc_display_upd_value("Usb3Lane1Ow2tapgen2deemph3p5", 1,
		old->Usb3Lane1Ow2tapgen2deemph3p5,
		new->Usb3Lane1Ow2tapgen2deemph3p5);
	soc_display_upd_value("Usb3Lane2Ow2tapgen2deemph3p5", 1,
		old->Usb3Lane2Ow2tapgen2deemph3p5,
		new->Usb3Lane2Ow2tapgen2deemph3p5);
	soc_display_upd_value("Usb3Lane3Ow2tapgen2deemph3p5", 1,
		old->Usb3Lane3Ow2tapgen2deemph3p5,
		new->Usb3Lane3Ow2tapgen2deemph3p5);
	soc_display_upd_value("PcdSataInterfaceSpeed", 1,
		old->PcdSataInterfaceSpeed,
		new->PcdSataInterfaceSpeed);
	soc_display_upd_value("PcdPchUsbSsicPort", 1,
		old->PcdPchUsbSsicPort, new->PcdPchUsbSsicPort);
	soc_display_upd_value("PcdPchUsbHsicPort", 1,
		old->PcdPchUsbHsicPort, new->PcdPchUsbHsicPort);
	soc_display_upd_value("PcdPcieRootPortSpeed", 1,
		old->PcdPcieRootPortSpeed, new->PcdPcieRootPortSpeed);
	soc_display_upd_value("PcdPchSsicEnable", 1, old->PcdPchSsicEnable,
		new->PcdPchSsicEnable);
	soc_display_upd_value("PcdLogoPtr", 4, old->PcdLogoPtr,
		new->PcdLogoPtr);
	soc_display_upd_value("PcdLogoSize", 4, old->PcdLogoSize,
		new->PcdLogoSize);
	soc_display_upd_value("PcdRtcLock", 1, old->PcdRtcLock,
		new->PcdRtcLock);
	soc_display_upd_value("PMIC_I2CBus", 1,
		old->PMIC_I2CBus, new->PMIC_I2CBus);
	soc_display_upd_value("ISPEnable", 1,
		old->ISPEnable, new->ISPEnable);
	soc_display_upd_value("ISPPciDevConfig", 1,
		old->ISPPciDevConfig, new->ISPPciDevConfig);
}

/* Called at BS_DEV_INIT_CHIPS time -- very early. Just after BS_PRE_DEVICE. */
static void soc_init(void *chip_info)
{
	printk(BIOS_SPEW, "%s/%s\n", __FILE__, __func__);
	soc_init_pre_device(chip_info);
}

struct chip_operations soc_intel_braswell_ops = {
	CHIP_NAME("Intel Braswell SoC")
	.enable_dev = enable_dev,
	.init = soc_init,
};

static void pci_set_subsystem(device_t dev, unsigned vendor, unsigned device)
{
	printk(BIOS_SPEW, "%s/%s ( %s, 0x%04x, 0x%04x )\n",
			__FILE__, __func__, dev_name(dev), vendor, device);
	if (!vendor || !device) {
		pci_write_config32(dev, PCI_SUBSYSTEM_VENDOR_ID,
				pci_read_config32(dev, PCI_VENDOR_ID));
	} else {
		pci_write_config32(dev, PCI_SUBSYSTEM_VENDOR_ID,
				((device & 0xffff) << 16) | (vendor & 0xffff));
	}
}

struct pci_operations soc_pci_ops = {
	.set_subsystem = &pci_set_subsystem,
};
