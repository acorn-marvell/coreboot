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

#ifndef __SOC_MARVELL_ARMADA38X_I2C_H_
#define __SOC_MARVELL_ARMADA38X_I2C_H_

#include <types.h>

#define TWSI_SPEED                              100000

#define MAX_I2C_NUM                             2
#define MAX_RETRY_CNT                           1000
#define TWSI_TIMEOUT_VALUE                      0x500

#define MV_TWSI_SLAVE_REGS_OFFSET(chanNum)      (0x11000 + (chanNum * 0x100))
#define MV_TWSI_SLAVE_REGS_BASE(unit)           (MV_TWSI_SLAVE_REGS_OFFSET(unit))
#define TWSI_SLAVE_ADDR_REG(chanNum)            (MV_TWSI_SLAVE_REGS_BASE(chanNum) + 0x00)

#define MV_CPUIF_REGS_OFFSET(cpu)               (0x21800 + (cpu) * 0x100)
#define MV_CPUIF_REGS_BASE(cpu)                 (MV_CPUIF_REGS_OFFSET(cpu))
#define CPU_MAIN_INT_CAUSE_REG(vec, cpu)        (MV_CPUIF_REGS_BASE(cpu) + 0x80 + (vec * 0x4))
#define CPU_MAIN_INT_TWSI_OFFS(i)               (2 + i)
#define CPU_MAIN_INT_CAUSE_TWSI(i)              (31 + i)
#define TWSI_CPU_MAIN_INT_CAUSE_REG(cpu)        CPU_MAIN_INT_CAUSE_REG(1, (cpu))
#define MV_TWSI_CPU_MAIN_INT_CAUSE(chNum, cpu)  TWSI_CPU_MAIN_INT_CAUSE_REG(cpu)

#define MV_MBUS_REGS_OFFSET                     (0x20000)
#define MV_CPUIF_SHARED_REGS_BASE               (MV_MBUS_REGS_OFFSET)
#define CPU_INT_SOURCE_CONTROL_REG(i)           (MV_CPUIF_SHARED_REGS_BASE + 0xB00 + (i * 0x4))

#define CPU_INT_SOURCE_CONTROL_IRQ_OFFS         28
#define CPU_INT_SOURCE_CONTROL_IRQ_MASK         (1 << CPU_INT_SOURCE_CONTROL_IRQ_OFFS )

#define TWSI_SLAVE_ADDR_GCE_ENA                 BIT0
#define TWSI_SLAVE_ADDR_7BIT_OFFS               0x1
#define TWSI_SLAVE_ADDR_7BIT_MASK               (0xFF << TWSI_SLAVE_ADDR_7BIT_OFFS)
#define TWSI_SLAVE_ADDR_10BIT_OFFS              0x7
#define TWSI_SLAVE_ADDR_10BIT_MASK              0x300
#define TWSI_SLAVE_ADDR_10BIT_CONST             0xF0

#define TWSI_DATA_REG(chanNum)                  (MV_TWSI_SLAVE_REGS_BASE(chanNum) + 0x04)
#define TWSI_DATA_COMMAND_OFFS                  0x0
#define TWSI_DATA_COMMAND_MASK                  (0x1 << TWSI_DATA_COMMAND_OFFS)
#define TWSI_DATA_COMMAND_WR                    (0x1 << TWSI_DATA_COMMAND_OFFS)
#define TWSI_DATA_COMMAND_RD                    (0x0 << TWSI_DATA_COMMAND_OFFS)
#define TWSI_DATA_ADDR_7BIT_OFFS                0x1
#define TWSI_DATA_ADDR_7BIT_MASK                (0xFF << TWSI_DATA_ADDR_7BIT_OFFS)
#define TWSI_DATA_ADDR_10BIT_OFFS               0x7
#define TWSI_DATA_ADDR_10BIT_MASK               0x300
#define TWSI_DATA_ADDR_10BIT_CONST              0xF0

#define TWSI_CONTROL_REG(chanNum)               (MV_TWSI_SLAVE_REGS_BASE(chanNum) + 0x08)
#define TWSI_CONTROL_ACK                        BIT2
#define TWSI_CONTROL_INT_FLAG_SET               BIT3
#define TWSI_CONTROL_STOP_BIT                   BIT4
#define TWSI_CONTROL_START_BIT                  BIT5
#define TWSI_CONTROL_ENA                        BIT6
#define TWSI_CONTROL_INT_ENA                    BIT7

