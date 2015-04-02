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
verstage-y += i2c.c

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
VBOOT_STUB_DEPS += $(obj)/soc/marvell/armada385/monotonic_timer.rmodules_$(ARCH-ROMSTAGE-y).o

INCLUDES += -Isrc/soc/marvell/armada385/include/

$(objcbfs)/bootblock.raw.elf: $(objcbfs)/bootblock.elf
	cp $< $@

BIN_HDR = $(src)/soc/marvell/armada385/bin_header/bin_hdr.bin
BIN_HDR_UART = $(src)/soc/marvell/armada385/bin_header/bin_hdr.uart.bin
CB_SPI = $(obj)/generated/coreboot.spi.rom
CB_NAND = $(obj)/generated/coreboot.nand.rom
CB_UART = $(obj)/generated/coreboot.uart.rom
DOIMAGE = $(src)/soc/marvell/armada385/bin_header/doimage
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
