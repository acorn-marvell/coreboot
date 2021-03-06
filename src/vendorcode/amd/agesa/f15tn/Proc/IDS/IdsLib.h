/* $NoKeywords:$ */
/**
 * @file
 *
 * AMD IDS Routines
 *
 * Contains AMD AGESA Integrated Debug Macros
 *
 * @xrefitem bom "File Content Label" "Release Content"
 * @e project:      AGESA
 * @e sub-project:  IDS
 * @e \$Revision: 63425 $   @e \$Date: 2011-12-22 11:24:10 -0600 (Thu, 22 Dec 2011) $
 */
/*****************************************************************************
 *
 * Copyright 2008 - 2012 ADVANCED MICRO DEVICES, INC.  All Rights Reserved.
 *
 * AMD is granting you permission to use this software (the Materials)
 * pursuant to the terms and conditions of your Software License Agreement
 * with AMD.  This header does *NOT* give you permission to use the Materials
 * or any rights under AMD's intellectual property.  Your use of any portion
 * of these Materials shall constitute your acceptance of those terms and
 * conditions.  If you do not agree to the terms and conditions of the Software
 * License Agreement, please do not use any portion of these Materials.
 *
 * CONFIDENTIALITY:  The Materials and all other information, identified as
 * confidential and provided to you by AMD shall be kept confidential in
 * accordance with the terms and conditions of the Software License Agreement.
 *
 * LIMITATION OF LIABILITY: THE MATERIALS AND ANY OTHER RELATED INFORMATION
 * PROVIDED TO YOU BY AMD ARE PROVIDED "AS IS" WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY OF ANY KIND, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, TITLE, FITNESS FOR ANY PARTICULAR PURPOSE,
 * OR WARRANTIES ARISING FROM CONDUCT, COURSE OF DEALING, OR USAGE OF TRADE.
 * IN NO EVENT SHALL AMD OR ITS LICENSORS BE LIABLE FOR ANY DAMAGES WHATSOEVER
 * (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF PROFITS, BUSINESS
 * INTERRUPTION, OR LOSS OF INFORMATION) ARISING OUT OF AMD'S NEGLIGENCE,
 * GROSS NEGLIGENCE, THE USE OF OR INABILITY TO USE THE MATERIALS OR ANY OTHER
 * RELATED INFORMATION PROVIDED TO YOU BY AMD, EVEN IF AMD HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.  BECAUSE SOME JURISDICTIONS PROHIBIT THE
 * EXCLUSION OR LIMITATION OF LIABILITY FOR CONSEQUENTIAL OR INCIDENTAL DAMAGES,
 * THE ABOVE LIMITATION MAY NOT APPLY TO YOU.
 *
 * AMD does not assume any responsibility for any errors which may appear in
 * the Materials or any other related information provided to you by AMD, or
 * result from use of the Materials or any related information.
 *
 * You agree that you will not reverse engineer or decompile the Materials.
 *
 * NO SUPPORT OBLIGATION: AMD is not obligated to furnish, support, or make any
 * further information, software, technical information, know-how, or show-how
 * available to you.  Additionally, AMD retains the right to modify the
 * Materials at any time, without notice, and is not obligated to provide such
 * modified Materials to you.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS: The Materials are provided with
 * "RESTRICTED RIGHTS." Use, duplication, or disclosure by the Government is
 * subject to the restrictions as set forth in FAR 52.227-14 and
 * DFAR252.227-7013, et seq., or its successor.  Use of the Materials by the
 * Government constitutes acknowledgement of AMD's proprietary rights in them.
 *
 * EXPORT ASSURANCE:  You agree and certify that neither the Materials, nor any
 * direct product thereof will be exported directly or indirectly, into any
 * country prohibited by the United States Export Administration Act and the
 * regulations thereunder, without the required authorization from the U.S.
 * government nor will be used for any purpose prohibited by the same.
 *
 ***************************************************************************/

#ifndef _IDS_LIB_H_
#define _IDS_LIB_H_
#include "OptionsIds.h"
#include "cpuRegisters.h"
#include "cpuApicUtilities.h"
#include "Table.h"
///Specific time stamp performance analysis which need ids control support
#if IDSOPT_CONTROL_ENABLED == TRUE
  #define PERF_SPEC_TS_ANALYSE(StdHeader)  IdsPerfSpecTsAnalyse(StdHeader)
