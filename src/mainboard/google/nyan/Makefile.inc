##
## This file is part of the coreboot project.
##
## Copyright 2014 Google Inc.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; version 2 of the License.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
##

# Add a handler for BCT config files
$(call add-special-class,bct-cfg)
bct-cfg-handler= $(eval $(obj)/generated/bct.cfg: $(1)$(2))

$(obj)/generated/bct.cfg:
	@printf "    CAT        $(subst $(obj)/,,$(@))\n"
	cat $^ > $@

subdirs-y += bct

bootblock-y += boardid.c
bootblock-y += bootblock.c
bootblock-y += pmic.c
bootblock-y += reset.c

romstage-y += reset.c
romstage-y += romstage.c
romstage-y += sdram_configs.c
romstage-y += early_configs.c

ramstage-y += reset.c
ramstage-y += boardid.c
ramstage-y += mainboard.c

bootblock-y += memlayout.ld
romstage-y += memlayout.ld
ramstage-y += memlayout.ld
