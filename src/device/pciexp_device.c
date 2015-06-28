/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2005 Linux Networx
 * (Written by Eric Biederman <ebiederman@lnxi.com> for Linux Networx)
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

#include <console/console.h>
#include <delay.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <device/pciexp.h>

unsigned int pciexp_find_extended_cap(device_t dev, unsigned int cap)
{
	unsigned int this_cap_offset, next_cap_offset;
	unsigned int this_cap, cafe;

	this_cap_offset = PCIE_EXT_CAP_OFFSET;
	do {
		this_cap = pci_mmio_read_config32(dev, this_cap_offset);
		next_cap_offset = this_cap >> 20;
		this_cap &= 0xffff;
		cafe = pci_mmio_read_config32(dev, this_cap_offset + 4);
		cafe &= 0xffff;
		if (this_cap == cap)
			return this_cap_offset;
		else if (cafe == cap)
			return this_cap_offset + 4;
		else
			this_cap_offset = next_cap_offset;
	} while (next_cap_offset != 0);

	return 0;
}

#if CONFIG_PCIEXP_COMMON_CLOCK
/*
 * Re-train a PCIe link
 */
#define PCIE_TRAIN_RETRY 10000
static int pciexp_retrain_link(device_t dev, unsigned cap)
{
	unsigned try = PCIE_TRAIN_RETRY;
	u16 lnk;

	/* Start link retraining */
	lnk = pci_read_config16(dev, cap + PCI_EXP_LNKCTL);
	lnk |= PCI_EXP_LNKCTL_RL;
	pci_write_config16(dev, cap + PCI_EXP_LNKCTL, lnk);

	/* Wait for training to complete */
	while (try--) {
		lnk = pci_read_config16(dev, cap + PCI_EXP_LNKSTA);
		if (!(lnk & PCI_EXP_LNKSTA_LT))
			return 0;
		udelay(100);
	}

	printk(BIOS_ERR, "%s: Link Retrain timeout\n", dev_path(dev));
	return -1;
}

/*
 * Check the Slot Clock Configuration for root port and endpoint
 * and enable Common Clock Configuration if possible.  If CCC is
 * enabled the link must be retrained.
 */
static void pciexp_enable_common_clock(device_t root, unsigned root_cap,
				       device_t endp, unsigned endp_cap)
{
	u16 root_scc, endp_scc, lnkctl;

	/* Get Slot Clock Configuration for root port */
	root_scc = pci_read_config16(root, root_cap + PCI_EXP_LNKSTA);
	root_scc &= PCI_EXP_LNKSTA_SLC;

	/* Get Slot Clock Configuration for endpoint */
	endp_scc = pci_read_config16(endp, endp_cap + PCI_EXP_LNKSTA);
	endp_scc &= PCI_EXP_LNKSTA_SLC;

	/* Enable Common Clock Configuration and retrain */
	if (root_scc && endp_scc) {
		printk(BIOS_INFO, "Enabling Common Clock Configuration\n");

		/* Set in endpoint */
		lnkctl = pci_read_config16(endp, endp_cap + PCI_EXP_LNKCTL);
		lnkctl |= PCI_EXP_LNKCTL_CCC;
		pci_write_config16(endp, endp_cap + PCI_EXP_LNKCTL, lnkctl);

		/* Set in root port */
		lnkctl = pci_read_config16(root, root_cap + PCI_EXP_LNKCTL);
		lnkctl |= PCI_EXP_LNKCTL_CCC;
		pci_write_config16(root, root_cap + PCI_EXP_LNKCTL, lnkctl);

		/* Retrain link if CCC was enabled */
		pciexp_retrain_link(root, root_cap);
	}
}
#endif /* CONFIG_PCIEXP_COMMON_CLOCK */

#if CONFIG_PCIEXP_CLK_PM
static void pciexp_enable_clock_power_pm(device_t endp, unsigned endp_cap)
{
	/* check if per port clk req is supported in device */
	u32 endp_ca;
	u16 lnkctl;
	endp_ca = pci_read_config32(endp, endp_cap + PCI_EXP_LNKCAP);
	if ((endp_ca & PCI_EXP_CLK_PM) == 0) {
		printk(BIOS_INFO, "PCIE CLK PM is not supported by endpoint");
		return;
	}
	lnkctl = pci_read_config16(endp, endp_cap + PCI_EXP_LNKCTL);
	lnkctl = lnkctl | PCI_EXP_EN_CLK_PM;
	pci_write_config16(endp, endp_cap + PCI_EXP_LNKCTL, lnkctl);
}
#endif /* CONFIG_PCIEXP_CLK_PM */

