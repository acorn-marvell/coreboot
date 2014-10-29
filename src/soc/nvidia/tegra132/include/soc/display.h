/*
 * Copyright 2014 Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SOC_NVIDIA_TEGRA132_INCLUDE_SOC_DISPLAY_H__
#define __SOC_NVIDIA_TEGRA132_INCLUDE_SOC_DISPLAY_H__

/* ardisplay.h */
#define DC_CMD_DISPLAY_WINDOW_HEADER_0		0x42
#define DC_COM_CRC_CONTROL_0			0x300
#define DC_COM_CRC_CHECKSUM_0			0x301
#define DC_COM_PIN_OUTPUT_ENABLE0_0		0x302
#define DC_COM_PIN_OUTPUT_ENABLE1_0		0x303
#define DC_COM_PIN_OUTPUT_ENABLE2_0		0x304
#define DC_COM_PIN_OUTPUT_ENABLE3_0		0x305
#define DC_CMD_STATE_ACCESS_0			0x40
#define DC_DISP_DISP_CLOCK_CONTROL_0		0x42e
#define DC_DISP_DISP_TIMING_OPTIONS_0		0x405
#define DC_DISP_REF_TO_SYNC_0			0x406
#define DC_DISP_SYNC_WIDTH_0			0x407
#define DC_DISP_BACK_PORCH_0			0x408
#define DC_DISP_DISP_ACTIVE_0			0x409
#define DC_DISP_FRONT_PORCH_0			0x40a
#define DC_DISP_DISP_WIN_OPTIONS_0		0x402
#define DC_DISP_DISP_WIN_OPTIONS_0_SOR_ENABLE_SHIFT	25
#define DC_DISP_DISP_WIN_OPTIONS_0_SOR_ENABLE_FIELD	(0x1 << DC_DISP_DISP_WIN_OPTIONS_0_SOR_ENABLE_SHIFT)
#define DC_DISP_DISP_SIGNAL_OPTIONS0_0		0x400
#define DC_DISP_BLEND_BACKGROUND_COLOR_0	0x4e4
#define DC_CMD_DISPLAY_COMMAND_0		0x32
#define DC_CMD_STATE_CONTROL_0			0x41
#define DC_CMD_DISPLAY_POWER_CONTROL_0		0x36

/* ardisplay_a.h */
#define DC_WIN_A_WIN_OPTIONS_0			0x700
#define DC_WIN_A_WIN_OPTIONS_0_A_WIN_ENABLE_SHIFT	30
#define DC_WIN_A_WIN_OPTIONS_0_A_WIN_ENABLE_FIELD	(0x1 << DC_WIN_A_WIN_OPTIONS_0_A_WIN_ENABLE_SHIFT)
#define DC_WIN_A_WIN_OPTIONS_0_A_WIN_ENABLE_ENABLE	(1)
#define DC_WIN_A_BYTE_SWAP_0			0x701
#define DC_WIN_A_BUFFER_CONTROL_0		0x702
#define DC_WIN_A_COLOR_DEPTH_0			0x703
#define DC_WIN_A_POSITION_0			0x704
#define DC_WIN_A_SIZE_0				0x705
#define DC_WIN_A_PRESCALED_SIZE_0		0x706
#define DC_WIN_A_H_INITIAL_DDA_0		0x707
#define DC_WIN_A_V_INITIAL_DDA_0		0x708
#define DC_WIN_A_DDA_INCREMENT_0		0x709
#define DC_WIN_A_LINE_STRIDE_0			0x70a
#define DC_WIN_A_DV_CONTROL_0			0x70e
#define DC_WIN_A_BLEND_LAYER_CONTROL_0		0x716
#define DC_WIN_A_BLEND_MATCH_SELECT_0		0x717
#define DC_WIN_A_BLEND_NOMATCH_SELECT_0		0x718
#define DC_WIN_A_BLEND_ALPHA_1BIT_0		0x719
#define DC_WINBUF_A_START_ADDR_LO_0		0x800
#define DC_WINBUF_A_START_ADDR_HI_0		0x80d
#define DC_WINBUF_A_ADDR_H_OFFSET_0		0x806
#define DC_WINBUF_A_ADDR_V_OFFSET_0		0x808

