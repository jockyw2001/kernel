////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2017 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// @file      cam_os_export.c
/// @brief     Cam OS Export Symbol Source File for Linux Kernel Space
///            Only Include This File in Linux Kernel
///////////////////////////////////////////////////////////////////////////////

#include <linux/module.h>
#include "cam_os_wrapper.h"

EXPORT_SYMBOL(CamOsPrintf);
EXPORT_SYMBOL(CamOsScanf);
EXPORT_SYMBOL(CamOsGetChar);
EXPORT_SYMBOL(CamOsSnprintf);
EXPORT_SYMBOL(CamOsMsSleep);
EXPORT_SYMBOL(CamOsGetTimeOfDay);
EXPORT_SYMBOL(CamOsSetTimeOfDay);
EXPORT_SYMBOL(CamOsGetMonotonicTime);
EXPORT_SYMBOL(CamOsThreadCreate);
EXPORT_SYMBOL(CamOsThreadSchedule);
EXPORT_SYMBOL(CamOsThreadWakeUp);
EXPORT_SYMBOL(CamOsThreadJoin);
EXPORT_SYMBOL(CamOsThreadStop);
EXPORT_SYMBOL(CamOsThreadShouldStop);
EXPORT_SYMBOL(CamOsThreadSetName);
EXPORT_SYMBOL(CamOsThreadGetName);
EXPORT_SYMBOL(CamOsThreadGetID);
EXPORT_SYMBOL(CamOsMutexInit);
EXPORT_SYMBOL(CamOsMutexDestroy);
EXPORT_SYMBOL(CamOsMutexLock);
EXPORT_SYMBOL(CamOsMutexTryLock);
EXPORT_SYMBOL(CamOsMutexUnlock);
EXPORT_SYMBOL(CamOsTsemInit);
EXPORT_SYMBOL(CamOsTsemDeinit);
EXPORT_SYMBOL(CamOsTsemUp);
EXPORT_SYMBOL(CamOsTsemDown);
EXPORT_SYMBOL(CamOsTsemTimedDown);
EXPORT_SYMBOL(CamOsTsemTryDown);
EXPORT_SYMBOL(CamOsRwsemInit);
EXPORT_SYMBOL(CamOsRwsemDeinit);
EXPORT_SYMBOL(CamOsRwsemUpRead);
EXPORT_SYMBOL(CamOsRwsemUpWrite);
EXPORT_SYMBOL(CamOsRwsemDownRead);
EXPORT_SYMBOL(CamOsRwsemDownWrite);
EXPORT_SYMBOL(CamOsRwsemTryDownRead);
EXPORT_SYMBOL(CamOsRwsemTryDownWrite);
EXPORT_SYMBOL(CamOsTcondInit);
EXPORT_SYMBOL(CamOsTcondDeinit);
EXPORT_SYMBOL(CamOsTcondSignal);
EXPORT_SYMBOL(CamOsTcondSignalAll);
EXPORT_SYMBOL(CamOsTcondWait);
EXPORT_SYMBOL(CamOsTcondTimedWait);
EXPORT_SYMBOL(CamOsTcondSignalInterruptible);
EXPORT_SYMBOL(CamOsTcondSignalAllInterruptible);
EXPORT_SYMBOL(CamOsTcondWaitInterruptible);
EXPORT_SYMBOL(CamOsTcondTimedWaitInterruptible);
EXPORT_SYMBOL(CamOsTcondWaitActive);
EXPORT_SYMBOL(CamOsMemAlloc);
EXPORT_SYMBOL(CamOsMemCalloc);
EXPORT_SYMBOL(CamOsMemRealloc);
EXPORT_SYMBOL(CamOsMemRelease);
EXPORT_SYMBOL(CamOsDirectMemAlloc);
EXPORT_SYMBOL(CamOsDirectMemRelease);
EXPORT_SYMBOL(CamOsDirectMemFlush);
EXPORT_SYMBOL(CamOsDirectMemStat);
EXPORT_SYMBOL(CamOsDirectMemPhysToMiu);
EXPORT_SYMBOL(CamOsDirectMemMiuToPhys);
EXPORT_SYMBOL(CamOsDirectMemPhysToVirt);
EXPORT_SYMBOL(CamOsDirectMemVirtToPhys);
EXPORT_SYMBOL(CamOsPhyMemMap);
EXPORT_SYMBOL(CamOsPhyMemUnMap);
EXPORT_SYMBOL(CamOsMemCacheCreate);
EXPORT_SYMBOL(CamOsMemCacheDestroy);
EXPORT_SYMBOL(CamOsMemCacheAlloc);
EXPORT_SYMBOL(CamOsMemCacheFree);
EXPORT_SYMBOL(CamOsPropertySet);
EXPORT_SYMBOL(CamOsPropertyGet);
EXPORT_SYMBOL(CamOsMathDivU64);
EXPORT_SYMBOL(CamOsMathDivS64);
EXPORT_SYMBOL(CamOsCopyFromUpperLayer);
EXPORT_SYMBOL(CamOsCopyToUpperLayer);
EXPORT_SYMBOL(CamOsTimerInit);
EXPORT_SYMBOL(CamOsTimerDelete);
EXPORT_SYMBOL(CamOsTimerAdd);
EXPORT_SYMBOL(CamOsTimerModify);
EXPORT_SYMBOL(CamOsAtomicRead);
EXPORT_SYMBOL(CamOsAtomicSet);
EXPORT_SYMBOL(CamOsAtomicAddReturn);
EXPORT_SYMBOL(CamOsAtomicSubReturn);
EXPORT_SYMBOL(CamOsAtomicSubAndTest);
EXPORT_SYMBOL(CamOsAtomicIncReturn);
EXPORT_SYMBOL(CamOsAtomicDecReturn);
EXPORT_SYMBOL(CamOsAtomicIncAndTest);
EXPORT_SYMBOL(CamOsAtomicDecAndTest);
EXPORT_SYMBOL(CamOsAtomicAddNegative);
EXPORT_SYMBOL(CamOsIdrInit);
EXPORT_SYMBOL(CamOsIdrDestroy);
EXPORT_SYMBOL(CamOsIdrAlloc);
EXPORT_SYMBOL(CamOsIdrRemove);
EXPORT_SYMBOL(CamOsIdrFind);
EXPORT_SYMBOL(CamOsPhysMemSize);
EXPORT_SYMBOL(CamOsChipId);
EXPORT_SYMBOL(CamOsInInterrupt);
EXPORT_SYMBOL(CamOsSmpMemoryBarrier);
EXPORT_SYMBOL(CamOsStrError);
EXPORT_SYMBOL(CamOsPanic);
EXPORT_SYMBOL(CamOsStrtol);
EXPORT_SYMBOL(CamOsStrtoul);
EXPORT_SYMBOL(CamOsStrtoull);