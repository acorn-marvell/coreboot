##
## This file is part of the libpayload project.
##
## Copyright (C) 2007 Uwe Hermann <uwe@hermann-uwe.de>
## Copyright (C) 2008 Advanced Micro Devices, Inc.
## Copyright (C) 2008 coresystems GmbH
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
## 1. Redistributions of source code must retain the above copyright
##    notice, this list of conditions and the following disclaimer.
## 2. Redistributions in binary form must reproduce the above copyright
##    notice, this list of conditions and the following disclaimer in the
##    documentation and/or other materials provided with the distribution.
## 3. The name of the author may not be used to endorse or promote products
##    derived from this software without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
## ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
## FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
## DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
## OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
## HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
## LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
## OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
## SUCH DAMAGE.
##

libc-$(CONFIG_LP_PCI) += pci.c

libc-$(CONFIG_LP_SPEAKER) += speaker.c

libc-$(CONFIG_LP_8250_SERIAL_CONSOLE) += serial/8250.c
libc-$(CONFIG_LP_S5P_SERIAL_CONSOLE) += serial/s5p.c
libc-$(CONFIG_LP_8250_MMIO32_SERIAL_CONSOLE) += serial/8250_mmio32.c
libc-$(CONFIG_LP_IPQ806X_SERIAL_CONSOLE) += serial/ipq806x.c
libc-$(CONFIG_LP_BG4CD_SERIAL_CONSOLE) += serial/bg4cd.c
libc-$(CONFIG_LP_MRVL_SERIAL_CONSOLE) += serial/mrvl_serial.c
libc-$(CONFIG_LP_PC_KEYBOARD) += keyboard.c

libc-$(CONFIG_LP_CBMEM_CONSOLE) += cbmem_console.c

libc-$(CONFIG_LP_NVRAM) += nvram.c
libc-$(CONFIG_LP_NVRAM) += options.c

# Timer drivers
libc-$(CONFIG_LP_TIMER_MCT) += timer/mct.c
libc-$(CONFIG_LP_TIMER_RDTSC) += timer/rdtsc.c
libc-$(CONFIG_LP_TIMER_TEGRA_1US) += timer/tegra_1us.c
libc-$(CONFIG_LP_TIMER_IPQ806X) += timer/ipq806x.c
libc-$(CONFIG_LP_TIMER_RK) += timer/rktimer.c
libc-$(CONFIG_LP_TIMER_BG4CD) += timer/bg4cd.c
libc-$(CONFIG_LP_TIMER_IMG_PISTACHIO) += timer/img_pistachio.c
libc-$(CONFIG_LP_TIMER_CYGNUS) += timer/cygnus.c
libc-$(CONFIG_LP_TIMER_MRVL_25MHZ) += timer/mrvl_25mhz.c

# Video console drivers
libc-$(CONFIG_LP_VIDEO_CONSOLE) += video/video.c
libc-$(CONFIG_LP_VGA_VIDEO_CONSOLE) += video/vga.c

# Geode LX console drivers
libc-$(CONFIG_LP_GEODELX_VIDEO_CONSOLE) += video/geodelx.c
libc-$(CONFIG_LP_GEODELX_VIDEO_CONSOLE) += video/font8x16.c

# coreboot generic framebuffer driver
libc-$(CONFIG_LP_COREBOOT_VIDEO_CONSOLE) += video/corebootfb.c
libc-$(CONFIG_LP_COREBOOT_VIDEO_CONSOLE) += video/font8x16.c

libc-$(CONFIG_LP_STORAGE) += storage/storage.c
libc-$(CONFIG_LP_STORAGE_ATA) += storage/ata.c
libc-$(CONFIG_LP_STORAGE_ATAPI) += storage/atapi.c
libc-$(CONFIG_LP_STORAGE_AHCI) += storage/ahci.c

# USB stack
libc-$(CONFIG_LP_USB) += usb/usbinit.c
libc-$(CONFIG_LP_USB) += usb/usb.c
libc-$(CONFIG_LP_USB) += usb/usb_dev.c
libc-$(CONFIG_LP_USB) += usb/quirks.c
libc-$(CONFIG_LP_USB_GEN_HUB) += usb/generic_hub.c
libc-$(CONFIG_LP_USB_HUB) += usb/usbhub.c
libc-$(CONFIG_LP_USB_UHCI) += usb/uhci.c
libc-$(CONFIG_LP_USB_UHCI) += usb/uhci_rh.c
libc-$(CONFIG_LP_USB_OHCI) += usb/ohci.c
libc-$(CONFIG_LP_USB_OHCI) += usb/ohci_rh.c
libc-$(CONFIG_LP_USB_EHCI) += usb/ehci.c
libc-$(CONFIG_LP_USB_EHCI) += usb/ehci_rh.c
libc-$(CONFIG_LP_USB_XHCI) += usb/xhci.c
libc-$(CONFIG_LP_USB_XHCI) += usb/xhci_debug.c
libc-$(CONFIG_LP_USB_XHCI) += usb/xhci_devconf.c
libc-$(CONFIG_LP_USB_XHCI) += usb/xhci_events.c
libc-$(CONFIG_LP_USB_XHCI) += usb/xhci_commands.c
libc-$(CONFIG_LP_USB_XHCI) += usb/xhci_rh.c
libc-$(CONFIG_LP_USB_HID) += usb/usbhid.c
libc-$(CONFIG_LP_USB_MSC) += usb/usbmsc.c
libc-$(CONFIG_LP_USB_DWC2) += usb/dwc2.c
libc-$(CONFIG_LP_USB_DWC2) += usb/dwc2_rh.c

# USB device stack
libc-$(CONFIG_LP_UDC) += udc/udc.c
libc-$(CONFIG_LP_UDC_CI) += udc/chipidea.c

# used by both USB HID and keyboard
libc-y += hid.c