/* ardisplay_bd.h */
#define DC_B_WIN_BD_SIZE_0			0xd85
#define DC_B_WIN_BD_PRESCALED_SIZE_0		0xd86
#define DC_B_WIN_BD_LINE_STRIDE_0		0xd8a
#define DC_B_WIN_BD_COLOR_DEPTH_0		0xd83
#define DC_B_WINBUF_BD_START_ADDR_0		0xdc0
#define DC_B_WIN_BD_DDA_INCREMENT_0		0xd89
#define DC_B_WIN_BD_WIN_OPTIONS_0		0xd80
#define DC_B_WIN_BD_WIN_OPTIONS_0_BD_WIN_ENABLE_SHIFT	30
#define DC_B_WIN_BD_WIN_OPTIONS_0_BD_WIN_ENABLE_FIELD	(0x1 << DC_B_WIN_BD_WIN_OPTIONS_0_BD_WIN_ENABLE_SHIFT)
#define DC_B_WIN_BD_WIN_OPTIONS_0_BD_WIN_ENABLE_ENABLE	(1)

/* arsor.h */
#define SOR_NV_PDISP_SOR_CLK_CNTRL_0		0x13
#define SOR_NV_PDISP_SOR_DP_PADCTL0_0		0x5c
#define SOR_NV_PDISP_SOR_PLL0_0			0x17
#define SOR_NV_PDISP_SOR_PLL1_0			0x18
#define SOR_NV_PDISP_SOR_PLL2_0			0x19
#define SOR_NV_PDISP_SOR_PLL3_0			0x1a
#define SOR_NV_PDISP_SOR_PLL2_0_AUX6_SHIFT	22
#define SOR_NV_PDISP_SOR_PLL2_0_AUX6_FIELD	(0x1 << SOR_NV_PDISP_SOR_PLL2_0_AUX6_SHIFT)
#define SOR_NV_PDISP_SOR_PLL0_0_PWR_SHIFT	0
#define SOR_NV_PDISP_SOR_PLL0_0_PWR_FIELD	(0x1 << SOR_NV_PDISP_SOR_PLL0_0_PWR_SHIFT)
#define SOR_NV_PDISP_SOR_PLL0_0_VCOPD_SHIFT	2
#define SOR_NV_PDISP_SOR_PLL0_0_VCOPD_FIELD	(0x1 << SOR_NV_PDISP_SOR_PLL0_0_VCOPD_SHIFT)
#define SOR_NV_PDISP_SOR_PLL2_0_AUX8_SHIFT	24
#define SOR_NV_PDISP_SOR_PLL2_0_AUX8_FIELD	(0x1 << SOR_NV_PDISP_SOR_PLL2_0_AUX8_SHIFT)
#define SOR_NV_PDISP_SOR_PLL2_0_AUX7_SHIFT	23
#define SOR_NV_PDISP_SOR_PLL2_0_AUX7_FIELD	(0x1 << SOR_NV_PDISP_SOR_PLL2_0_AUX7_SHIFT)
#define SOR_NV_PDISP_SOR_PLL2_0_AUX9_SHIFT	25
#define SOR_NV_PDISP_SOR_PLL2_0_AUX9_FIELD	(0x1 << SOR_NV_PDISP_SOR_PLL2_0_AUX9_SHIFT)
#define SOR_NV_PDISP_SOR_LANE_DRIVE_CURRENT0_0	0x4e
#define SOR_NV_PDISP_SOR_LANE_PREEMPHASIS0_0	0x52
#define SOR_NV_PDISP_SOR_POSTCURSOR0_0		0x56
#define SOR_NV_PDISP_SOR_DP_PADCTL0_0		0x5c
#define SOR_NV_PDISP_SOR_DP_PADCTL0_0_TX_PU_VALUE_SHIFT	8
#define SOR_NV_PDISP_SOR_DP_PADCTL0_0_TX_PU_VALUE_FIELD	(0xff << SOR_NV_PDISP_SOR_DP_PADCTL0_0_TX_PU_VALUE_SHIFT)
#define SOR_NV_PDISP_SOR_DP_PADCTL0_0_TX_PU_SHIFT	22
#define SOR_NV_PDISP_SOR_DP_PADCTL0_0_TX_PU_FIELD	(0x1 << SOR_NV_PDISP_SOR_DP_PADCTL0_0_TX_PU_SHIFT)
#define SOR_NV_PDISP_SOR_LVDS_0			0x1c
#define SOR_NV_PDISP_SOR_CLK_CNTRL_0		0x13
#define SOR_NV_PDISP_SOR_DP_LINKCTL0_0		0x4c
#define SOR_NV_PDISP_SOR_LANE_SEQ_CTL_0		0x21
#define SOR_NV_PDISP_SOR_DP_TPG_0		0x6d
#define SOR_NV_PDISP_HEAD_STATE1_0		0x7
#define SOR_NV_PDISP_HEAD_STATE2_0		0x9
#define SOR_NV_PDISP_HEAD_STATE3_0		0xb
#define SOR_NV_PDISP_HEAD_STATE4_0		0xd
#define SOR_NV_PDISP_SOR_STATE1_0		0x4
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_HSYNCPOL_SHIFT	12
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_HSYNCPOL_FIELD	(0x1 << SOR_NV_PDISP_SOR_STATE1_0_ASY_HSYNCPOL_SHIFT)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_VSYNCPOL_SHIFT	13
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_VSYNCPOL_FIELD	(0x1 << SOR_NV_PDISP_SOR_STATE1_0_ASY_VSYNCPOL_SHIFT)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_PROTOCOL_SHIFT	8
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_PROTOCOL_FIELD	(0xf << SOR_NV_PDISP_SOR_STATE1_0_ASY_PROTOCOL_SHIFT)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_PROTOCOL_LVDS_CUSTOM	(0)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_PROTOCOL_DP_A		(8)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_PROTOCOL_DP_B		(9)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_PROTOCOL_CUSTOM		(15)