#else
  #define PERF_SPEC_TS_ANALYSE(StdHeader)
#endif


#define IDS_NV_READ_SKIP(NvValue, Nvid, IdsNvPtr, StdHeader)\
  if (((NvValue) = AmdIdsNvReader ((Nvid), (IdsNvPtr), (StdHeader))) != IDS_UNSUPPORTED)
#define IDS_GET_MASK32(HighBit, LowBit) ((((UINT32) 1 << (HighBit - LowBit + 1)) - 1) << LowBit)

#define IDS_MAX_MEM_ITEMS   80     ///< Maximum IDS Mem Table Size in Heap.
///Macro for Ids family feat
#define MAKE_IDS_FAMILY_FEAT_ALL_CORES(FEAT_ID, FAMILY, FUNCTION) \
        {IDS_FEAT_COMMON, IDS_ALL_CORES, FEAT_ID, FAMILY, FUNCTION}


// TYPEDEFS, STRUCTURES, ENUMS
//

typedef AGESA_STATUS (*PF_IDS_AP_TASK) (VOID *AptaskPara, AMD_CONFIG_PARAMS *StdHeader);

///Structure define for IdsAgesaRunFcnOnApLate
typedef struct _IDSAPLATETASK {
  PF_IDS_AP_TASK  ApTask; ///< Point function which AP need to do
  VOID *ApTaskPara; ///< Point to Ap function parameter1
} IDSAPLATETASK;

/// Data Structure defining IDS Data in HEAP
/// This data structure contains information that is stored in HEAP and will be
/// used in IDS backend function. It includes the size of memory to be allocated
/// for IDS, the relative offsets of the mapping table IDS setup options, the GRA
/// table and the register table to override mem setting. It also includes a base
/// address of IDS override image which will be used to control the behavior of
/// AGESA testpoint if this feature is enabled.
typedef struct {
  BOOLEAN IgnoreIdsDefault;             ///< Control ignore Default value of IDS NV list specified by IdsNvTableOffset
  UINT64 IdsImageBase;                 ///< IDS Override Image Base Address
  UINT32 IdsHeapMemSize;              ///< IDS Total Memory Size in Heap
  UINT32 IdsNvTableOffset;            ///< Offset of IDS NV Table
  UINT32 IdsMemTableOffset;           ///< Offset of IDS Mem Table
  UINT32 IdsExtendOffset;                ///< Offset of Ids extend heap
} IDS_CONTROL_STRUCT;

#define MAX_PERFORMANCE_UNIT_NUM 100
/// Data Structure of Parameters for TestPoint_TSC.
typedef struct {
  UINT32 LineInFile;                    ///< Line of current time counter
  UINT64 StartTsc;                  ///< The StartTimer of TestPoint_TSC
} TestPoint_TSC;

/// Data Structure of Parameters for TP_Perf_STRUCT.
typedef struct {
  UINT32 Signature;                ///< "TIME"
  UINT32 Index;                    ///< The Index of TP_Perf_STRUCT
  UINT32 TscInMhz;            ///< Tsc counter in 1 mhz
  TestPoint_TSC TP[MAX_PERFORMANCE_UNIT_NUM];       ///< The TP of TP_Perf_STRUCT
} TP_Perf_STRUCT;

///Bus speed Optimization
typedef enum {
  IDS_POWER_POLICY_PERFORMANCE = 0,              ///< Performance
  IDS_POWER_POLICY_POWER = 1,              ///< Power
  IDS_POWER_POLICY_AUTO = 3,              ///< Auto
} IDS_NV_AMDBUSSPEEDOPTIMIZATION;

///IDS early AP task
typedef struct _IDS_EARLY_AP_TASK0 {
  UINT8 Core;             ///< Core to run Aptask
  AP_TASK ApTask;        ///< Speicify task property
} IDS_EARLY_AP_TASK0;

#define IDS_EARLY_AP_TASK_PARA_NUM 100
///IDS early AP task
typedef struct _IDS_EARLY_AP_TASK {
  IDS_EARLY_AP_TASK0 Ap_Task0;    ///< Ap Task exclude parameter buffer
  UINT8 Parameters[IDS_EARLY_AP_TASK_PARA_NUM];          ///< Parameter buffer
} IDS_EARLY_AP_TASK;


