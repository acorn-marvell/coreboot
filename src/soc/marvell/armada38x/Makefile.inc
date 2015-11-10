bootblock-y += bootblock.c
bootblock-y += bootblock_asm.S
bootblock-y += spi.c
bootblock-y += monotonic_timer.c
bootblock-y += timer.c
ifeq ($(CONFIG_BOOTBLOCK_CONSOLE),y)
bootblock-$(CONFIG_DRIVERS_UART) += uart.c
endif

verstage-$(CONFIG_DRIVERS_UART) += uart.c
verstage-y += monotonic_timer.c
verstage-y += timer.c
verstage-y += spi.c

romstage-y += spi.c
romstage-y += cbmem.c
romstage-y += monotonic_timer.c
romstage-y += timer.c
romstage-$(CONFIG_DRIVERS_UART) += uart.c

ramstage-y += spi.c
ramstage-y += cbmem.c
ramstage-y += monotonic_timer.c
ramstage-y += soc.c
ramstage-y += timer.c
ramstage-$(CONFIG_DRIVERS_UART) += uart.c

rmodules_$(ARCH-ROMSTAGE-y)-y += monotonic_timer.c
VBOOT_STUB_DEPS += $(obj)/soc/marvell/armada38x/monotonic_timer.rmodules_$(ARCH-ROMSTAGE-y).o

CPPFLAGS_common += -Isrc/soc/marvell/armada38x/include/

BIN_HDR = $(src)/soc/marvell/armada38x/bin_header/bin_hdr
DOIMAGE = $(src)/soc/marvell/armada38x/bin_header/doimage

$(objcbfs)/bootblock.raw: $(objcbfs)/bootblock.elf
	@printf "    OBJCOPY    $(subst $(obj)/,,$(@))\n"
	$(OBJCOPY_bootblock) -O binary $< $@

$(objcbfs)/bootblock.bin: $(objcbfs)/bootblock.raw
	$(DOIMAGE) -T flash -D 0 -E 0 -G $(BIN_HDR) $< $@
	rm $<
