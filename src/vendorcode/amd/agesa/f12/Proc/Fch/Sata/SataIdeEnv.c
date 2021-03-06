/* $NoKeywords:$ */
/**
 * @file
 *
 * Config Fch SATA (IDE mode) controller
 *
 * Init SATA IDE (Native IDE) mode features.
 *
 * @xrefitem bom "File Content Label" "Release Content"
 * @e project:     AGESA
 * @e sub-project: FCH
 * @e \$Revision: 44324 $   @e \$Date: 2010-12-22 17:16:51 +0800 (Wed, 22 Dec 2010) $
 *
 */
/*
*****************************************************************************
*
* Copyright (c) 2011, Advanced Micro Devices, Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Advanced Micro Devices, Inc. nor the names of 
 *       its contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ADVANCED MICRO DEVICES, INC. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************
*/
#include "FchPlatform.h"
#include "Filecode.h"
#define FILECODE PROC_FCH_SATA_SATAIDEENV_FILECODE

/**
 * FchInitEnvSataIde - Config SATA IDE controller before PCI
 * emulation
 *
 *
 *
 * @param[in] FchDataPtr Fch configuration structure pointer.
 *
 */
VOID
FchInitEnvSataIde (
  IN  VOID     *FchDataPtr
  )
{
  UINT8        ChannelByte;
  FCH_DATA_BLOCK         *LocalCfgPtr;
  AMD_CONFIG_PARAMS      *StdHeader;

  LocalCfgPtr = (FCH_DATA_BLOCK *) FchDataPtr;
  StdHeader = LocalCfgPtr->StdHeader;

  //
  // Class code
  //
  if ( LocalCfgPtr->Sata.SataClass == SataLegacyIde ) {
    RwPci (((SATA_BUS_DEV_FUN << 16) + FCH_SATA_REG08), AccessWidth32, 0, 0x01018A40, StdHeader);
  } else {
    RwPci (((SATA_BUS_DEV_FUN << 16) + FCH_SATA_REG08), AccessWidth32, 0, 0x01018F40, StdHeader);
  }
  //
  // Device ID
  //
  RwPci (((SATA_BUS_DEV_FUN << 16) + FCH_SATA_REG02), AccessWidth16, 0, FCH_SATA_DID, StdHeader);
  //
  // SSID
  //
  if (LocalCfgPtr->Sata.SataIdeSsid != NULL ) {
    RwPci ((SATA_BUS_DEV_FUN << 16) + FCH_SATA_REG2C, AccessWidth32, 0x00, LocalCfgPtr->Sata.SataIdeSsid, StdHeader);
  }
  //
  // Sata IDE Channel configuration
  //
  ChannelByte = 0x00;
  ReadPci (((SATA_BUS_DEV_FUN << 16) + FCH_SATA_REG48 + 3), AccessWidth8, &ChannelByte, StdHeader);
  ChannelByte &= 0xCF;

  if ( LocalCfgPtr->Sata.SataDisUnusedIdePChannel ) {
    ChannelByte |= 0x10;
  }

  if ( LocalCfgPtr->Sata.SataDisUnusedIdeSChannel ) {
    ChannelByte |= 0x20;
  }

  WritePci (((SATA_BUS_DEV_FUN << 16) + FCH_SATA_REG48 + 3), AccessWidth8, &ChannelByte, StdHeader);
}
