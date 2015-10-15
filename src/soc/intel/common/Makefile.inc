ifeq ($(CONFIG_SOC_INTEL_COMMON),y)

verstage-$(CONFIG_SOC_INTEL_COMMON_RESET) += reset.c

romstage-$(CONFIG_CACHE_MRC_SETTINGS) += mrc_cache.c
romstage-$(CONFIG_SOC_INTEL_COMMON_RESET) += reset.c
romstage-y += util.c

ramstage-y += hda_verb.c
ramstage-$(CONFIG_CACHE_MRC_SETTINGS) += mrc_cache.c
ramstage-$(CONFIG_CACHE_MRC_SETTINGS) += nvm.c
ramstage-$(CONFIG_SOC_INTEL_COMMON_RESET) += reset.c
ramstage-y += util.c
ramstage-$(CONFIG_SOC_INTEL_COMMON_ACPI_WAKE_SOURCE) += acpi_wake_source.c

endif
