bootblock-y += bootblock.c
bootblock-y += bootblock_asm.S
bootblock-y += cbfs.c
bootblock-y += maincpu.S
bootblock-y += monotonic_timer.c
bootblock-y += timer.c
ifeq ($(CONFIG_BOOTBLOCK_CONSOLE),y)
bootblock-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c
endif

verstage-y += verstage.c
verstage-y += cbfs.c
verstage-y += monotonic_timer.c
verstage-y += timer.c

romstage-y += cbfs.c
romstage-y += cbmem.c
romstage-y += monotonic_timer.c
romstage-y += timer.c
ifeq ($(CONFIG_EARLY_CONSOLE),y)
romstage-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c
endif

ramstage-y += cbfs.c
ramstage-y += cbmem.c
ramstage-y += maincpu.S
ramstage-y += monotonic_timer.c
ramstage-y += soc.c
ramstage-y += timer.c
ramstage-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c

rmodules_$(ARCH-ROMSTAGE-y)-y += monotonic_timer.c
VBOOT_STUB_DEPS += $(obj)/soc/marvell/armada38x/monotonic_timer.rmodules_$(ARCH-ROMSTAGE-y).o

INCLUDES += -Isrc/soc/marvell/armada38x/include/

$(objcbfs)/bootblock.raw.elf: $(objcbfs)/bootblock.elf
	cp $< $@

BIN_HDR = $(src)/soc/marvell/armada38x/bin_header/bin_hdr.bin
BIN_HDR_UART = $(src)/soc/marvell/armada38x/bin_header/bin_hdr.uart.bin
CB_SPI = $(obj)/generated/coreboot.spi.rom
CB_NAND = $(obj)/generated/coreboot.nand.rom
CB_UART = $(obj)/generated/coreboot.uart.rom
CB_UART_PLUS = $(obj)/generated/coreboot.uart.plus.rom
CB_UART_PLUS_KERNEL = $(obj)/generated/coreboot.uart.plus.kernel.rom
CB_UIMAGE = $(obj)/generated/coreboot_uimage
CB_UIMAGE_SPI = $(obj)/generated/coreboot_uimage.spi.rom
DOIMAGE = $(src)/soc/marvell/armada38x/bin_header/doimage
UIMAGE = $(src)/soc/marvell/armada38x/bin_header/uImage.armada
UBOOT = $(src)/soc/marvell/armada38x/bin_header/u-boot.bin
bootable: coreboot
	echo "Generating SPI bootable image"
	rm -rf $(CB_SPI)
	$(DOIMAGE) -T flash -D 0 -E 0 -G $(BIN_HDR) $(obj)/coreboot.rom $(CB_SPI)
	echo "Generating NAND bootable image"
	rm -rf $(CB_NAND)
	$(DOIMAGE) -T nand -D 0 -E 0 -P 4096 -L 128 -N MLC -G $(BIN_HDR) $(obj)/coreboot.rom $(CB_NAND)
	echo "Generating UART bootable image"
	rm -rf $(CB_UART)
	$(DOIMAGE) -T uart -D 0 -E 0 -G $(BIN_HDR_UART) $(obj)/coreboot.rom $(CB_UART)
	echo "Generating UART+ bootable image"
	rm -rf $(CB_UART_PLUS)
	$(DOIMAGE) -T uart -D 0 -E 0 -s $(CB_SPI) -G $(BIN_HDR_UART) $(UBOOT) $(CB_UART_PLUS)
	echo "Generating UART+KERNEL bootable image"
	rm -rf $(CB_UART_PLUS_KERNEL) $(CB_UIMAGE) $(CB_UIMAGE_SPI)
	cat $(obj)/coreboot.rom $(UIMAGE) > $(CB_UIMAGE)
	$(DOIMAGE) -T flash -D 0 -E 0 -G $(BIN_HDR) $(CB_UIMAGE) $(CB_UIMAGE_SPI)
	$(DOIMAGE) -T uart -D 0 -E 0 -s $(CB_UIMAGE_SPI) -G $(BIN_HDR_UART) $(UBOOT) $(CB_UART_PLUS_KERNEL)
