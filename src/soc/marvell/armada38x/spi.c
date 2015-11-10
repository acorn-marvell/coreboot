/*
 * This file is part of the coreboot project.
 *
 * Copyright 2015 Google Inc.
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
 * Foundation, Inc.
 */

#include <arch/io.h>
#include <delay.h>
#include <spi_flash.h>
#include <stdlib.h>
#include <string.h>
#include <console/console.h>
#include <soc/common.h>

/******************************************************************************
base type define
*******************************************************************************/
#define MV_SPI_REG_READ		MV_REG_READ
#define MV_SPI_REG_WRITE	MV_REG_WRITE
#define MV_SPI_REG_BIT_SET	MV_REG_BIT_SET
#define MV_SPI_REG_BIT_RESET	MV_REG_BIT_RESET

#define MV_SPI_REGS_OFFSET(unit)                        (0x10600 + (unit * 0x80))
#define MV_SPI_REGS_BASE(unit)		                (MV_SPI_REGS_OFFSET(unit))
#define	MV_SPI_IF_CONFIG_REG(spiId)			(MV_SPI_REGS_BASE(spiId) + 0x04)
#define	MV_SPI_SPR_OFFSET				0
#define	MV_SPI_SPR_MASK					(0xF << MV_SPI_SPR_OFFSET)
#define	MV_SPI_SPPR_0_OFFSET				4
#define	MV_SPI_SPPR_0_MASK				(0x1 << MV_SPI_SPPR_0_OFFSET)
#define	MV_SPI_SPPR_HI_OFFSET				6
#define	MV_SPI_SPPR_HI_MASK				(0x3 << MV_SPI_SPPR_HI_OFFSET)

#define	MV_SPI_BYTE_LENGTH_OFFSET			5	/* bit 5 */
#define	MV_SPI_BYTE_LENGTH_MASK				(0x1  << MV_SPI_BYTE_LENGTH_OFFSET)

#define	MV_SPI_IF_CTRL_REG(spiId)			(MV_SPI_REGS_BASE(spiId) + 0x00)
#define	MV_SPI_CS_ENABLE_OFFSET				0		/* bit 0 */
#define	MV_SPI_CS_ENABLE_MASK				(0x1  << MV_SPI_CS_ENABLE_OFFSET)

#define	MV_SPI_CS_NUM_OFFSET				2
#define	MV_SPI_CS_NUM_MASK				(0x7 << MV_SPI_CS_NUM_OFFSET)
#define	MV_SPI_CPOL_OFFSET				11
#define	MV_SPI_CPOL_MASK				(0x1 << MV_SPI_CPOL_OFFSET)
#define	MV_SPI_CPHA_OFFSET				12
#define	MV_SPI_CPHA_MASK				(0x1 << MV_SPI_CPHA_OFFSET)
#define	MV_SPI_TXLSBF_OFFSET				13
#define	MV_SPI_TXLSBF_MASK				(0x1 << MV_SPI_TXLSBF_OFFSET)
#define	MV_SPI_RXLSBF_OFFSET				14
#define	MV_SPI_RXLSBF_MASK				(0x1 << MV_SPI_RXLSBF_OFFSET)

/* SPI transfer flags */
#define SPI_XFER_BEGIN	0x01			/* Assert CS before transfer */
#define SPI_XFER_END	0x02			/* Deassert CS after transfer */

#define	MV_SPI_INT_CAUSE_REG(spiId)			(MV_SPI_REGS_BASE(spiId) + 0x10)
#define	MV_SPI_DATA_OUT_REG(spiId)			(MV_SPI_REGS_BASE(spiId) + 0x08)
#define	MV_SPI_WAIT_RDY_MAX_LOOP			100000
#define	MV_SPI_DATA_IN_REG(spiId)			(MV_SPI_REGS_BASE(spiId) + 0x0c)

#define MV_SPI_TMNG_PARAMS_REG(spiId)                   (MV_SPI_REGS_BASE(spiId) + 0x18)
#define MV_SPI_TMISO_SAMPLE_OFFSET                      6
#define MV_SPI_TMISO_SAMPLE_MASK                        (0x3 << MV_SPI_TMISO_SAMPLE_OFFSET)

#define CONFIG_ENV_SPI_MAX_HZ           50000000
#define CONFIG_SF_DEFAULT_SPEED        CONFIG_ENV_SPI_MAX_HZ

#define CMD_READ_ARRAY_FAST		0x0b

/******************************************************************************
base type define end
*******************************************************************************/