#define IDS_ALL_SOCKET 0xFF
#define IDS_ALL_MODULE 0xFF
#define IDS_ALL_CORE   0xFF
#define IDS_ALL_DCT   0xFF

/*----------------------------------------------------------------------------------------
 *                        F U N C T I O N    P R O T O T Y P E
 *----------------------------------------------------------------------------------------
 */

IDS_STATUS
IdsSubUCode (
  IN OUT  VOID *DataPtr,
  IN OUT  AMD_CONFIG_PARAMS *StdHeader,
  IN      IDS_NV_ITEM *IdsNvPtr
  );

IDS_STATUS
IdsSubGangingMode (
  IN OUT   VOID *DataPtr,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader,
  IN       IDS_NV_ITEM *IdsNvPtr
  );

IDS_STATUS
IdsSubPowerDownMode (
  IN OUT   VOID *DataPtr,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader,
  IN       IDS_NV_ITEM *IdsNvPtr
  );

IDS_STATUS
IdsSubBurstLength32 (
  IN OUT   VOID *DataPtr,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader,
  IN       IDS_NV_ITEM *IdsNvPtr
  );


IDS_STATUS
IdsSubAllMemClkEn (
  IN OUT   VOID *DataPtr,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader,
  IN       IDS_NV_ITEM *IdsNvPtr
  );

IDS_STATUS
IdsSubDllShutDownSR (
  IN OUT   VOID *DataPtr,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader,
  IN       IDS_NV_ITEM *IdsNvPtr
  );

IDS_STATUS
IdsSubHtLinkControl (
     OUT   VOID  *Data,
  IN       AMD_CONFIG_PARAMS *StdHeader,
  IN       IDS_NV_ITEM *IdsNvPtr
  );

IDS_STATUS
IdsSubPostPState (
  IN OUT   VOID *DataPtr,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader,
  IN       IDS_NV_ITEM *IdsNvPtr
  );

IDS_STATUS
IdsSubPowerPolicyOverride (
  IN OUT   VOID *DataPtr,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader,
  IN       IDS_NV_ITEM *IdsNvPtr
  );

AGESA_STATUS
AmdIdsCtrlInitialize (
  IN OUT   AMD_CONFIG_PARAMS *StdHeader
  );

IDS_STATUS
AmdIdsNvReader (
  IN      UINT16 IdsNvId,
  IN      IDS_NV_ITEM *NvTablePtr,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader
  );

AGESA_STATUS
AmdGetIdsNvTable (
  IN OUT   VOID  **IdsNvTable,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader
  );

VOID
IdsOutPort (
  IN       UINT32 Addr,
  IN       UINT32 Value,
  IN       UINT32 Flag
  );

IDS_STATUS
IdsCommonReturn (
  IN OUT   VOID *DataPtr,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader,
  IN       IDS_NV_ITEM *IdsNvPtr
  );

AGESA_STATUS
IdsAgesaRunFcnOnApLate  (
  IN       UINTN               ApicIdOfCore,
  IN         IDSAPLATETASK  *ApLateTaskPtr,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader
  );

AGESA_STATUS
IdsAgesaRunFcnOnAllCoresLate  (
  IN         IDSAPLATETASK  *ApLateTaskPtr,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader
  );

IDS_STATUS
IdsParseFeatTbl (
  IN       AGESA_IDS_OPTION IdsOption,
  IN       CONST IDS_FAMILY_FEAT_STRUCT * PIdsFeatTbl[],
  IN OUT   VOID *DataPtr,
  IN       IDS_NV_ITEM *IdsNvPtr,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader
  );

IDS_STATUS
IdsSubPowerDownCtrl (
  IN OUT   VOID *DataPtr,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader,
  IN       IDS_NV_ITEM *IdsNvPtr
  );

IDS_STATUS
IdsSubHdtOut (
  IN OUT   VOID *DataPtr,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader,
  IN       IDS_NV_ITEM *IdsNvPtr
  );