#if CONFIG_PCIEXP_L1_SUB_STATE
static void pcie_update_cfg(device_t dev, int reg, u32 mask, u32 or)
{
	u32 reg32;

	reg32 = pci_mmio_read_config32(dev, reg);
	reg32 &= mask;
	reg32 |= or;
	pci_mmio_write_config32(dev, reg, reg32);
}

static void pciexp_config_max_latency(device_t root, device_t dev)
{
	unsigned int cap;
	cap = pciexp_find_extended_cap(dev, PCIE_EXT_CAP_LTR_ID);
	if ((cap) && (root->ops->ops_pci != NULL) &&
		(root->ops->ops_pci->set_L1_ss_latency != NULL))
			root->ops->ops_pci->set_L1_ss_latency(dev, cap + 4);
}

static void pciexp_enable_ltr(device_t dev)
{
	unsigned int cap;
	cap = pci_find_capability(dev, PCI_CAP_ID_PCIE);
	if(!cap) {
		printk(BIOS_INFO, "Failed to enable LTR for dev = %s\n",
			dev_path(dev));
		return;
	}
	pcie_update_cfg(dev, cap + 0x28, ~(1 << 10), 1 << 10);
}

static unsigned char pciexp_L1_substate_cal(device_t dev, unsigned int endp_cap,
	unsigned int *data)
{
	unsigned char mult[4] = {2, 10, 100, 0};

	unsigned int L1SubStateSupport = *data & 0xf;
	unsigned int comm_mode_rst_time = (*data >> 8) & 0xff;
	unsigned int power_on_scale = (*data >> 16) & 0x3;
	unsigned int power_on_value = (*data >> 19) & 0x1f;

	unsigned int endp_data = pci_mmio_read_config32(dev, endp_cap + 4);
	unsigned int endp_L1SubStateSupport = endp_data & 0xf;
	unsigned int endp_comm_mode_restore_time = (endp_data >> 8) & 0xff;
	unsigned int endp_power_on_scale = (endp_data >> 16) & 0x3;
	unsigned int endp_power_on_value = (endp_data >> 19) & 0x1f;

	L1SubStateSupport &= endp_L1SubStateSupport;

	if (L1SubStateSupport == 0)
		return 0;

	if (power_on_value * mult[power_on_scale] <
		endp_power_on_value * mult[endp_power_on_scale]) {
		power_on_value = endp_power_on_value;
		power_on_scale = endp_power_on_scale;
	}
	if (comm_mode_rst_time < endp_comm_mode_restore_time)
		comm_mode_rst_time = endp_comm_mode_restore_time;

	*data = (comm_mode_rst_time << 8) | (power_on_scale << 16)
		| (power_on_value << 19) | L1SubStateSupport;

	return 1;
}

static void pciexp_L1_substate_commit(device_t root, device_t dev,
	unsigned int root_cap, unsigned int end_cap)
{
	device_t dev_t;
	unsigned char L1_ss_ok;
	unsigned int rp_L1_support = pci_mmio_read_config32(root, root_cap + 4);
	unsigned int L1SubStateSupport;
	unsigned int comm_mode_rst_time;
	unsigned int power_on_scale;
	unsigned int endp_power_on_value;

	for (dev_t = dev; dev_t; dev_t = dev_t->sibling) {
		/*
		 * rp_L1_support is init'd above from root port.
		 * it needs coordination with endpoints to reach in common.
		 * if certain endpoint doesn't support L1 Sub-State, abort
		 * this feature enabling.
		 */
		L1_ss_ok = pciexp_L1_substate_cal(dev_t, end_cap,
						&rp_L1_support);
		if (!L1_ss_ok)
			return;
	}

	L1SubStateSupport = rp_L1_support & 0xf;
	comm_mode_rst_time = (rp_L1_support >> 8) & 0xff;
	power_on_scale = (rp_L1_support >> 16) & 0x3;
	endp_power_on_value = (rp_L1_support >> 19) & 0x1f;

	printk(BIOS_INFO, "L1 Sub-State supported from root port %d\n",
		root->path.pci.devfn >> 3);
	printk(BIOS_INFO, "L1 Sub-State Support = 0x%x\n", L1SubStateSupport);
	printk(BIOS_INFO, "CommonModeRestoreTime = 0x%x\n", comm_mode_rst_time);
	printk(BIOS_INFO, "Power On Value = 0x%x, Power On Scale = 0x%x\n",
		endp_power_on_value, power_on_scale);

	pciexp_enable_ltr(root);

	pcie_update_cfg(root, root_cap + 0x08, ~0xff00,
		(comm_mode_rst_time << 8));

	pcie_update_cfg(root, root_cap + 0x0c , 0xffffff04,
		(endp_power_on_value << 3) | (power_on_scale));

	pcie_update_cfg(root, root_cap + 0x08, ~0xe3ff0000,
		(1 << 21) | (1 << 23) | (1 << 30));

	pcie_update_cfg(root, root_cap + 0x08, ~0x1f,
		L1SubStateSupport);

	for (dev_t = dev; dev_t; dev_t = dev_t->sibling) {
		pcie_update_cfg(dev_t, end_cap + 0x0c , 0xffffff04,
			(endp_power_on_value << 3) | (power_on_scale));

		pcie_update_cfg(dev_t, end_cap + 0x08, ~0xe3ff0000,
			(1 << 21) | (1 << 23) | (1 << 30));

		pcie_update_cfg(dev_t, end_cap + 0x08, ~0x1f,
			L1SubStateSupport);

		pciexp_enable_ltr(dev_t);

		pciexp_config_max_latency(root, dev_t);
	}
}

