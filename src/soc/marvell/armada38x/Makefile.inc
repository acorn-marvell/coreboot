bootblock-y += bootblock.c
bootblock-y += bootblock_asm.S
bootblock-y += spi.c
bootblock-y += gpio.c
bootblock-y += maincpu.S
bootblock-y += monotonic_timer.c
bootblock-y += timer.c
ifeq ($(CONFIG_BOOTBLOCK_CONSOLE),y)
bootblock-$(CONFIG_DRIVERS_UART) += uart.c
endif

verstage-$(CONFIG_DRIVERS_UART) += uart.c
verstage-y += monotonic_timer.c
verstage-y += timer.c
verstage-y += spi.c
verstage-y += gpio.c
verstage-y += i2c.c

romstage-y += spi.c
romstage-y += gpio.c
romstage-y += cbmem.c
romstage-y += monotonic_timer.c
romstage-y += timer.c
romstage-$(CONFIG_DRIVERS_UART) += uart.c

ramstage-y += spi.c
ramstage-y += gpio.c
ramstage-y += cbmem.c
ramstage-y += maincpu.S
ramstage-y += monotonic_timer.c
ramstage-y += soc.c
ramstage-y += timer.c
ramstage-$(CONFIG_DRIVERS_UART) += uart.c

rmodules_$(ARCH-ROMSTAGE-y)-y += monotonic_timer.c
VBOOT_STUB_DEPS += $(obj)/soc/marvell/armada38x/monotonic_timer.rmodules_$(ARCH-ROMSTAGE-y).o

CPPFLAGS_common += -Isrc/soc/marvell/armada38x/include/

$(objcbfs)/bootblock.raw.elf: $(objcbfs)/bootblock.elf
	cp $< $@

BIN_HDR = $(src)/soc/marvell/armada38x/bin_header/bin_hdr
CB_SPI = $(obj)/generated/coreboot.spi.rom
CB_2M = $(obj)/generated/coreboot_2M.rom
DOIMAGE = $(src)/soc/marvell/armada38x/bin_header/doimage
bootable: coreboot
	echo "Generating SPI bootable image"
	rm -rf $(CB_SPI)
	dd if=$(obj)/coreboot.rom of=$(CB_2M) bs=1024 count=2000 conv=notrunc
	$(DOIMAGE) -T flash -D 0 -E 0 -S $(CONFIG_MRVL_SPI_COREBOOT_OFFSET) -G $(BIN_HDR) $(CB_2M) $(CB_SPI)