/******************************************************************************
struct define
*******************************************************************************/
typedef enum {
        SPI_TYPE_FLASH = 0,
        SPI_TYPE_SLIC_ZARLINK_SILABS,
        SPI_TYPE_SLIC_LANTIQ,
        SPI_TYPE_SLIC_ZSI,
        SPI_TYPE_SLIC_ISI
} MV_SPI_TYPE;

typedef struct {
	unsigned short		ctrlModel;
	unsigned int		tclk;
} MV_SPI_HAL_DATA;

typedef struct {
	int		clockPolLow;
	enum {
		SPI_CLK_HALF_CYC,
		SPI_CLK_BEGIN_CYC
	}		clockPhase;
	int		txMsbFirst;
	int		rxMsbFirst;
} MV_SPI_IF_PARAMS;

typedef struct {
	/* Does this device support 16 bits access */
	int	en16Bit;
	/* should we assert / disassert CS for each byte we read / write */
	int	byteCsAsrt;
	int clockPolLow;
	unsigned int	baudRate;
	unsigned int clkPhase;
} MV_SPI_TYPE_INFO;


/******************************************************************************
struct define end 
*******************************************************************************/

/******************************************************************************
param define
*******************************************************************************/
static MV_SPI_HAL_DATA	spiHalData;
static MV_SPI_TYPE_INFO *currSpiInfo = NULL;
static MV_SPI_TYPE_INFO spiTypes[] = {
	{
		.en16Bit = MV_TRUE,
		.clockPolLow = MV_TRUE,
		.byteCsAsrt = MV_FALSE,
		.baudRate = (20 << 20), /*  20M */
		.clkPhase = SPI_CLK_BEGIN_CYC
	},
	{
		.en16Bit = MV_FALSE,
		.clockPolLow = MV_TRUE,
		.byteCsAsrt = MV_TRUE,
		.baudRate = 0x00800000,
		.clkPhase = SPI_CLK_BEGIN_CYC
	},
	{
		.en16Bit = MV_FALSE,
		.clockPolLow = MV_TRUE,
		.byteCsAsrt = MV_FALSE,
		.baudRate = 0x00800000,
		.clkPhase = SPI_CLK_BEGIN_CYC
	},
	{
		.en16Bit = MV_FALSE,
		.clockPolLow = MV_TRUE,
		.byteCsAsrt = MV_TRUE,
		.baudRate = 0x00800000,
		.clkPhase = SPI_CLK_HALF_CYC
	},
	{
		.en16Bit = MV_FALSE,
		.clockPolLow = MV_FALSE,
		.byteCsAsrt = MV_TRUE,
		.baudRate = 0x00200000,
		.clkPhase = SPI_CLK_HALF_CYC
	}
};

/******************************************************************************
param define end
*******************************************************************************/

static struct spi_slave s_spi;

static int mvSpiBaudRateSet(unsigned char spiId, unsigned int serialBaudRate);
static void mvSpiCsDeassert(unsigned char spiId);
static int mvSpiCsSet(unsigned char spiId, unsigned char csId);
static int mvSpiIfConfigSet(unsigned char spiId, MV_SPI_IF_PARAMS *ifParams);
static int mvSpiParamsSet(unsigned char spiId, unsigned char csId, MV_SPI_TYPE type);
static int mvSpiInit(unsigned char spiId, unsigned int serialBaudRate, MV_SPI_HAL_DATA *halData);
static int mvSysSpiInit(unsigned char spiId, unsigned int serialBaudRate);
static void mvSpiCsAssert(unsigned char spiId);
static int mvSpi8bitDataTxRx(unsigned char spiId, unsigned char txData, unsigned char *pRxData);

int mvSpiBaudRateSet(unsigned char spiId, unsigned int serialBaudRate)
{
	unsigned int spr, sppr;
	unsigned int divider;
	unsigned int bestSpr = 0, bestSppr = 0;
	unsigned char exactMatch = 0;
	unsigned int minBaudOffset = 0xFFFFFFFF;
	unsigned int cpuClk = spiHalData.tclk; /*mvCpuPclkGet();*/
	unsigned int tempReg;

	/* Find the best prescale configuration - less or equal */
	for (spr = 1; spr <= 15; spr++) {
		for (sppr = 0; sppr <= 7; sppr++) {
			divider = spr * (1 << sppr);
			/* check for higher - irrelevent */
			if ((cpuClk / divider) > serialBaudRate)
				continue;

			/* check for exact fit */
			if ((cpuClk / divider) == serialBaudRate) {
				bestSpr = spr;
				bestSppr = sppr;
				exactMatch = 1;
				break;
			}

			/* check if this is better than the previous one */
			if ((serialBaudRate - (cpuClk / divider)) < minBaudOffset) {
				minBaudOffset = (serialBaudRate - (cpuClk / divider));
				bestSpr = spr;
				bestSppr = sppr;
			}
		}

		if (exactMatch == 1)
			break;
	}

	if (bestSpr == 0) {
		printk(BIOS_INFO, "%s ERROR: SPI baud rate prescale error!\n", __func__);
		return MV_OUT_OF_RANGE;
	}

	/* configure the Prescale */
	tempReg = MV_SPI_REG_READ(MV_SPI_IF_CONFIG_REG(spiId)) & ~(MV_SPI_SPR_MASK | MV_SPI_SPPR_0_MASK |
			MV_SPI_SPPR_HI_MASK);
	tempReg |= ((bestSpr << MV_SPI_SPR_OFFSET) |
			((bestSppr & 0x1) << MV_SPI_SPPR_0_OFFSET) |
			((bestSppr >> 1) << MV_SPI_SPPR_HI_OFFSET));
	MV_SPI_REG_WRITE(MV_SPI_IF_CONFIG_REG(spiId), tempReg);

	return MV_OK;
}