static void pciexp_config_L1_sub_state(device_t root, device_t dev)
{
	unsigned int root_cap, end_cap;

	/* Do it for function 0 only */
	if (dev->path.pci.devfn & 0x7)
		return;

	root_cap = pciexp_find_extended_cap(root, PCIE_EXT_CAP_L1SS_ID);
	if (!root_cap)
		return;

	end_cap = pciexp_find_extended_cap(dev, PCIE_EXT_CAP_L1SS_ID);
	if (!end_cap) {
		end_cap = pciexp_find_extended_cap(dev, 0xcafe);
		if (!end_cap)
			return;
	}

	pciexp_L1_substate_commit(root, dev, root_cap, end_cap);
}
#endif /* CONFIG_PCIEXP_L1_SUB_STATE */

#if CONFIG_PCIEXP_ASPM
/*
 * Determine the ASPM L0s or L1 exit latency for a link
 * by checking both root port and endpoint and returning
 * the highest latency value.
 */
static int pciexp_aspm_latency(device_t root, unsigned root_cap,
			       device_t endp, unsigned endp_cap,
			       enum aspm_type type)
{
	int root_lat = 0, endp_lat = 0;
	u32 root_lnkcap, endp_lnkcap;

	root_lnkcap = pci_read_config32(root, root_cap + PCI_EXP_LNKCAP);
	endp_lnkcap = pci_read_config32(endp, endp_cap + PCI_EXP_LNKCAP);

	/* Make sure the link supports this ASPM type by checking
	 * capability bits 11:10 with aspm_type offset by 1 */
	if (!(root_lnkcap & (1 << (type + 9))) ||
	    !(endp_lnkcap & (1 << (type + 9))))
		return -1;

	/* Find the one with higher latency */
	switch (type) {
	case PCIE_ASPM_L0S:
		root_lat = (root_lnkcap & PCI_EXP_LNKCAP_L0SEL) >> 12;
		endp_lat = (endp_lnkcap & PCI_EXP_LNKCAP_L0SEL) >> 12;
		break;
	case PCIE_ASPM_L1:
		root_lat = (root_lnkcap & PCI_EXP_LNKCAP_L1EL) >> 15;
		endp_lat = (endp_lnkcap & PCI_EXP_LNKCAP_L1EL) >> 15;
		break;
	default:
		return -1;
	}

	return (endp_lat > root_lat) ? endp_lat : root_lat;
}

/*
 * Enable ASPM on PCIe root port and endpoint.
 *
 * Returns APMC value:
 *   -1 = Error
 *    0 = no ASPM
 *    1 = L0s Enabled
 *    2 = L1 Enabled
 *    3 = L0s and L1 Enabled
 */