#define SOR_NV_PDISP_SOR_STATE1_0_ASY_CRCMODE_ACTIVE_RASTER	(0)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_CRCMODE_COMPLETE_RASTER	(1)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_CRCMODE_NON_ACTIVE_RASTER	(2)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_CRCMODE_SHIFT		6
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_CRCMODE_FIELD		(0x3 << SOR_NV_PDISP_SOR_STATE1_0_ASY_CRCMODE_SHIFT)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_SUBOWNER_SHIFT		4
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_SUBOWNER_FIELD		(0x3 << SOR_NV_PDISP_SOR_STATE1_0_ASY_SUBOWNER_SHIFT)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_SUBOWNER_NONE		(0)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_SUBOWNER_SUBHEAD0		(1)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_SUBOWNER_SUBHEAD1		(2)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_SUBOWNER_BOTH		(3)

#define SOR_NV_PDISP_SOR_STATE1_0_ASY_OWNER_SHIFT		0
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_OWNER_FIELD		(0xf << SOR_NV_PDISP_SOR_STATE1_0_ASY_OWNER_SHIFT)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_OWNER_NONE		(0)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_OWNER_HEAD0		(1)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_OWNER_HEAD1		(2)

#define SOR_NV_PDISP_SOR_DP_CONFIG0_0				0x58
#define SOR_NV_PDISP_SOR_DP_CONFIG0_0_ACTIVESYM_POLARITY_SHIFT	24
#define SOR_NV_PDISP_SOR_DP_CONFIG0_0_ACTIVESYM_POLARITY_FIELD	(0x1 << SOR_NV_PDISP_SOR_DP_CONFIG0_0_ACTIVESYM_POLARITY_SHIFT)
#define SOR_NV_PDISP_SOR_DP_CONFIG0_0_ACTIVESYM_FRAC_SHIFT	16
#define SOR_NV_PDISP_SOR_DP_CONFIG0_0_ACTIVESYM_FRAC_FIELD	(0xf << SOR_NV_PDISP_SOR_DP_CONFIG0_0_ACTIVESYM_FRAC_SHIFT)
#define SOR_NV_PDISP_SOR_DP_CONFIG0_0_ACTIVESYM_COUNT_SHIFT	8
#define SOR_NV_PDISP_SOR_DP_CONFIG0_0_ACTIVESYM_COUNT_FIELD	(0x7f << SOR_NV_PDISP_SOR_DP_CONFIG0_0_ACTIVESYM_COUNT_SHIFT)
#define SOR_NV_PDISP_SOR_DP_CONFIG0_0_WATERMARK_SHIFT		0
#define SOR_NV_PDISP_SOR_DP_CONFIG0_0_WATERMARK_FIELD		(0x3f << SOR_NV_PDISP_SOR_DP_CONFIG0_0_WATERMARK_SHIFT)
#define SOR_NV_PDISP_SOR_DP_LINKCTL0_0_TUSIZE_SHIFT		2
#define SOR_NV_PDISP_SOR_DP_LINKCTL0_0_TUSIZE_FIELD		(0x7f << SOR_NV_PDISP_SOR_DP_LINKCTL0_0_TUSIZE_SHIFT)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_PIXELDEPTH_SHIFT		17
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_PIXELDEPTH_FIELD		(0xf << SOR_NV_PDISP_SOR_STATE1_0_ASY_PIXELDEPTH_SHIFT)