void mvSpiCsDeassert(unsigned char spiId)
{
	MV_SPI_REG_BIT_RESET(MV_SPI_IF_CTRL_REG(spiId), MV_SPI_CS_ENABLE_MASK);
}

int mvSpiCsSet(unsigned char spiId, unsigned char csId)
{
	unsigned int	ctrlReg;
	static unsigned char lastCsId = 0xFF;

	if (csId > 7)
		return MV_BAD_PARAM;

	if (lastCsId == csId)
		return MV_OK;

	ctrlReg = MV_SPI_REG_READ(MV_SPI_IF_CTRL_REG(spiId));
	ctrlReg &= ~MV_SPI_CS_NUM_MASK;
	ctrlReg |= (csId << MV_SPI_CS_NUM_OFFSET);
	MV_SPI_REG_WRITE(MV_SPI_IF_CTRL_REG(spiId), ctrlReg);

	lastCsId = csId;

	return MV_OK;
}

int mvSpiIfConfigSet(unsigned char spiId, MV_SPI_IF_PARAMS *ifParams)
{
	unsigned int	ctrlReg;

	ctrlReg = MV_SPI_REG_READ(MV_SPI_IF_CONFIG_REG(spiId));

	/* Set Clock Polarity */
	ctrlReg &= ~(MV_SPI_CPOL_MASK | MV_SPI_CPHA_MASK |
			MV_SPI_TXLSBF_MASK | MV_SPI_RXLSBF_MASK);
	if (ifParams->clockPolLow)
		ctrlReg |= MV_SPI_CPOL_MASK;

	if (ifParams->clockPhase == SPI_CLK_BEGIN_CYC)
		ctrlReg |= MV_SPI_CPHA_MASK;

	if (ifParams->txMsbFirst)
		ctrlReg |= MV_SPI_TXLSBF_MASK;

	if (ifParams->rxMsbFirst)
		ctrlReg |= MV_SPI_RXLSBF_MASK;

	MV_SPI_REG_WRITE(MV_SPI_IF_CONFIG_REG(spiId), ctrlReg);

	return MV_OK;
}

int mvSpiParamsSet(unsigned char spiId, unsigned char csId, MV_SPI_TYPE type)
{
	MV_SPI_IF_PARAMS ifParams;

	if (MV_OK != mvSpiCsSet(spiId, csId)) {
		printk(BIOS_INFO, "Error, setting SPI CS failed\n");
		return MV_ERROR;
	}

	if (currSpiInfo != (&(spiTypes[type]))) {
		currSpiInfo = &(spiTypes[type]);
		mvSpiBaudRateSet(spiId, currSpiInfo->baudRate);

		ifParams.clockPolLow = currSpiInfo->clockPolLow;
		ifParams.clockPhase = currSpiInfo->clkPhase;
		ifParams.txMsbFirst = MV_FALSE;
		ifParams.rxMsbFirst = MV_FALSE;
		mvSpiIfConfigSet(spiId, &ifParams);
	}

	return MV_OK;
}

int mvSpiInit(unsigned char spiId, unsigned int serialBaudRate, MV_SPI_HAL_DATA *halData)
{
	int ret;
	unsigned int timingReg;

	spiHalData.ctrlModel = halData->ctrlModel;
	spiHalData.tclk = halData->tclk;

	/* Set the serial clock */
	ret = mvSpiBaudRateSet(spiId, serialBaudRate);
	if (ret != MV_OK)
		return ret;

	/* Configure the default SPI mode to be 16bit */
	MV_SPI_REG_BIT_SET(MV_SPI_IF_CONFIG_REG(spiId), MV_SPI_BYTE_LENGTH_MASK);

	timingReg = MV_REG_READ(MV_SPI_TMNG_PARAMS_REG(spiId));
        timingReg &= ~MV_SPI_TMISO_SAMPLE_MASK;
        timingReg |= (0x2) << MV_SPI_TMISO_SAMPLE_OFFSET;
        MV_REG_WRITE(MV_SPI_TMNG_PARAMS_REG(spiId), timingReg);

	/* Verify that the CS is deasserted */
	mvSpiCsDeassert(spiId);

	mvSpiParamsSet(spiId, 0, SPI_TYPE_FLASH);

	return MV_OK;
}