UINT8
IdsGetNumPstatesFamCommon (
  IN OUT   AMD_CONFIG_PARAMS  *StdHeader
  );

VOID
IdsApRunCodeOnAllLocalCores (
  IN       AP_TASK *TaskPtr,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader
  );
IDS_STATUS
IdsSubTargetPstate (
  IN OUT   VOID *DataPtr,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader,
  IN       IDS_NV_ITEM *IdsNvPtr
  );

VOID
IdsMakePciRegEntry (
  IN OUT   TABLE_ENTRY_FIELDS **TableEntry,
  IN       UINT64 Family,
  IN       UINT64 Revision,
  IN       UINT32 PciAddr,
  IN       UINT32 Data,
  IN        UINT32 Mask
  );

VOID
IdsMakeHtLinkPciRegEntry (
  IN OUT   TABLE_ENTRY_FIELDS **TableEntry,
  IN       UINT64 Family,
  IN       UINT64 Revision,
  IN       UINT32 HtHostFeat,
  IN       UINT32 PciAddr,
  IN       UINT32 Data,
  IN        UINT32 Mask
  );

VOID
IdsMakeHtFeatPciRegEntry (
  IN OUT   TABLE_ENTRY_FIELDS **TableEntry,
  IN       UINT64 Family,
  IN       UINT64 Revision,
  IN       UINT32 HtHostFeat,
  IN       UINT32 PackageType,
  IN       UINT32 PciAddr,
  IN       UINT32 Data,
  IN        UINT32 Mask
  );

VOID
IdsMakeHtHostPciRegEntry (
  IN OUT   TABLE_ENTRY_FIELDS **TableEntry,
  IN       UINT64 Family,
  IN       UINT64 Revision,
  IN       UINT32 HtHostFeat,
  IN       UINT32 PciAddr,
  IN       UINT32 Data,
  IN        UINT32 Mask
  );

VOID
IdsMakeHtPhyRegEntry (
  IN OUT   TABLE_ENTRY_FIELDS **TableEntry,
  IN       UINT64 Family,
  IN       UINT64 Revision,
  IN       UINT32 HtPhyLinkFeat,
  IN       UINT32 Address,
  IN       UINT32 Data,
  IN        UINT32 Mask
  );

VOID
IdsLibPciWriteBitsToAllNode (
  IN       PCI_ADDR PciAddress,
  IN       UINT8 Highbit,
  IN       UINT8 Lowbit,
  IN       UINT32 *Value,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader
  );
VOID
IdsRunCodeOnCoreEarly (
  IN       UINT8 Socket,
  IN       UINT8 Core,
  IN       AP_TASK* ApTask,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader
  );

VOID
IdsGetMask64bits (
  IN       UINT64 RegVal,
  IN       UINT8  Highbit,
  IN       UINT8  Lowbit,
  IN OUT   UINT64 *AndMask,
  IN OUT   UINT64 *OrMask
  );

VOID
IdsGetMask32bits (
  IN       UINT32 RegVal,
  IN       UINT8  Highbit,
  IN       UINT8  Lowbit,
  IN OUT   UINT32 *AndMask,
  IN OUT   UINT32 *OrMask
  );

VOID
IdsGetMask16bits (
  IN       UINT16 RegVal,
  IN       UINT8  Highbit,
  IN       UINT8  Lowbit,
  IN OUT   UINT32 *AndMask,
  IN OUT   UINT32 *OrMask
  );

VOID
IdsGetStartEndModule (
  IN       UINT8  ModuleId,
  IN OUT   UINT8 *StartModule,
  IN OUT   UINT8 *EndModule
  );


VOID
IdsGetStartEndSocket (
  IN       UINT8  SocketId,
  IN OUT   UINT8 *StartSocket,
  IN OUT   UINT8 *EndSocket
  );

BOOLEAN
IdsCheckPciExisit (
  IN       PCI_ADDR PciAddr,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader
  );

VOID
IdsLibDataMaskSet32 (
  IN OUT   UINT32 *Value,
  IN       UINT32  AndMask,
  IN       UINT32  OrMask
  );
#define IDS_CPB_BOOST_DIS_IGNORE  0xFFFFFFFFul

#endif //_IDS_LIB_H_