#define TWSI_STATUS_BAUDE_RATE_REG(chanNum)     (MV_TWSI_SLAVE_REGS_BASE(chanNum) + 0x0c)
#define TWSI_BAUD_RATE_N_OFFS                   0
#define TWSI_BAUD_RATE_N_MASK                   (0x7 << TWSI_BAUD_RATE_N_OFFS)
#define TWSI_BAUD_RATE_M_OFFS                   3
#define TWSI_BAUD_RATE_M_MASK                   (0xF << TWSI_BAUD_RATE_M_OFFS)

#define TWSI_EXTENDED_SLAVE_ADDR_REG(chanNum)   (MV_TWSI_SLAVE_REGS_BASE(chanNum) + 0x10)
#define TWSI_EXTENDED_SLAVE_OFFS                0
#define TWSI_EXTENDED_SLAVE_MASK                (0xFF << TWSI_EXTENDED_SLAVE_OFFS)

#define TWSI_SOFT_RESET_REG(chanNum)            (MV_TWSI_SLAVE_REGS_BASE(chanNum) + 0x1c)

#define TWSI_BUS_ERROR                                            0x00
#define TWSI_START_CON_TRA                                        0x08
#define TWSI_REPEATED_START_CON_TRA                               0x10
#define TWSI_AD_PLS_WR_BIT_TRA_ACK_REC                            0x18
#define TWSI_AD_PLS_WR_BIT_TRA_ACK_NOT_REC                        0x20
#define TWSI_M_TRAN_DATA_BYTE_ACK_REC                             0x28
#define TWSI_M_TRAN_DATA_BYTE_ACK_NOT_REC                         0x30
#define TWSI_M_LOST_ARB_DUR_AD_OR_DATA_TRA                        0x38
#define TWSI_AD_PLS_RD_BIT_TRA_ACK_REC                            0x40
#define TWSI_AD_PLS_RD_BIT_TRA_ACK_NOT_REC                        0x48
#define TWSI_M_REC_RD_DATA_ACK_TRA                                0x50
#define TWSI_M_REC_RD_DATA_ACK_NOT_TRA                            0x58
#define TWSI_SLA_REC_AD_PLS_WR_BIT_ACK_TRA                        0x60
#define TWSI_M_LOST_ARB_DUR_AD_TRA_AD_IS_TRGT_TO_SLA_ACK_TRA_W    0x68
#define TWSI_GNL_CALL_REC_ACK_TRA                                 0x70
#define TWSI_M_LOST_ARB_DUR_AD_TRA_GNL_CALL_AD_REC_ACK_TRA        0x78
#define TWSI_SLA_REC_WR_DATA_AF_REC_SLA_AD_ACK_TRAN               0x80
#define TWSI_SLA_REC_WR_DATA_AF_REC_SLA_AD_ACK_NOT_TRAN           0x88
#define TWSI_SLA_REC_WR_DATA_AF_REC_GNL_CALL_ACK_TRAN             0x90
#define TWSI_SLA_REC_WR_DATA_AF_REC_GNL_CALL_ACK_NOT_TRAN         0x98
#define TWSI_SLA_REC_STOP_OR_REPEATED_STRT_CON                    0xA0
#define TWSI_SLA_REC_AD_PLS_RD_BIT_ACK_TRA                        0xA8
#define TWSI_M_LOST_ARB_DUR_AD_TRA_AD_IS_TRGT_TO_SLA_ACK_TRA_R    0xB0
#define TWSI_SLA_TRA_RD_DATA_ACK_REC                              0xB8
#define TWSI_SLA_TRA_RD_DATA_ACK_NOT_REC                          0xC0
#define TWSI_SLA_TRA_LAST_RD_DATA_ACK_REC                         0xC8
#define TWSI_SEC_AD_PLS_WR_BIT_TRA_ACK_REC                        0xD0
#define TWSI_SEC_AD_PLS_WR_BIT_TRA_ACK_NOT_REC                    0xD8
#define TWSI_SEC_AD_PLS_RD_BIT_TRA_ACK_REC                        0xE0
#define TWSI_SEC_AD_PLS_RD_BIT_TRA_ACK_NOT_REC                    0xE8
#define TWSI_NO_REL_STS_INT_FLAG_IS_KEPT_0                        0xF8

#endif // __SOC_MARVELL_ARMADA38X_I2C_H__