int mvSysSpiInit(unsigned char spiId, unsigned int serialBaudRate)
{
	MV_SPI_HAL_DATA halData;

	halData.ctrlModel = MV_6810_DEV_ID;
	halData.tclk = MV_BOARD_TCLK_250MHZ;

	return mvSpiInit(spiId, serialBaudRate, &halData);
}

void mvSpiCsAssert(unsigned char spiId)
{
	MV_SPI_REG_BIT_SET(MV_SPI_IF_CTRL_REG(spiId), MV_SPI_CS_ENABLE_MASK);
}

int mvSpi8bitDataTxRx(unsigned char spiId, unsigned char txData, unsigned char *pRxData)
{
	unsigned int i;
	int ready = MV_FALSE;

	if (currSpiInfo->byteCsAsrt)
		mvSpiCsAssert(spiId);

	/* First clear the bit in the interrupt cause register */
	MV_SPI_REG_WRITE(MV_SPI_INT_CAUSE_REG(spiId), 0x0);

	/* Transmit data */
	MV_SPI_REG_WRITE(MV_SPI_DATA_OUT_REG(spiId), txData);

	/* wait with timeout for memory ready */
	for (i = 0; i < MV_SPI_WAIT_RDY_MAX_LOOP; i++) {
		if (MV_SPI_REG_READ(MV_SPI_INT_CAUSE_REG(spiId))) {
			ready = MV_TRUE;
			break;
		}
	}

	if (!ready) {
		if (currSpiInfo->byteCsAsrt) {
			mvSpiCsDeassert(spiId);
			/* WA to compansate Zarlink SLIC CS off time */
			udelay(4);
		}
		return MV_TIMEOUT;
	}

	/* check that the RX data is needed */
	if (pRxData)
		*pRxData = MV_SPI_REG_READ(MV_SPI_DATA_IN_REG(spiId));

	if (currSpiInfo->byteCsAsrt) {
		mvSpiCsDeassert(spiId);
		/* WA to compansate Zarlink SLIC CS off time */
		udelay(4);
	}

	return MV_OK;
}


static int mrvl_spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	int ret;
	unsigned char* pdout = (unsigned char*)dout;
	unsigned char* pdin = (unsigned char*)din;
	int tmp_bitlen = bitlen;
	unsigned char tmp_dout = 0;

	/* Verify that the SPI mode is in 8bit mode */
	MV_REG_BIT_RESET(MV_SPI_IF_CONFIG_REG(slave->bus), MV_SPI_BYTE_LENGTH_MASK);

	while (tmp_bitlen > 0){
		if(pdout)
			tmp_dout = (*pdout) & 0xff;

		/* Transmitted and wait for the transfer to be completed */
		if ((ret = mvSpi8bitDataTxRx(slave->bus,tmp_dout, pdin)) != MV_OK)
			return ret;

		/* increment the pointers */
		if (pdin){
			pdin++;
		}
		if (pdout){
			pdout++;
		}
		tmp_bitlen-=8;
	}
	return 0;
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs)
{
        struct spi_slave *slave = &s_spi;

        slave->bus = bus;
        slave->cs = cs;

        mvSysSpiInit(bus, CONFIG_SF_DEFAULT_SPEED);
        return slave;
}


int spi_claim_bus(struct spi_slave *slave)
{
	mvSpiCsSet(slave->bus, slave->cs);
	mvSpiCsAssert(slave->bus);
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	mvSpiCsDeassert(slave->bus);
}

unsigned int spi_crop_chunk(unsigned int cmd_len, unsigned int buf_len)
{
        return buf_len;
}

int spi_xfer(struct spi_slave *slave, const void *dout,
             unsigned out_bytes, void *din, unsigned in_bytes)
{
	int ret = -1;
	if(out_bytes){
		ret = mrvl_spi_xfer(slave, out_bytes*8, dout, din, SPI_XFER_BEGIN | SPI_XFER_END);
	}else{
		ret = mrvl_spi_xfer(slave, in_bytes*8, dout, din,SPI_XFER_BEGIN | SPI_XFER_END);
	}
	return ret;
}