#define SOR_NV_PDISP_SOR_STATE1_0_ASY_PIXELDEPTH_DEFAULTVAL	(0)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_PIXELDEPTH_BPP_16_422	(1)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_PIXELDEPTH_BPP_18_444	(2)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_PIXELDEPTH_BPP_20_422	(3)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_PIXELDEPTH_BPP_24_422	(4)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_PIXELDEPTH_BPP_24_444	(5)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_PIXELDEPTH_BPP_30_444	(6)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_PIXELDEPTH_BPP_32_422	(7)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_PIXELDEPTH_BPP_36_444	(8)
#define SOR_NV_PDISP_SOR_STATE1_0_ASY_PIXELDEPTH_BPP_48_444	(9)

#define SOR_NV_PDISP_SOR_CRC_CNTRL_0				0x11
#define SOR_NV_PDISP_SOR_DP_AUDIO_VBLANK_SYMBOLS_0		0x64
#define SOR_NV_PDISP_SOR_DP_SPARE0_0				0x60
#define SOR_NV_PDISP_SOR_PWR_0					0x15
#define SOR_NV_PDISP_SOR_STATE0_0				0x3
#define SOR_NV_PDISP_SOR_SUPER_STATE1_0				0x2
#define SOR_NV_PDISP_SOR_SUPER_STATE0_0				0x1

/* ardpaux.h */
#define DPAUX_DP_AUXDATA_READ_W0				0x19

#define DP_LVDS_SHIFT	25
#define DP_LVDS		(1 << DP_LVDS_SHIFT)

#define SRC_BPP		16
#define COLORDEPTH	0x6
#define COLOR_WHITE	0xFFFFFF

struct soc_nvidia_tegra132_config;	/* forward declaration */
void setup_display(struct soc_nvidia_tegra132_config *config);
void init_dca_regs(void);
void dp_io_powerup(void);
u32 dp_setup_timing(u32 width, u32 height);
void dp_misc_setting(u32 panel_bpp, u32 width, u32 height, u32 winb_addr,
		     u32 lane_count, u32 enhanced_framing, u32 panel_edp,
		     u32 pclkfreq, u32 linkfreq);

#endif /* __SOC_NVIDIA_TEGRA132_INCLUDE_SOC_DISPLAY_H__ */