static enum aspm_type pciexp_enable_aspm(device_t root, unsigned root_cap,
					 device_t endp, unsigned endp_cap)
{
	const char *aspm_type_str[] = { "None", "L0s", "L1", "L0s and L1" };
	enum aspm_type apmc = PCIE_ASPM_NONE;
	int exit_latency, ok_latency;
	u16 lnkctl;
	u32 devcap;

	/* Get endpoint device capabilities for acceptable limits */
	devcap = pci_read_config32(endp, endp_cap + PCI_EXP_DEVCAP);

	/* Enable L0s if it is within endpoint acceptable limit */
	ok_latency = (devcap & PCI_EXP_DEVCAP_L0S) >> 6;
	exit_latency = pciexp_aspm_latency(root, root_cap, endp, endp_cap,
					   PCIE_ASPM_L0S);
	if (exit_latency >= 0 && exit_latency <= ok_latency)
		apmc |= PCIE_ASPM_L0S;

	/* Enable L1 if it is within endpoint acceptable limit */
	ok_latency = (devcap & PCI_EXP_DEVCAP_L1) >> 9;
	exit_latency = pciexp_aspm_latency(root, root_cap, endp, endp_cap,
					   PCIE_ASPM_L1);
	if (exit_latency >= 0 && exit_latency <= ok_latency)
		apmc |= PCIE_ASPM_L1;

	if (apmc != PCIE_ASPM_NONE) {
		/* Set APMC in root port first */
		lnkctl = pci_read_config16(root, root_cap + PCI_EXP_LNKCTL);
		lnkctl |= apmc;
		pci_write_config16(root, root_cap + PCI_EXP_LNKCTL, lnkctl);

		/* Set APMC in endpoint device next */
		lnkctl = pci_read_config16(endp, endp_cap + PCI_EXP_LNKCTL);
		lnkctl |= apmc;
		pci_write_config16(endp, endp_cap + PCI_EXP_LNKCTL, lnkctl);
	}

	printk(BIOS_INFO, "ASPM: Enabled %s\n", aspm_type_str[apmc]);
	return apmc;
}
#endif /* CONFIG_PCIEXP_ASPM */

static void pciexp_tune_dev(device_t dev)
{
	device_t root = dev->bus->dev;
	unsigned int root_cap, cap;

	cap = pci_find_capability(dev, PCI_CAP_ID_PCIE);
	if (!cap)
		return;

	root_cap = pci_find_capability(root, PCI_CAP_ID_PCIE);
	if (!root_cap)
		return;

#if CONFIG_PCIEXP_COMMON_CLOCK
	/* Check for and enable Common Clock */
	pciexp_enable_common_clock(root, root_cap, dev, cap);
#endif

#if CONFIG_PCIEXP_CLK_PM
	/* Check if per port CLK req is supported by endpoint*/
	pciexp_enable_clock_power_pm(dev, cap);
#endif

#if CONFIG_PCIEXP_L1_SUB_STATE
	/* Enable L1 Sub-State when both root port and endpoint support */
	pciexp_config_L1_sub_state(root, dev);
#endif /* CONFIG_PCIEXP_L1_SUB_STATE */

#if CONFIG_PCIEXP_ASPM
	/* Check for and enable ASPM */
	enum aspm_type apmc = pciexp_enable_aspm(root, root_cap, dev, cap);

	if (apmc != PCIE_ASPM_NONE) {
		/* Enable ASPM role based error reporting. */
		u32 reg32 = pci_read_config32(dev, cap + PCI_EXP_DEVCAP);
		reg32 |= PCI_EXP_DEVCAP_RBER;
		pci_write_config32(dev, cap + PCI_EXP_DEVCAP, reg32);
	}
#endif
}

unsigned int pciexp_scan_bus(struct bus *bus, unsigned int min_devfn,
			     unsigned int max_devfn, unsigned int max)
{
	device_t child;

	max = pci_scan_bus(bus, min_devfn, max_devfn, max);

	for (child = bus->children; child; child = child->sibling) {
		if ((child->path.pci.devfn < min_devfn) ||
		    (child->path.pci.devfn > max_devfn)) {
			continue;
		}
		pciexp_tune_dev(child);
	}
	return max;
}

unsigned int pciexp_scan_bridge(device_t dev, unsigned int max)
{
	return do_pci_scan_bridge(dev, max, pciexp_scan_bus);
}

/** Default device operations for PCI Express bridges */
static struct pci_operations pciexp_bus_ops_pci = {
	.set_subsystem = 0,
};

struct device_operations default_pciexp_ops_bus = {
	.read_resources   = pci_bus_read_resources,
	.set_resources    = pci_dev_set_resources,
	.enable_resources = pci_bus_enable_resources,
	.init             = 0,
	.scan_bus         = pciexp_scan_bridge,
	.enable           = 0,
	.reset_bus        = pci_bus_reset,
	.ops_pci          = &pciexp_bus_ops_pci,
};
