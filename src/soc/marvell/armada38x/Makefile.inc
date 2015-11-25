bootblock-y += bootblock.c
bootblock-y += bootblock_asm.S
bootblock-y += spi.c
bootblock-y += i2c.c
bootblock-y += gpio.c
bootblock-y += maincpu.S
bootblock-y += monotonic_timer.c
bootblock-y += timer.c
ifeq ($(CONFIG_BOOTBLOCK_CONSOLE),y)
bootblock-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c
endif

verstage-y += uart.c
verstage-y += monotonic_timer.c
verstage-y += timer.c
verstage-y += spi.c
verstage-y += gpio.c
verstage-y += i2c.c

romstage-y += uart.c
romstage-y += spi.c
romstage-y += gpio.c
romstage-y += cbmem.c
romstage-y += monotonic_timer.c
romstage-y += timer.c
ifeq ($(CONFIG_EARLY_CONSOLE),y)
romstage-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c
endif

ramstage-y += uart.c
ramstage-y += spi.c
ramstage-y += gpio.c
ramstage-y += cbmem.c
ramstage-y += maincpu.S
ramstage-y += monotonic_timer.c
ramstage-y += soc.c
ramstage-y += timer.c
ramstage-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c

rmodules_$(ARCH-ROMSTAGE-y)-y += monotonic_timer.c
VBOOT_STUB_DEPS += $(obj)/soc/marvell/armada38x/monotonic_timer.rmodules_$(ARCH-ROMSTAGE-y).o

INCLUDES += -Isrc/soc/marvell/armada38x/include/

BIN_HDR = $(src)/soc/marvell/armada38x/bin_header/bin_hdr.bin
DOIMAGE = $(src)/soc/marvell/armada38x/bin_header/doimage

$(objcbfs)/bootblock.raw: $(objcbfs)/bootblock.elf
	@printf "    OBJCOPY    $(subst $(obj)/,,$(@))\n"
	$(OBJCOPY_bootblock) -O binary $< $@

$(objcbfs)/bootblock.bin: $(objcbfs)/bootblock.raw
	$(DOIMAGE) -T flash -D 0 -E 0 -S $(CONFIG_MRVL_SPI_COREBOOT_OFFSET) -G $(BIN_HDR) $< $@
	rm $<
