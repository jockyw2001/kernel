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
/// @file      cam_os_wrapper.c
/// @brief     Cam OS Wrapper Source File for
///            1. RTK OS
///            2. Linux User Space
///            3. Linux Kernel Space
///////////////////////////////////////////////////////////////////////////////

#if defined(__KERNEL__)
#define CAM_OS_LINUX_KERNEL
#endif

#ifdef CAM_OS_RTK
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "time.h"
#include "sys_sys.h"
#include "sys_sys_math64.h"
#include "sys_sys_tools.h"
#include "sys_sys_core.h"
#include "sys_MsWrapper_cus_os_flag.h"
#include "sys_MsWrapper_cus_os_sem.h"
#include "sys_MsWrapper_cus_os_util.h"
#include "sys_MsWrapper_cus_os_timer.h"
#include "sys_sys_isw_uart.h"
#include "sys_rtk_hp.h"
#include "hal_drv_util.h"
#include "sys_time.h"
#include "cam_os_wrapper.h"
#include "cam_os_util.h"
#include "cam_os_util_list.h"
#include "cam_os_util_bitmap.h"


#define __RTK_US_RESOLUTION_TIMER__
#define CAM_OS_THREAD_STACKSIZE_DEFAULT         8192

typedef void *CamOsThreadEntry_t(void *);

typedef struct
{
    MsTaskId_e eHandleObj;
    CamOsThreadEntry_t *pfnEntry;
    void *pArg;
    Ms_Flag_t tExitFlag;
    void *pStack;
} CamOsThreadHandleRtk_t, *pCamOsThreadHandleRtk;

typedef struct
{
    uint32_t nInited;
    Ms_Mutex_t tMutex;
} CamOsMutexRtk_t, *pCamOsMutexRtk;

typedef struct
{
    uint32_t nInited;
    void *pTsem;
} CamOsTsemRtk_t, *pCamOsTsemRtk;

typedef struct
{
    uint32_t nInited;
    uint32_t nReadCount;
    Ms_Mutex_t tRMutex;
    Ms_Mutex_t tWMutex;
} CamOsRwsemRtk_t, *pCamOsRwsemRtk;

typedef struct
{
    uint32_t nInited;
    void *pTsem;
} CamOsTcondRtk_t, *pCamOsTcondRtk;

typedef struct
{
    MsTimerId_e eTimerID;
    void (*pfnCallback)(uint32_t);
    void *pDataPtr;
} CamOsTimerRtk_t, *pCamOsTimerRtk;

typedef struct
{
    uint8_t nPoolID;
    uint32_t nObjSize;
} CamOsMemCacheRtk_t, *pCamOsMemCacheRtk;

static Ms_Mutex_t _gtSelfInitLock = {0};
static Ms_Mutex_t _gtMemLock = {0};

_Static_assert(sizeof(CamOsMutex_t) >= sizeof(Ms_Flag_t) + 4, "CamOsMutex_t size define not enough!");
_Static_assert(sizeof(CamOsTsem_t) >= sizeof(CamOsTsemRtk_t), "CamOsTsem_t size define not enough!");
_Static_assert(sizeof(CamOsRwsem_t) >= sizeof(CamOsRwsemRtk_t), "CamOsRwsem_t size define not enough!");
_Static_assert(sizeof(CamOsTcond_t) >= sizeof(CamOsTcondRtk_t), "CamOsTcond_t size define not enough!");
_Static_assert(sizeof(CamOsTimer_t) >= sizeof(CamOsTimerRtk_t), "CamOsTimer_t size define not enough!");
_Static_assert(sizeof(CamOsMemCache_t) >= sizeof(CamOsMemCacheRtk_t), "CamOsMemCache_t size define not enough!");

#elif defined(CAM_OS_LINUX_USER)
#define _GNU_SOURCE
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/time.h>
#include <mdrv_msys_io.h>
#include <mdrv_msys_io_st.h>
#include "mdrv_verchk.h"
#include <sys/mman.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <stdarg.h>
#include "time.h"
#include "cam_os_wrapper.h"
#include "cam_os_util.h"
#include "cam_os_util_list.h"
#include "cam_os_util_bitmap.h"

typedef struct
{
    uint32_t nInited;
    pthread_mutex_t tMutex;
} CamOsMutexLU_t, *pCamOsMutexLU;

typedef struct
{
    uint32_t nInited;
    sem_t tSem;
} CamOsTsemLU_t, *pCamOsTsemLU;

typedef struct
{
    uint32_t nInited;
    pthread_rwlock_t tRwsem;
} CamOsRwsemLU_t, *pCamOsRwsemLU;

typedef struct
{
    uint32_t nInited;
    pthread_mutex_t tMutex;
    pthread_cond_t tCondition;
} CamOsTcondLU_t, *pCamOsTcondLU;

typedef struct
{
    timer_t tTimerID;
} CamOsTimerLU_t, *pCamOsTimerLU;

typedef struct
{
    uint32_t nIdrSize;
    void *pEntryPtr;
} CamOsIdrLU_t, *pCamOsIdrLU;

static pthread_mutex_t _gtSelfInitLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t _gtMemLock = PTHREAD_MUTEX_INITIALIZER;

_Static_assert(sizeof(CamOsMutex_t) >= sizeof(pthread_mutex_t) + 4, "CamOsMutex_t size define not enough! %d");
_Static_assert(sizeof(CamOsTsem_t) >= sizeof(CamOsTsemLU_t), "CamOsTsem_t size define not enough!");
_Static_assert(sizeof(CamOsRwsem_t) >= sizeof(CamOsRwsemLU_t), "CamOsRwsem_t size define not enough!");
_Static_assert(sizeof(CamOsTcond_t) >= sizeof(CamOsTcondLU_t), "CamOsTcond_t size define not enough!");
_Static_assert(sizeof(CamOsTimer_t) >= sizeof(CamOsTimerLU_t), "CamOsTimer_t size define not enough!");
_Static_assert(sizeof(CamOsTimespec_t) == sizeof(struct timespec), "CamOsTimespec_t size define error!");
_Static_assert(sizeof(CamOsIdr_t) >= sizeof(CamOsIdrLU_t), "CamOsIdr_t size define not enough!");

#elif defined(CAM_OS_LINUX_KERNEL)
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/time.h>
#include <linux/math64.h>
#include <linux/mm.h>
#include <linux/wait.h>
#include <linux/vmalloc.h>
#include <linux/list.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/scatterlist.h>
//#include <linux/slab_def.h>       // comment because of struct redifined
#include <linux/idr.h>
#include <linux/hardirq.h>
#include <asm/io.h>
#include <asm/cacheflush.h>
#include <asm/uaccess.h>
#include <ms_msys.h>
#include <ms_platform.h>
#include "cam_os_wrapper.h"
#include "cam_os_util.h"
#include "cam_os_util_list.h"
#include "cam_os_util_bitmap.h"

#define CAM_OS_THREAD_STACKSIZE_DEFAULT         8192

typedef struct
{
    uint32_t nInited;
    struct mutex tMutex;
} CamOsMutexLK_t, *pCamOsMutexLK;

typedef struct
{
    uint32_t nInited;
    struct semaphore tSem;
} CamOsTsemLK_t, *pCamOsTsemLK;

typedef struct
{
    uint32_t nInited;
    struct rw_semaphore tRwsem;
} CamOsRwsemLK_t, *pCamOsRwsemLK;

typedef struct
{
    uint32_t nInited;
    wait_queue_head_t tWqHead;
    uint8_t nWakeUpFlag; // 0: not wake up, 1: wake up one, >=2: wake up all
} CamOsTcondLK_t, *pCamOsTcondLK;

typedef struct
{
    struct timer_list tTimerID;
} CamOsTimerLK_t, *pCamOsTimerLK;

typedef struct
{
    struct kmem_cache *ptKmemCache;
} CamOsMemCacheLK_t, *pCamOsMemCacheLK;

typedef struct
{
    struct idr tIdr;
} CamOsIdrLK_t, *pCamOsIdrLK;

typedef int32_t CamOsThreadEntry_t(void *);

extern int msys_find_dmem_by_phys(unsigned long long phys, MSYS_DMEM_INFO* pdmem);

static DEFINE_MUTEX(_gtSelfInitLock);
static DEFINE_MUTEX(_gtMemLock);

_Static_assert(sizeof(CamOsMutex_t) >= sizeof(struct mutex) + 4, "CamOsMutex_t size define not enough! %d");
_Static_assert(sizeof(CamOsTsem_t) >= sizeof(CamOsTsemLK_t), "CamOsTsem_t size define not enough!");
_Static_assert(sizeof(CamOsRwsem_t) >= sizeof(CamOsRwsemLK_t), "CamOsRwsem_t size define not enough!");
_Static_assert(sizeof(CamOsTcond_t) >= sizeof(CamOsTcondLK_t), "CamOsTcond_t size define not enough!");
_Static_assert(sizeof(CamOsTimespec_t) == sizeof(struct timespec), "CamOsTimespec_t size define error!");
_Static_assert(sizeof(CamOsTimer_t) >= sizeof(CamOsTimerLK_t), "CamOsTimer_t size define not enough!");
_Static_assert(sizeof(CamOsMemCache_t) >= sizeof(CamOsMemCacheLK_t), "CamOsMemCache_t size define not enough!");
_Static_assert(sizeof(CamOsAtomic_t) == sizeof(atomic_t), "CamOsAtomic_t size define not enough!");
_Static_assert(sizeof(CamOsIdr_t) >= sizeof(CamOsIdrLK_t), "CamOsIdr_t size define not enough!");

#endif

#define RIU_BASE_ADDR           0x1F000000
#define RIU_MEM_SIZE_OFFSET     0x2025A4
#define RIU_CHIP_ID_OFFSET      0x003C00

#define ASSIGN_POINTER_VALUE(a, b) if((a))*(a)=(b)

#define INIT_MAGIC_NUM          0x55AA5AA5

typedef struct MemoryList_t
{
    struct CamOsListHead_t tList;
    void *pPtr;
    void *pMemifoPtr;
    char *szName;
    uint32_t  nSize;
} MemoryList_t;

static MemoryList_t _gtMemList;

static int32_t _gnDmemDbgListInited = 0;

char *CamOsVersion(void)
{
    return CAM_OS_WRAPPER_VERSION;
}

void CamOsPrintf(const char *szFmt, ...)
{
#ifdef CAM_OS_RTK
    va_list tArgs;
    char nLineStr[256];

    va_start(tArgs, szFmt);
    vsprintf(nLineStr, szFmt, tArgs);
    send_msg(nLineStr);
    va_end(tArgs);
#elif defined(CAM_OS_LINUX_USER)
    va_list tArgs;

    va_start(tArgs, szFmt);
    vfprintf(stderr, szFmt, tArgs);
    va_end(tArgs);
#elif defined(CAM_OS_LINUX_KERNEL)
    va_list tArgs;

    va_start(tArgs, szFmt);
    vprintk(szFmt, tArgs);
    va_end(tArgs);
#endif
}

#ifdef CAM_OS_RTK
static char* _CamOsAdvance(char* pBuf)
{

    char* pNewBuf = pBuf;

    /* Skip over nonwhite space */
    while((*pNewBuf != ' ')  && (*pNewBuf != '\t') &&
            (*pNewBuf != '\n') && (*pNewBuf != '\0'))
    {
        pNewBuf++;
    }

    /* Skip white space */
    while((*pNewBuf == ' ')  || (*pNewBuf == '\t') ||
            (*pNewBuf == '\n') || (*pNewBuf == '\0'))
    {
        pNewBuf++;
    }

    return pNewBuf;
}

static int32_t _CamOsVsscanf(char* szBuf, char* szFmt, va_list tArgp)
{
    char*    pFmt;
    char*    pBuf;
    char*    pnSval;
    uint32_t*     pnU32Val;
    int32_t*     pnS32Val;
    uint64_t*     pnU64Val;
    int64_t*     pnS64Val;
    double*  pdbDval;
    float*   pfFval;
    int32_t      nCount = 0;

    pBuf = szBuf;

    for(pFmt = szFmt; *pFmt; pFmt++)
    {
        if(*pFmt == '%')
        {
            pFmt++;
            if(strncmp(pFmt, "u", 1) == 0)
            {
                pnU32Val = va_arg(tArgp, uint32_t *);
                sscanf(pBuf, "%u", pnU32Val);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else if(strncmp(pFmt, "d", 1) == 0)
            {
                pnS32Val = va_arg(tArgp, int32_t *);
                sscanf(pBuf, "%d", pnS32Val);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else if(strncmp(pFmt, "llu", 3) == 0)
            {
                pnU64Val = va_arg(tArgp, uint64_t *);
                sscanf(pBuf, "%llu", pnU64Val);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else if(strncmp(pFmt, "lld", 3) == 0)
            {
                pnS64Val = va_arg(tArgp, int64_t *);
                sscanf(pBuf, "%lld", pnS64Val);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else if(strncmp(pFmt, "f", 1) == 0)
            {
                pfFval = va_arg(tArgp, float *);
                sscanf(pBuf, "%f", pfFval);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else if(strncmp(pFmt, "lf", 2) == 0)
            {
                pdbDval = va_arg(tArgp, double *);
                sscanf(pBuf, "%lf", pdbDval);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else if(strncmp(pFmt, "s", 1) == 0)
            {
                pnSval = va_arg(tArgp, char *);
                sscanf(pBuf, "%s", pnSval);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else
            {
                CamOsPrintf("_CamOsVsscanf error: unsupported format (\%%s)\n", pFmt);
            }
        }
    }

    return nCount;
}

static int32_t _CamOsGetString(char* szBuf, int32_t nMaxLen,  int32_t nEcho)
{
    int32_t nLen;
    static char ch = '\0';

    nLen = 0;
    while(1)
    {
        szBuf[nLen] = get_char();

        // To ignore one for (\r,\n) or (\n, \r) pair
        if((szBuf[nLen] == '\n' && ch == '\r') || (szBuf[nLen] == '\r' && ch == '\n'))
        {
            ch = '\0';
            continue;
        }
        ch = szBuf[nLen];
        if(ch == '\n' || ch == '\r')
        {
            if(nEcho)
                CamOsPrintf("\n");
            break;
        }
        if(nLen < (nMaxLen - 1))
        {
            if(ch == '\b')  /* Backspace? */
            {
                if(nLen <= 0)
                    CamOsPrintf("\007");
                else
                {
                    CamOsPrintf("\b \b");
                    nLen --;
                }
                continue;
            }
            nLen++;
        }
        if(nEcho)
            CamOsPrintf("%c", ch);
    }
    szBuf[nLen] = '\0';
    return nLen;
}

static int32_t _CamOsVfscanf(const char *szFmt, va_list tArgp)
{
    int32_t nCount;
    char szCommandBuf[128];

    _CamOsGetString(szCommandBuf, sizeof(szCommandBuf), 1);

    nCount = _CamOsVsscanf(szCommandBuf, (char *)szFmt, tArgp);
    return nCount;
}
#endif

int32_t CamOsScanf(const char *szFmt, ...)
{
#ifdef CAM_OS_RTK
    int32_t nCount = 0;
    va_list tArgp;

    va_start(tArgp, szFmt);
    nCount = _CamOsVfscanf(szFmt, tArgp);
    va_end(tArgp);
    return nCount;
#elif defined(CAM_OS_LINUX_USER)
    int32_t nCount = 0;
    va_list tArgp;

    va_start(tArgp, szFmt);
    nCount = vfscanf(stdin, szFmt, tArgp);
    va_end(tArgp);
    return nCount;
#elif defined(CAM_OS_LINUX_KERNEL)
    return 0;
#endif
}

int32_t CamOsGetChar(void)
{
#ifdef CAM_OS_RTK
    int32_t Ret;
    Ret = get_char();
    CamOsPrintf("\n");
    return Ret;
#elif defined(CAM_OS_LINUX_USER)
    return getchar();
#elif defined(CAM_OS_LINUX_KERNEL)
    return 0;
#endif
}

int32_t CamOsSnprintf(char *szBuf, uint32_t nSize, const char *szFmt, ...)
{
    va_list tArgs;
    int32_t i;

    va_start(tArgs, szFmt);
    i = vsnprintf(szBuf, nSize, szFmt, tArgs);
    va_end(tArgs);

    return i;
}

void CamOsMsSleep(uint32_t nMsec)
{
#ifdef CAM_OS_RTK
    MsSleep(RTK_MS_TO_TICK(nMsec));
#elif defined(CAM_OS_LINUX_USER)
    usleep((useconds_t)nMsec * 1000);
#elif defined(CAM_OS_LINUX_KERNEL)
    msleep(nMsec);
#endif
}

void CamOsGetTimeOfDay(CamOsTimespec_t *ptRes)
{
#ifdef CAM_OS_RTK
    if(ptRes)
    {
        SysTimeGetUTCSeconds(&ptRes->nSec);
        ptRes->nNanoSec = 0;
    }
#elif defined(CAM_OS_LINUX_USER)
    struct timeval tTV;
    if(ptRes)
    {
        gettimeofday(&tTV, NULL);
        ptRes->nSec = tTV.tv_sec;
        ptRes->nNanoSec = tTV.tv_usec * 1000;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    struct timeval tTv;
    if(ptRes)
    {
        do_gettimeofday(&tTv);
        ptRes->nSec = tTv.tv_sec;
        ptRes->nNanoSec = tTv.tv_usec * 1000;
    }
#endif
}

void CamOsSetTimeOfDay(const CamOsTimespec_t *ptRes)
{
#ifdef CAM_OS_RTK
    struct tm * tTm;
    vm_rtcTimeFormat_t tLocalTime;
    if(ptRes)
    {
        tTm = localtime((time_t *)&ptRes->nSec);

        tLocalTime.Year = tTm->tm_year;
        tLocalTime.Month = tTm->tm_mon + 1;
        tLocalTime.Day = tTm->tm_mday;
        tLocalTime.DayOfWeek = tTm->tm_wday ? tTm->tm_wday : 7;
        tLocalTime.Hour = tTm->tm_hour;
        tLocalTime.Minute = tTm->tm_min;
        tLocalTime.Second = tTm->tm_sec;

        SysTimeSetTime(&tLocalTime);
    }
#elif defined(CAM_OS_LINUX_USER)
    struct timeval tTV;
    if(ptRes)
    {
        tTV.tv_sec = ptRes->nSec;
        tTV.tv_usec = ptRes->nNanoSec / 1000;
        settimeofday(&tTV, NULL);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    struct timespec tTs;
    if(ptRes)
    {
        tTs.tv_sec = ptRes->nSec;
        tTs.tv_nsec = ptRes->nNanoSec;
        do_settimeofday(&tTs);
    }
#endif
}

void CamOsGetMonotonicTime(CamOsTimespec_t *ptRes)
{
#ifdef CAM_OS_RTK
#ifdef __RTK_US_RESOLUTION_TIMER__
    uint64_t nUs;
    if(ptRes)
    {
        nUs = DrvTimerStdaTimerTick2UsEx(RtkGetOsTickExt());
        ptRes->nSec = nUs / 1000000;
        ptRes->nNanoSec = (nUs % 1000000) * 1000;
    }
#else
    uint32_t nMs;
    if(ptRes)
    {
        nMs = VM_RTK_TICK_TO_MS(MsGetOsTick());
        ptRes->nSec = nMs / 1000;
        ptRes->nNanoSec = (nMs % 1000) * 1000000;
    }
#endif
#elif defined(CAM_OS_LINUX_USER)
    clock_gettime(CLOCK_MONOTONIC, (struct timespec *)ptRes);
#elif defined(CAM_OS_LINUX_KERNEL)
    getrawmonotonic((struct timespec *)ptRes);
#endif
}

#ifdef CAM_OS_RTK
static void _CamOSThreadEntry(void *pEntryData)
{
    CamOsThreadHandleRtk_t *ptTaskHandle = (CamOsThreadHandleRtk_t *)pEntryData;

    ptTaskHandle->pfnEntry(ptTaskHandle->pArg);

    MsFlagSetbits(&ptTaskHandle->tExitFlag, 0x00000001);
}

static void _CamOsThreadEmptyParser(vm_msg_t *ptMessage)
{

}
#endif

CamOsRet_e CamOsThreadCreate(CamOsThread *ptThread,
                             CamOsThreadAttrb_t *ptAttrb,
                             void *(*pfnStartRoutine)(void *),
                             void *pArg)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsThreadHandleRtk_t *ptTaskHandle = NULL;
    MsTaskCreateArgs_t tTaskArgs = {0};
    uint32_t nPrio = 100;
    uint32_t nStkSz = CAM_OS_THREAD_STACKSIZE_DEFAULT;

    if(ptAttrb != NULL)
    {
        if((ptAttrb->nPriority > 0) && (ptAttrb->nPriority < 100))
        {
            nPrio = ptAttrb->nPriority * 2;
        }
        nStkSz = (ptAttrb->nStackSize) ? ptAttrb->nStackSize : CAM_OS_THREAD_STACKSIZE_DEFAULT;
    }

    *ptThread = (void*) - 1;
    do
    {
        if(!(ptTaskHandle = MsCallocateMem(sizeof(CamOsThreadHandleRtk_t))))
        {
            CamOsPrintf("%s : Allocate ptHandle fail\n\r", __FUNCTION__);
            eRet = CAM_OS_ALLOCMEM_FAIL;
            break;
        }

        ptTaskHandle->pfnEntry = pfnStartRoutine;
        ptTaskHandle->pArg   = pArg;
        if(!(ptTaskHandle->pStack = MsAllocateMem((nStkSz) ? nStkSz : CAM_OS_THREAD_STACKSIZE_DEFAULT)))
        {
            CamOsPrintf("%s : Allocate stack fail\n\r", __FUNCTION__);
            eRet = CAM_OS_ALLOCMEM_FAIL;
            break;
        }

        memset(&ptTaskHandle->tExitFlag, 0, sizeof(Ms_Flag_t));
        MsFlagInit(&ptTaskHandle->tExitFlag);
        //VEN_TEST_CHECK_RESULT((pTaskHandle->exit_flag.FlagId >> 0)  && (pTaskHandle->exit_flag.FlagState == RTK_FLAG_INITIALIZED));

        tTaskArgs.Priority = (nPrio >= 0 && nPrio <= 200) ? nPrio : 100;
        tTaskArgs.StackSize = (nStkSz) ? nStkSz : CAM_OS_THREAD_STACKSIZE_DEFAULT;
        tTaskArgs.pStackTop = (uint32_t*)ptTaskHandle->pStack;
        tTaskArgs.AppliInit = &_CamOSThreadEntry;
        tTaskArgs.AppliParser = _CamOsThreadEmptyParser;
        tTaskArgs.pInitArgs = ptTaskHandle;
        tTaskArgs.TaskId = &ptTaskHandle->eHandleObj;
        tTaskArgs.ImmediatelyStart = TRUE;
        tTaskArgs.TimeSliceMax = 10;
        tTaskArgs.TimeSliceLeft = 10;
        strncpy(tTaskArgs.TaskName, "CamOsWrp", sizeof(tTaskArgs.TaskName));

        if(MS_OK != MsCreateTask(&tTaskArgs))
        {
            CamOsPrintf("%s : Create task fail\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        *ptThread = ptTaskHandle;
    }
    while(0);

    if(!*ptThread)
    {
        if(ptTaskHandle)
        {
            if(ptTaskHandle->pStack)
            {
                MsReleaseMemory(ptTaskHandle->pStack);
            }
            MsFlagDestroy(&ptTaskHandle->tExitFlag);
            MsReleaseMemory(ptTaskHandle);
        }
    }

    //CamOsPrintf("%s get taskid: %d(%s)  priority: %d\n\r", __FUNCTION__, (uint32_t)ptTaskHandle->eHandleObj, tTaskArgs.TaskName, tTaskArgs.Priority);
#elif defined(CAM_OS_LINUX_USER)
    struct sched_param tSched;
    pthread_t tThreadHandle = NULL;
    pthread_attr_t tAttr;

    if(ptAttrb != NULL)
    {
        do
        {
            pthread_attr_init(&tAttr);

            if((ptAttrb->nPriority > 0) && (ptAttrb->nPriority < 100))
            {
                pthread_attr_getschedparam(&tAttr, &tSched);
                pthread_attr_setinheritsched(&tAttr, PTHREAD_EXPLICIT_SCHED);
                pthread_attr_setschedpolicy(&tAttr, SCHED_RR);
                tSched.sched_priority = ptAttrb->nPriority;
                if(0 != pthread_attr_setschedparam(&tAttr, &tSched))
                {
                    CamOsPrintf("%s: pthread_attr_setschedparam failed\n\r", __FUNCTION__);
                    eRet = CAM_OS_FAIL;
                    break;
                }
            }

            if(0 != ptAttrb->nStackSize)
            {
                if(0 != pthread_attr_setstacksize(&tAttr, (size_t) ptAttrb->nStackSize))
                {
                    eRet = CAM_OS_FAIL;
                    CamOsPrintf("%s pthread_attr_setstacksize failed\n\r", __FUNCTION__);
                    break;
                }
            }
            pthread_create(&tThreadHandle, &tAttr, pfnStartRoutine, pArg);
        }
        while(0);
        pthread_attr_destroy(&tAttr);
    }
    else
    {
        pthread_create(&tThreadHandle, NULL, pfnStartRoutine, pArg);
    }

    *ptThread = (CamOsThread *)tThreadHandle;
    //CamOsPrintf("%s: done(%d)\n\r", __FUNCTION__, eRet);

#elif defined(CAM_OS_LINUX_KERNEL)
    struct task_struct *tpThreadHandle;
    struct sched_param tSched = { .sched_priority = 0 };
    uint32_t nStkSz = CAM_OS_THREAD_STACKSIZE_DEFAULT;

    if(ptAttrb != NULL)
    {
        if((ptAttrb->nPriority > 0) && (ptAttrb->nPriority < 100))
        {
            tSched.sched_priority = ptAttrb->nPriority;
        }
        nStkSz = ptAttrb->nStackSize;
    }
    tpThreadHandle = kthread_run((CamOsThreadEntry_t *)pfnStartRoutine, pArg, "CAMOS");
    sched_setscheduler(tpThreadHandle, SCHED_RR, &tSched);
    *ptThread = (CamOsThread *)tpThreadHandle;
#endif

    return eRet;
}
// [FIX ME]:sched_setscheduler seems not to work after pthread run in Linux
//          kernel and user space
#if 0
CamOsRet_e CamOsThreadChangePriority(CamOsThread tThread, uint32_t nPriority)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsThreadHandleRtk_t *ptTaskHandle = (CamOsThreadHandleRtk_t *)tThread;
    uint32_t nPrio = 100;
    if(ptTaskHandle)
    {
        if(nPriority > 0 && nPriority < 100)
        {
            nPrio = nPriority * 2;
        }
        MsChangeTaskPriority(ptTaskHandle->eHandleObj, (nPrio >= 0 && nPrio <= 200) ? nPrio : 100);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    // [FIX ME]:sched_setscheduler seems not to work after pthread run
    pthread_t tThreadHandle = (pthread_t)tThread;
    struct sched_param tSched;
    int policy;
    if(tThreadHandle && nPriority > 0 && nPriority < 100)
    {
        pthread_getschedparam(tThreadHandle, &policy, &tSched);
        tSched.sched_priority = nPriority;

        if(0 != pthread_setschedparam(tThreadHandle, policy, &tSched))
        {
            CamOsPrintf("%s: pthread_setschedparam failed\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    // [FIX ME]:sched_setscheduler seems not to work after kthread run
    struct task_struct *tpThreadHandle = (struct task_struct *)tThread;
    struct sched_param tSched = { .sched_priority = 0 };
    if(tpThreadHandle && nPriority > 0 && nPriority < 100)
    {
        tSched.sched_priority = nPriority;

        if(0 != sched_setscheduler(tpThreadHandle, SCHED_RR, &tSched))
        {
            CamOsPrintf("%s : pthread_setschedparam failed\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}
#endif
CamOsRet_e CamOsThreadSchedule(bool bInterruptible, uint32_t nMsec)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    if(nMsec == CAM_OS_MAX_TIMEOUT)
        MsSelfYield();//MsSuspendTask(1, MsCurrTask());
    else if(nMsec > 0 && CAM_OS_OK != MsSleep(RTK_MS_TO_TICK(nMsec)))
        eRet = CAM_OS_TIMEOUT;
#elif defined(CAM_OS_LINUX_USER)
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    if(nMsec == CAM_OS_MAX_TIMEOUT)
    {
        if(bInterruptible)
            set_current_state(TASK_INTERRUPTIBLE);
        else
            set_current_state(TASK_UNINTERRUPTIBLE);

        schedule();
    }
    else if(nMsec > 0)
    {
        if(bInterruptible)
        {
            if(0 == schedule_timeout(msecs_to_jiffies(nMsec) + 1))
                eRet = CAM_OS_TIMEOUT;
        }
        else
        {
            schedule_timeout_uninterruptible(msecs_to_jiffies(nMsec) + 1);
        }
    }
#endif
    return eRet;
}

CamOsRet_e CamOsThreadWakeUp(CamOsThread tThread)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsThreadHandleRtk_t *ptTaskHandle = (CamOsThreadHandleRtk_t *)tThread;
    if(ptTaskHandle)
    {
        MsResumeTask(1, ptTaskHandle->eHandleObj);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct task_struct *tpThreadHandle = (struct task_struct *)tThread;
    if(tpThreadHandle)
    {
        wake_up_process(tpThreadHandle);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsThreadJoin(CamOsThread tThread)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsThreadHandleRtk_t *ptTaskHandle = (CamOsThreadHandleRtk_t *)tThread;
    if(ptTaskHandle)
    {
        MsFlagWait(&ptTaskHandle->tExitFlag, 0x00000001, RTK_FLAG_WAITMODE_AND | RTK_FLAG_WAITMODE_CLR);
        MsFlagDestroy(&ptTaskHandle->tExitFlag);
        MsDeleteTask(ptTaskHandle->eHandleObj);

        if(ptTaskHandle->pStack)
        {
            MsReleaseMemory(ptTaskHandle->pStack);
        }
        MsReleaseMemory(ptTaskHandle);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    pthread_t tThreadHandle = (pthread_t)tThread;
    if(tThreadHandle)
    {
        pthread_join(tThreadHandle, NULL);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
    eRet = CAM_OS_FAIL;
#endif
    return eRet;
}

CamOsRet_e CamOsThreadStop(CamOsThread tThread)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_USER)
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct task_struct *tpThreadHandle = (struct task_struct *)tThread;
    int32_t nErr;
    if(tpThreadHandle)
    {
        if(0 != (nErr = kthread_stop(tpThreadHandle)))
        {
            CamOsPrintf("%s : kthread_stop failed(nErr=%d)\n\r", __FUNCTION__, nErr);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsThreadShouldStop(void)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_USER)
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    if(kthread_should_stop())
    {
        eRet = CAM_OS_OK;
    }
    else
    {
        eRet = CAM_OS_FAIL;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsThreadSetName(CamOsThread tThread, const char *szName)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsThreadHandleRtk_t *ptTaskHandle = (CamOsThreadHandleRtk_t *)tThread;
    if(ptTaskHandle && szName)
    {
        MsSetTaskName(ptTaskHandle->eHandleObj, szName);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    pthread_t tThreadHandle = (pthread_t)tThread;
    if(tThreadHandle)
    {
        if(strlen(szName) >= 16) // Linux limitation
            return CAM_OS_PARAM_ERR;
#if defined(__GLIBC__) && defined(__GLIBC_PREREQ) && __GLIBC_PREREQ(2, 12)
        if(pthread_setname_np(tThreadHandle, szName) != 0)
        {
            eRet = CAM_OS_PARAM_ERR;
        }
#else
        if(tThreadHandle == pthread_self())
        {
            if(prctl(PR_SET_NAME, szName) != 0)
                eRet = CAM_OS_PARAM_ERR;
        }
        else
        {
            CamOsPrintf("%s: Only support set thread name by itself in this libc(uclibc?)\n\r", __FUNCTION__);
        }
#endif
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
    eRet = CAM_OS_FAIL;
#endif

    return eRet;
}

CamOsRet_e CamOsThreadGetName(CamOsThread tThread, char *szName, uint32_t nLen)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsThreadHandleRtk_t *ptTaskHandle = (CamOsThreadHandleRtk_t *)tThread;
    if(ptTaskHandle && szName)
    {
        MsGetTaskName(ptTaskHandle->eHandleObj, szName, nLen);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    pthread_t tThreadHandle = (pthread_t)tThread;
    if(tThreadHandle)
    {
        if(nLen < 16) // Linux limitation
            return CAM_OS_PARAM_ERR;
#if defined(__GLIBC__) && defined(__GLIBC_PREREQ) && __GLIBC_PREREQ(2, 12)
        if(pthread_getname_np(tThreadHandle, szName, nLen) != 0)
        {
            eRet = CAM_OS_PARAM_ERR;
        }
#else
        if(tThreadHandle == pthread_self())
        {
            if(prctl(PR_GET_NAME, szName) != 0)
                eRet = CAM_OS_PARAM_ERR;
        }
        else
        {
            CamOsPrintf("%s: Only support get thread name by itself in this libc(uclibc?)\n\r", __FUNCTION__);
        }
#endif
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
    eRet = CAM_OS_FAIL;
#endif
    return eRet;
}

uint32_t CamOsThreadGetID()
{
#ifdef CAM_OS_RTK
    return MsCurrTask();
#elif defined(CAM_OS_LINUX_USER)
    return (uint32_t)syscall(__NR_gettid);
#elif defined(CAM_OS_LINUX_KERNEL)
    return current->tgid;
#endif
}

CamOsRet_e CamOsMutexInit(CamOsMutex_t *ptMutex)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsMutexRtk_t *ptHandle = (CamOsMutexRtk_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            MsMutexLock(&_gtSelfInitLock);

            if(CUS_OS_OK != MsInitMutex(&ptHandle->tMutex))
            {
                CamOsPrintf("%s : Init mutex fail\n\r", __FUNCTION__);
                eRet = CAM_OS_FAIL;
            }
            else
                ptHandle->nInited = INIT_MAGIC_NUM;

            MsMutexUnlock(&_gtSelfInitLock);
        }
        else
        {
            CamOsPrintf("%s : Mutex was already inited\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsMutexLU_t *ptHandle = (CamOsMutexLU_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            pthread_mutex_lock(&_gtSelfInitLock);

            if(0 != pthread_mutex_init(&ptHandle->tMutex, NULL))
            {
                fprintf(stderr, "%s : Init mutex fail\n\r", __FUNCTION__);
                eRet = CAM_OS_FAIL;
            }
            else
                ptHandle->nInited = INIT_MAGIC_NUM;

            pthread_mutex_unlock(&_gtSelfInitLock);
        }
        else
        {
            CamOsPrintf("%s : Mutex was already inited\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMutexLK_t *ptHandle = (CamOsMutexLK_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            mutex_lock(&_gtSelfInitLock);

            mutex_init(&ptHandle->tMutex);
            ptHandle->nInited = INIT_MAGIC_NUM;

            mutex_unlock(&_gtSelfInitLock);
        }
        else
        {
            CamOsPrintf("%s : Mutex was already inited\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsMutexDestroy(CamOsMutex_t *ptMutex)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsMutexRtk_t *ptHandle = (CamOsMutexRtk_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Mutex not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else
            ptHandle->nInited = 0;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsMutexLU_t *ptHandle = (CamOsMutexLU_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Mutex not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else
        {
            pthread_mutex_destroy(&ptHandle->tMutex);
            ptHandle->nInited = 0;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMutexLK_t *ptHandle = (CamOsMutexLK_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Mutex not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else
            ptHandle->nInited = 0;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsMutexLock(CamOsMutex_t *ptMutex)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsMutexRtk_t *ptHandle = (CamOsMutexRtk_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Mutex not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else if(CUS_OS_OK != MsMutexLock(&ptHandle->tMutex))
        {
            CamOsPrintf("%s : Lock mutex fail\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    int32_t nErr = 0;
    CamOsMutexLU_t *ptHandle = (CamOsMutexLU_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Mutex not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else if(0 != (nErr = pthread_mutex_lock(&ptHandle->tMutex)))
        {
            fprintf(stderr, "%s : Lock mutex fail, err %d\n\r", __FUNCTION__, nErr);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMutexLK_t *ptHandle = (CamOsMutexLK_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Mutex not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else
            mutex_lock(&ptHandle->tMutex);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsMutexTryLock(CamOsMutex_t *ptMutex)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsMutexRtk_t *ptHandle = (CamOsMutexRtk_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Mutex not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else if(CUS_OS_UNIT_NOAVAIL == MsMutexTryLock(&ptHandle->tMutex))
        {
            eRet = CAM_OS_RESOURCE_BUSY;
        }
        else
        {
            CamOsPrintf("%s : MsMutexLock failed\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    int32_t nErr = 0;
    CamOsMutexLU_t *ptHandle = (CamOsMutexLU_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Mutex not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else if(0 != (nErr = pthread_mutex_trylock(&ptHandle->tMutex)))
        {
            if(nErr == EAGAIN)
            {
                eRet = CAM_OS_RESOURCE_BUSY;
            }
            else
            {
                fprintf(stderr, "%s : Lock mutex fail, err %d\n\r", __FUNCTION__, nErr);
                eRet = CAM_OS_FAIL;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMutexLK_t *ptHandle = (CamOsMutexLK_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Mutex not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else if(0 != mutex_trylock(&ptHandle->tMutex))
            eRet = CAM_OS_RESOURCE_BUSY;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsMutexUnlock(CamOsMutex_t *ptMutex)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsMutexRtk_t *ptHandle = (CamOsMutexRtk_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Mutex not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else if(CUS_OS_OK != MsMutexUnlock(&ptHandle->tMutex))
        {
            CamOsPrintf("%s : Unlock mutex fail\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    int32_t nErr = 0;
    CamOsMutexLU_t *ptHandle = (CamOsMutexLU_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Mutex not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else if(0 != (nErr = pthread_mutex_unlock(&ptHandle->tMutex)))
        {
            fprintf(stderr, "%s : Unlock mutex fail, err %d\n\r", __FUNCTION__, nErr);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMutexLK_t *ptHandle = (CamOsMutexLK_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Mutex not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else
            mutex_unlock(&ptHandle->tMutex);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsTsemInit(CamOsTsem_t *ptTsem, uint32_t nVal)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            MsMutexLock(&_gtSelfInitLock);

            if(CUS_OS_OK != MsCreateDynSem(&ptHandle->pTsem, nVal))
            {
                CamOsPrintf("%s : Init semaphore fail\n\r", __FUNCTION__);
                eRet = CAM_OS_FAIL;
            }
            else
                ptHandle->nInited = INIT_MAGIC_NUM;

            MsMutexUnlock(&_gtSelfInitLock);
        }
        else
        {
            CamOsPrintf("%s : Semaphore was already inited\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            pthread_mutex_lock(&_gtSelfInitLock);

            if(0 != sem_init(&ptHandle->tSem, 1, nVal))
            {
                CamOsPrintf("%s : Init semaphore fail\n\r", __FUNCTION__);
                eRet = CAM_OS_FAIL;
            }
            else
                ptHandle->nInited = INIT_MAGIC_NUM;

            pthread_mutex_unlock(&_gtSelfInitLock);
        }
        else
        {
            CamOsPrintf("%s : Semaphore was already inited\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            mutex_lock(&_gtSelfInitLock);

            sema_init(&ptHandle->tSem, nVal);
            ptHandle->nInited = INIT_MAGIC_NUM;

            mutex_unlock(&_gtSelfInitLock);
        }
        else
        {
            CamOsPrintf("%s : Semaphore was already inited\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsTsemDeinit(CamOsTsem_t *ptTsem)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Semaphore not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else
        {
            MsDestroyDynSem(ptHandle->pTsem);
            ptHandle->nInited = 0;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Semaphore not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else if(0 != sem_destroy(&ptHandle->tSem))
        {
            ptHandle->nInited = 0;
            eRet = CAM_OS_FAIL;
        }
        else
            ptHandle->nInited = 0;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Semaphore not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else
            ptHandle->nInited = 0;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

void CamOsTsemUp(CamOsTsem_t *ptTsem)
{
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Semaphore not init\n\r", __FUNCTION__);
        else
            MsProduceDynSem(ptHandle->pTsem);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Semaphore not init\n\r", __FUNCTION__);
        else
            sem_post(&ptHandle->tSem);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Semaphore not init\n\r", __FUNCTION__);
        else
            up(&ptHandle->tSem);
    }
#endif
}

void CamOsTsemDown(CamOsTsem_t *ptTsem)
{
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Semaphore not init\n\r", __FUNCTION__);
        else
            MsConsumeDynSem(ptHandle->pTsem);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Semaphore not init\n\r", __FUNCTION__);
        else
            sem_wait(&ptHandle->tSem);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Semaphore not init\n\r", __FUNCTION__);
        else
            down(&ptHandle->tSem);
    }
#endif
}

void CamOsTsemDownInterruptible(CamOsTsem_t *ptTsem)
{
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Semaphore not init\n\r", __FUNCTION__);
        else
            MsConsumeDynSem(ptHandle->pTsem);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Semaphore not init\n\r", __FUNCTION__);
        else
            sem_wait(&ptHandle->tSem);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Semaphore not init\n\r", __FUNCTION__);
        else if(-EINTR != down_interruptible(&ptHandle->tSem))
            CamOsPrintf("down_interruptible is interrupted!\n");
    }
#endif
}

CamOsRet_e CamOsTsemTimedDown(CamOsTsem_t *ptTsem, uint32_t nMsec)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Semaphore not init\n\r", __FUNCTION__);
        else if(CUS_OS_NO_MESSAGE == MsConsumeDynSemDelay(ptHandle->pTsem, RTK_MS_TO_TICK(nMsec)))
            eRet = CAM_OS_TIMEOUT;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    struct timespec tFinalTime;
    int64_t nNanoDelay = 0;

    if(ptHandle)
    {
        if(clock_gettime(CLOCK_REALTIME, &tFinalTime) == -1)
            CamOsPrintf("%s : clock_gettime fail\n\r", __FUNCTION__);

        nNanoDelay = (nMsec * 1000000LL) + tFinalTime.tv_nsec;
        tFinalTime.tv_sec += (nNanoDelay / 1000000000LL);
        tFinalTime.tv_nsec = nNanoDelay % 1000000000LL;

        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Semaphore not init\n\r", __FUNCTION__);
        else if(0 != sem_timedwait(&ptHandle->tSem, &tFinalTime))
        {
            if(errno == ETIMEDOUT)
            {
                eRet = CAM_OS_TIMEOUT;
            }
            else
            {
                CamOsPrintf("%s : sem_timedwait failed(errno=%d)\n\r", __FUNCTION__, errno);
                eRet = CAM_OS_FAIL;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Semaphore not init\n\r", __FUNCTION__);
        else if(0 != down_timeout(&ptHandle->tSem, msecs_to_jiffies(nMsec)))
            eRet = CAM_OS_TIMEOUT;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsTsemTryDown(CamOsTsem_t *ptTsem)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Semaphore not init\n\r", __FUNCTION__);
        else if(CUS_OS_UNIT_NOAVAIL == MsPollDynSem(ptHandle->pTsem))
        {
            eRet = CAM_OS_RESOURCE_BUSY;
        }
        else
        {
            CamOsPrintf("%s : MsPollDynSem failed\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Semaphore not init\n\r", __FUNCTION__);
        else if(0 != sem_trywait(&ptHandle->tSem))
        {
            if(errno == EAGAIN)
            {
                eRet = CAM_OS_RESOURCE_BUSY;
            }
            else
            {
                CamOsPrintf("%s : sem_trywait failed(errno=%d)\n\r", __FUNCTION__, errno);
                eRet = CAM_OS_FAIL;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    int32_t nErr;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Semaphore not init\n\r", __FUNCTION__);
        else if(0 != (nErr = down_trylock(&ptHandle->tSem)))
        {
            CamOsPrintf("%s : down_trylock failed(nErr=%d)\n\r", __FUNCTION__, nErr);
            eRet = CAM_OS_RESOURCE_BUSY;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsRwsemInit(CamOsRwsem_t *ptRwsem)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsRwsemRtk_t *ptHandle = (CamOsRwsemRtk_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else
            ptHandle->nInited = 0;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsRwsemLU_t *ptHandle = (CamOsRwsemLU_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            pthread_mutex_lock(&_gtSelfInitLock);

            if(0 != pthread_rwlock_init(&ptHandle->tRwsem, NULL))
            {
                CamOsPrintf("%s : Init rw semaphore fail\n\r", __FUNCTION__);
                eRet = CAM_OS_FAIL;
            }
            else
                ptHandle->nInited = INIT_MAGIC_NUM;

            pthread_mutex_unlock(&_gtSelfInitLock);
        }
        else
        {
            CamOsPrintf("%s : Rw semaphore was already inited\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsRwsemLK_t *ptHandle = (CamOsRwsemLK_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            mutex_lock(&_gtSelfInitLock);

            init_rwsem(&ptHandle->tRwsem);
            ptHandle->nInited = INIT_MAGIC_NUM;

            mutex_unlock(&_gtSelfInitLock);
        }
        else
        {
            CamOsPrintf("%s : Rw semaphore was already inited\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsRwsemDeinit(CamOsRwsem_t *ptRwsem)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsRwsemRtk_t *ptHandle = (CamOsRwsemRtk_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else
        {
            ptHandle->nInited = 0;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsRwsemLU_t *ptHandle = (CamOsRwsemLU_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else if(0 != pthread_rwlock_destroy(&ptHandle->tRwsem))
        {
            ptHandle->nInited = 0;
            eRet = CAM_OS_FAIL;
        }
        else
            ptHandle->nInited = 0;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsRwsemLK_t *ptHandle = (CamOsRwsemLK_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else
            ptHandle->nInited = 0;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

void CamOsRwsemUpRead(CamOsRwsem_t *ptRwsem)
{
#ifdef CAM_OS_RTK
    CamOsRwsemRtk_t *ptHandle = (CamOsRwsemRtk_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
        else
        {
            MsMutexLock(&ptHandle->tRMutex);
            ptHandle->nReadCount--;
            if(ptHandle->nReadCount == 0)
                MsMutexUnlock(&ptHandle->tWMutex);
            MsMutexUnlock(&ptHandle->tRMutex);
        }
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsRwsemLU_t *ptHandle = (CamOsRwsemLU_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
        else
            pthread_rwlock_unlock(&ptHandle->tRwsem);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsRwsemLK_t *ptHandle = (CamOsRwsemLK_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
        else
            up_read(&ptHandle->tRwsem);
    }
#endif
}

void CamOsRwsemUpWrite(CamOsRwsem_t *ptRwsem)
{
#ifdef CAM_OS_RTK
    CamOsRwsemRtk_t *ptHandle = (CamOsRwsemRtk_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
        else
            MsMutexUnlock(&ptHandle->tWMutex);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsRwsemLU_t *ptHandle = (CamOsRwsemLU_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
        else
            pthread_rwlock_unlock(&ptHandle->tRwsem);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsRwsemLK_t *ptHandle = (CamOsRwsemLK_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
        else
            up_write(&ptHandle->tRwsem);
    }
#endif
}

void CamOsRwsemDownRead(CamOsRwsem_t *ptRwsem)
{
#ifdef CAM_OS_RTK
    CamOsRwsemRtk_t *ptHandle = (CamOsRwsemRtk_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
        else
        {
            MsMutexLock(&ptHandle->tRMutex);
            ptHandle->nReadCount++;
            if(ptHandle->nReadCount == 1)
                MsMutexLock(&ptHandle->tWMutex);
            MsMutexUnlock(&ptHandle->tRMutex);
        }
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsRwsemLU_t *ptHandle = (CamOsRwsemLU_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
        else
            pthread_rwlock_rdlock(&ptHandle->tRwsem);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsRwsemLK_t *ptHandle = (CamOsRwsemLK_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
        else
            down_read(&ptHandle->tRwsem);
    }
#endif
}

void CamOsRwsemDownWrite(CamOsRwsem_t *ptRwsem)
{
#ifdef CAM_OS_RTK
    CamOsRwsemRtk_t *ptHandle = (CamOsRwsemRtk_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
        else
            MsMutexLock(&ptHandle->tWMutex);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsRwsemLU_t *ptHandle = (CamOsRwsemLU_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
        else
            pthread_rwlock_wrlock(&ptHandle->tRwsem);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsRwsemLK_t *ptHandle = (CamOsRwsemLK_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
        else
            down_write(&ptHandle->tRwsem);
    }
#endif
}

CamOsRet_e CamOsRwsemTryDownRead(CamOsRwsem_t *ptRwsem)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsRwsemRtk_t *ptHandle = (CamOsRwsemRtk_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
        else if(CUS_OS_UNIT_NOAVAIL == MsMutexTryLock(&ptHandle->tRMutex))
        {
            eRet = CAM_OS_RESOURCE_BUSY;
        }
        else
        {
            ptHandle->nReadCount++;
            if(ptHandle->nReadCount == 1)
                MsMutexLock(&ptHandle->tWMutex);
            MsMutexUnlock(&ptHandle->tRMutex);
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsRwsemLU_t *ptHandle = (CamOsRwsemLU_t *)ptRwsem;
    int32_t nErr;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
        else if(0 != (nErr = pthread_rwlock_tryrdlock(&ptHandle->tRwsem)))
        {
            if(nErr == EBUSY)
            {
                eRet = CAM_OS_RESOURCE_BUSY;
            }
            else
            {
                CamOsPrintf("%s : pthread_rwlock_tryrdlock failed(nErr=%d)\n\r", __FUNCTION__, nErr);
                eRet = CAM_OS_FAIL;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsRwsemLK_t *ptHandle = (CamOsRwsemLK_t *)ptRwsem;
    int32_t nErr;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
        else if(1 != (nErr = down_read_trylock(&ptHandle->tRwsem)))
        {
            eRet = CAM_OS_RESOURCE_BUSY;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsRwsemTryDownWrite(CamOsRwsem_t *ptRwsem)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsRwsemRtk_t *ptHandle = (CamOsRwsemRtk_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
        else if(CUS_OS_UNIT_NOAVAIL == MsMutexTryLock(&ptHandle->tWMutex))
        {
            eRet = CAM_OS_RESOURCE_BUSY;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsRwsemLU_t *ptHandle = (CamOsRwsemLU_t *)ptRwsem;
    int32_t nErr;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
        else if(0 != (nErr = pthread_rwlock_trywrlock(&ptHandle->tRwsem)))
        {
            if(nErr == EBUSY)
            {
                eRet = CAM_OS_RESOURCE_BUSY;
            }
            else
            {
                CamOsPrintf("%s : pthread_rwlock_trywrlock failed(nErr=%d)\n\r", __FUNCTION__, nErr);
                eRet = CAM_OS_FAIL;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsRwsemLK_t *ptHandle = (CamOsRwsemLK_t *)ptRwsem;
    int32_t nErr;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Rw semaphore not init\n\r", __FUNCTION__);
        else if(1 != (nErr = down_write_trylock(&ptHandle->tRwsem)))
        {
            eRet = CAM_OS_RESOURCE_BUSY;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsTcondInit(CamOsTcond_t *ptTcond)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTcondRtk_t *ptHandle = (CamOsTcondRtk_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            MsMutexLock(&_gtSelfInitLock);

            if(CUS_OS_OK != MsCreateDynSem(&ptHandle->pTsem, 0))
            {
                CamOsPrintf("%s : MsCreateDynSem fail\n\r", __FUNCTION__);
                eRet = CAM_OS_FAIL;
            }
            else
                ptHandle->nInited = INIT_MAGIC_NUM;

            MsMutexUnlock(&_gtSelfInitLock);
        }
        else
        {
            CamOsPrintf("%s : Condition was already inited\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTcondLU_t *ptHandle = (CamOsTcondLU_t *)ptTcond;
    pthread_condattr_t cattr;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            pthread_mutex_lock(&_gtSelfInitLock);

            if(0 != pthread_condattr_init(&cattr) ||
                    0 != pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC))
            {
                CamOsPrintf("%s : pthread_condattr_init failed(errno=%d)\n\r", __FUNCTION__, errno);
                eRet = CAM_OS_FAIL;
            }

            if(0 != pthread_cond_init(&ptHandle->tCondition, &cattr))
            {
                CamOsPrintf("%s : pthread_cond_init failed(errno=%d)\n\r", __FUNCTION__, errno);
                eRet = CAM_OS_FAIL;
            }
            if(0 != pthread_mutex_init(&ptHandle->tMutex, NULL))
            {
                CamOsPrintf("%s : pthread_mutex_init failed(errno=%d)\n\r", __FUNCTION__, errno);
                eRet = CAM_OS_FAIL;
            }
            else
                ptHandle->nInited = INIT_MAGIC_NUM;

            pthread_mutex_unlock(&_gtSelfInitLock);
        }
        else
        {
            CamOsPrintf("%s : Condition was already inited\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            mutex_lock(&_gtSelfInitLock);

            init_waitqueue_head(&ptHandle->tWqHead);
            ptHandle->nWakeUpFlag = 0;
            ptHandle->nInited = INIT_MAGIC_NUM;

            mutex_unlock(&_gtSelfInitLock);
        }
        else
        {
            CamOsPrintf("%s : Condition was already inited\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsTcondDeinit(CamOsTcond_t *ptTcond)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTcondRtk_t *ptHandle = (CamOsTcondRtk_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else
        {
            MsDestroyDynSem(ptHandle->pTsem);
            ptHandle->nInited = 0;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTcondLU_t *ptHandle = (CamOsTcondLU_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else
        {
            pthread_cond_destroy(&ptHandle->tCondition);
            pthread_mutex_destroy(&ptHandle->tMutex);
            ptHandle->nInited = 0;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else
            ptHandle->nInited = 0;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

void CamOsTcondSignal(CamOsTcond_t *ptTcond)
{
#ifdef CAM_OS_RTK
    CamOsTcondRtk_t *ptHandle = (CamOsTcondRtk_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
        else
            MsProduceSafeDynSem(ptHandle->pTsem, 0);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTcondLU_t *ptHandle = (CamOsTcondLU_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
        else
        {
            pthread_mutex_lock(&ptHandle->tMutex);
            pthread_cond_signal(&ptHandle->tCondition);
            pthread_mutex_unlock(&ptHandle->tMutex);
        }
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
        else
        {
            ptHandle->nWakeUpFlag = 1;
            wake_up(&ptHandle->tWqHead);
        }
    }
#endif
}

void CamOsTcondSignalAll(CamOsTcond_t *ptTcond)
{
#ifdef CAM_OS_RTK
    CamOsTcondRtk_t *ptHandle = (CamOsTcondRtk_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
        else
            MsProduceSafeDynSem(ptHandle->pTsem, 1);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTcondLU_t *ptHandle = (CamOsTcondLU_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
        else
        {
            pthread_mutex_lock(&ptHandle->tMutex);
            pthread_cond_broadcast(&ptHandle->tCondition);
            pthread_mutex_unlock(&ptHandle->tMutex);
        }
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    struct list_head *curr, *next;
    uint32_t nWaitTaskNum = 0;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
        else
        {
            list_for_each_safe(curr, next, &ptHandle->tWqHead.task_list)
            {
                ++nWaitTaskNum;
            }

            ptHandle->nWakeUpFlag = nWaitTaskNum;
            wake_up_all(&ptHandle->tWqHead);
            ptHandle->nWakeUpFlag = 0;

            if(!list_empty(&ptHandle->tWqHead.task_list))
                CamOsPrintf("%s: [warning] wait queue isn't empty\n\r", __FUNCTION__);
        }
    }
#endif
}

void CamOsTcondWait(CamOsTcond_t *ptTcond)
{
#ifdef CAM_OS_RTK
    CamOsTcondRtk_t *ptHandle = (CamOsTcondRtk_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
        else
            MsConsumeAllDynSem(ptHandle->pTsem);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTcondLU_t *ptHandle = (CamOsTcondLU_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
        else
        {
            pthread_mutex_lock(&ptHandle->tMutex);
            pthread_cond_wait(&ptHandle->tCondition, &ptHandle->tMutex);
            pthread_mutex_unlock(&ptHandle->tMutex);
        }
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
        else
        {
            wait_event(ptHandle->tWqHead, (ptHandle->nWakeUpFlag != 0));
            if(ptHandle->nWakeUpFlag > 0)
                --ptHandle->nWakeUpFlag;
        }
    }
#endif
}

CamOsRet_e CamOsTcondTimedWait(CamOsTcond_t *ptTcond, uint32_t nMsec)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTcondRtk_t *ptHandle = (CamOsTcondRtk_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else if(CUS_OS_NO_MESSAGE == MsConsumeAllDynSemDelay(ptHandle->pTsem, RTK_MS_TO_TICK(nMsec)))
            eRet = CAM_OS_TIMEOUT;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTcondLU_t *ptHandle = (CamOsTcondLU_t *)ptTcond;
    int32_t nErr = 0;
    struct timespec tFinalTime;
    int64_t nNanoDelay = 0;

    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else
        {
            if(clock_gettime(CLOCK_MONOTONIC, &tFinalTime) == -1)
                CamOsPrintf("%s : clock_gettime fail\n\r", __FUNCTION__);

            nNanoDelay = (nMsec * 1000000LL) + tFinalTime.tv_nsec;
            tFinalTime.tv_sec += (nNanoDelay / 1000000000LL);
            tFinalTime.tv_nsec = nNanoDelay % 1000000000LL;

            pthread_mutex_lock(&ptHandle->tMutex);

            nErr = pthread_cond_timedwait(&ptHandle->tCondition, &ptHandle->tMutex,
                                          &tFinalTime);

            pthread_mutex_unlock(&ptHandle->tMutex);

            if(!nErr)
            {
                eRet = CAM_OS_OK;
            }
            else if(nErr == ETIMEDOUT)
            {
                eRet = CAM_OS_TIMEOUT;
            }
            else
            {
                CamOsPrintf("%s : pthread_cond_timedwait failed(errno=%d)\n\r", __FUNCTION__, errno);
                eRet = CAM_OS_FAIL;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    int32_t nErr = 0;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else
        {
            nErr = wait_event_timeout(ptHandle->tWqHead, (ptHandle->nWakeUpFlag != 0), msecs_to_jiffies(nMsec));
            if(!nErr)
            {
                eRet = CAM_OS_TIMEOUT;
            }
            else
            {
                if(ptHandle->nWakeUpFlag > 0)
                    --ptHandle->nWakeUpFlag;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

void CamOsTcondSignalInterruptible(CamOsTcond_t *ptTcond)
{
#ifdef CAM_OS_RTK
    CamOsTcondSignal(ptTcond);
#elif defined(CAM_OS_LINUX_USER)
    CamOsTcondSignal(ptTcond);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
        }
        else
        {
            ptHandle->nWakeUpFlag = 1;
            wake_up_interruptible(&ptHandle->tWqHead);
            ptHandle->nWakeUpFlag = 0;
        }
    }
#endif
}

void CamOsTcondSignalAllInterruptible(CamOsTcond_t *ptTcond)
{
#ifdef CAM_OS_RTK
    CamOsTcondSignalAll(ptTcond);
#elif defined(CAM_OS_LINUX_USER)
    CamOsTcondSignalAll(ptTcond);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    uint32_t nWaitTaskNum = 0;
    struct list_head *curr, *next;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
        }
        else
        {
            list_for_each_safe(curr, next, &ptHandle->tWqHead.task_list)
            {
                ++nWaitTaskNum;
            }

            ptHandle->nWakeUpFlag = nWaitTaskNum;
            wake_up_interruptible_all(&ptHandle->tWqHead);
            ptHandle->nWakeUpFlag = 0;
        }
    }
#endif
}

void CamOsTcondWaitInterruptible(CamOsTcond_t *ptTcond)
{
#ifdef CAM_OS_RTK
    CamOsTcondWait(ptTcond);
#elif defined(CAM_OS_LINUX_USER)
    CamOsTcondWait(ptTcond);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
        }
        else
        {
            if(ptHandle->nWakeUpFlag != 2)
                ptHandle->nWakeUpFlag = 0;
            wait_event_interruptible(ptHandle->tWqHead, (ptHandle->nWakeUpFlag != 0));
            if(ptHandle->nWakeUpFlag == 1)
                ptHandle->nWakeUpFlag = 0;
        }
    }
#endif
}

CamOsRet_e CamOsTcondTimedWaitInterruptible(CamOsTcond_t *ptTcond, uint32_t nMsec)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    eRet = CamOsTcondTimedWait(ptTcond, nMsec);
#elif defined(CAM_OS_LINUX_USER)
    eRet = CamOsTcondTimedWait(ptTcond, nMsec);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    int32_t nErr = 0;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else
        {
            if(ptHandle->nWakeUpFlag != 2)
                ptHandle->nWakeUpFlag = 0;
            nErr = wait_event_interruptible_timeout(ptHandle->tWqHead, (ptHandle->nWakeUpFlag != 0), msecs_to_jiffies(nMsec));
            if(!nErr)
            {
                eRet = CAM_OS_TIMEOUT;
            }
            else
            {
                if(ptHandle->nWakeUpFlag == 1)
                    ptHandle->nWakeUpFlag = 0;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsTcondWaitActive(CamOsTcond_t *ptTcond)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTcondRtk_t *ptHandle = (CamOsTcondRtk_t *)ptTcond;
    int16_t nSemCount;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else
        {
            if(CUS_OS_OK == MsGetDynSemCount(ptHandle->pTsem, &nSemCount))
            {
                if(nSemCount == 0)
                    eRet = CAM_OS_FAIL;
            }
            else
                eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    int32_t nErr = 0;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s : Condition not init\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
        else
        {
            nErr = waitqueue_active(&ptHandle->tWqHead);
            if(!nErr)
            {
                eRet = CAM_OS_FAIL;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

void* CamOsMemAlloc(uint32_t nSize)
{
#ifdef CAM_OS_RTK
    return MsAllocateMem(nSize);
#elif defined(CAM_OS_LINUX_USER)
    return malloc(nSize);
#elif defined(CAM_OS_LINUX_KERNEL)
    return vzalloc(nSize);
#endif
}

void* CamOsMemCalloc(uint32_t nNum, uint32_t nSize)
{
#ifdef CAM_OS_RTK
    return MsCallocateMem(nNum * nSize);
#elif defined(CAM_OS_LINUX_USER)
    return calloc(nNum, nSize);
#elif defined(CAM_OS_LINUX_KERNEL)
    return vzalloc(nNum * nSize);
#endif
}

void* CamOsMemRealloc(void* pPtr, uint32_t nSize)
{
#ifdef CAM_OS_RTK
    return MsMemoryReAllocate(pPtr, nSize);
#elif defined(CAM_OS_LINUX_USER)
    return realloc(pPtr, nSize);
#elif defined(CAM_OS_LINUX_KERNEL)
    void *pAddr = vzalloc(nSize);
    if(pPtr && pAddr)
    {
        memcpy(pAddr, pPtr, nSize);
        vfree(pPtr);
    }
    return pAddr;
#endif
}

void CamOsMemFlush(void* pPtr, uint32_t nSize)
{
#ifdef CAM_OS_RTK
    sys_flush_data_cache_buffer((uint32_t)pPtr, nSize);
#elif defined(CAM_OS_LINUX_USER)
    // TODO: implement cache flush in linux user space.
#elif defined(CAM_OS_LINUX_KERNEL)
    flush_icache_range((unsigned long)pPtr, nSize);
    Chip_Flush_Memory();
#endif
}

void CamOsMemRelease(void* pPtr)
{
#ifdef CAM_OS_RTK
    if(pPtr)
    {
        MsReleaseMemory(pPtr);
    }
#elif defined(CAM_OS_LINUX_USER)
    if(pPtr)
    {
        free(pPtr);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(pPtr)
    {
        vfree(pPtr);
    }
#endif
}

static int32_t _CheckDmemInfoListInited(void)
{
#ifdef CAM_OS_RTK
    MsMutexLock(&_gtMemLock);
#elif defined(CAM_OS_LINUX_USER)
    pthread_mutex_lock(&_gtMemLock);
#elif defined(CAM_OS_LINUX_KERNEL)
    mutex_lock(&_gtMemLock);
#endif
    if(!_gnDmemDbgListInited)
    {
        memset(&_gtMemList, 0, sizeof(MemoryList_t));
        CAM_OS_INIT_LIST_HEAD(&_gtMemList.tList);

        _gnDmemDbgListInited = 1;
    }
#ifdef CAM_OS_RTK
    MsMutexUnlock(&_gtMemLock);
#elif defined(CAM_OS_LINUX_USER)
    pthread_mutex_unlock(&_gtMemLock);
#elif defined(CAM_OS_LINUX_KERNEL)
    mutex_unlock(&_gtMemLock);
#endif

    return 0;
}

CamOsRet_e CamOsDirectMemAlloc(const char* szName,
                               uint32_t nSize,
                               void** ppVirtPtr,
                               void** ppPhysPtr,
                               void** ppMiuPtr)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    uint8_t nAllocSucc = TRUE;
    void *pNonCachePtr = NULL;

    pNonCachePtr = MsAllocateNonCacheMemExt(nSize, 12);
    nAllocSucc &= MsIsHeapMemory(pNonCachePtr);

    if((uint32_t)pNonCachePtr & ((1 << 6) - 1))
    {
        nAllocSucc &= FALSE;
        MsReleaseMemory(pNonCachePtr);
    }

    if(nAllocSucc == TRUE)
    {
        ASSIGN_POINTER_VALUE(ppVirtPtr, pNonCachePtr);
        ASSIGN_POINTER_VALUE(ppPhysPtr, MsVA2PA(pNonCachePtr));
        ASSIGN_POINTER_VALUE(ppMiuPtr, (void *)HalUtilPHY2MIUAddr((uint32_t)*ppPhysPtr));
    }
    else
    {
        ASSIGN_POINTER_VALUE(ppVirtPtr, NULL);
        ASSIGN_POINTER_VALUE(ppPhysPtr, NULL);
        ASSIGN_POINTER_VALUE(ppMiuPtr, NULL);
        eRet = CAM_OS_FAIL;
    }

    /*CamOsPrintf("%s    0x%08X  0x%08X  0x%08X\r\n",
            __FUNCTION__,
            (uint32_t)*ppVirtPtr,
            (uint32_t)*ppPhysPtr,
            (uint32_t)*ppMiuPtr);*/

    _CheckDmemInfoListInited();

    MsMutexLock(&_gtMemLock);
    MemoryList_t* ptNewEntry = (MemoryList_t*) MsAllocateMem(sizeof(MemoryList_t));
    ptNewEntry->pPtr = pNonCachePtr;
    ptNewEntry->pMemifoPtr = NULL;
    ptNewEntry->szName = (char *)MsAllocateMem(strlen(szName) + 1);
    if(ptNewEntry->szName)
        strncpy(ptNewEntry->szName, szName, strlen(szName));
    ptNewEntry->nSize = nSize;
    CAM_OS_LIST_ADD_TAIL(&(ptNewEntry->tList), &_gtMemList.tList);
    MsMutexUnlock(&_gtMemLock);
#elif defined(CAM_OS_LINUX_USER)
    int32_t nMsysFd = -1;
    int32_t nMemFd = -1;
    MSYS_DMEM_INFO * ptMsysMem = NULL;
    unsigned char* pMmapPtr = NULL;
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;

    do
    {
        //Check request name to avoid allocate same dmem address.
        _CheckDmemInfoListInited();

        pthread_mutex_lock(&_gtMemLock);
        CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
        {
            ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

            if(ptTmp->pPtr && ptTmp->szName && 0 == strcmp(szName, ptTmp->szName))
            {
                fprintf(stderr, "%s request same dmem name: %s\n", __FUNCTION__, szName);
                eRet = CAM_OS_PARAM_ERR;
            }
        }
        pthread_mutex_unlock(&_gtMemLock);
        if(eRet == CAM_OS_PARAM_ERR)
        {
            ASSIGN_POINTER_VALUE(ppVirtPtr, NULL);
            ASSIGN_POINTER_VALUE(ppPhysPtr, NULL);
            ASSIGN_POINTER_VALUE(ppMiuPtr, NULL);
            break;
        }

        if(0 > (nMsysFd = open("/dev/msys", O_RDWR | O_SYNC)))
        {
            fprintf(stderr, "%s open /dev/msys failed!!\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        if(0 > (nMemFd = open("/dev/mem", O_RDWR | O_SYNC)))
        {
            fprintf(stderr, "%s open /dev/mem failed!!\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        ptMsysMem = (MSYS_DMEM_INFO *) malloc(sizeof(MSYS_DMEM_INFO));
        MSYS_ADDR_TRANSLATION_INFO tAddrInfo;
        FILL_VERCHK_TYPE(tAddrInfo, tAddrInfo.VerChk_Version, tAddrInfo.VerChk_Size,
                         IOCTL_MSYS_VERSION);
        FILL_VERCHK_TYPE(*ptMsysMem, ptMsysMem->VerChk_Version,
                         ptMsysMem->VerChk_Size, IOCTL_MSYS_VERSION);

        ptMsysMem->length = nSize;
        snprintf(ptMsysMem->name, sizeof(ptMsysMem->name), "%s", szName);

        if(ioctl(nMsysFd, IOCTL_MSYS_REQUEST_DMEM, ptMsysMem))
        {
            ptMsysMem->length = 0;
            fprintf(stderr, "%s [%s][%d]Request Direct Memory Failed!!\n", __FUNCTION__, szName, (uint32_t)nSize);
            free(ptMsysMem);
            eRet = CAM_OS_FAIL;
            break;
        }

        if(ptMsysMem->length < nSize)
        {
            ioctl(nMsysFd, IOCTL_MSYS_RELEASE_DMEM, ptMsysMem);
            fprintf(stderr, "%s [%s]Request Direct Memory Failed!! because dmem size <%d>smaller than <%d>\n",
                    __FUNCTION__, szName, ptMsysMem->length, (uint32_t)nSize);
            free(ptMsysMem);
            eRet = CAM_OS_FAIL;
            break;
        }

        tAddrInfo.addr = ptMsysMem->phys;
        ASSIGN_POINTER_VALUE(ppPhysPtr, (void *)(uintptr_t)ptMsysMem->phys);
        if(ioctl(nMsysFd, IOCTL_MSYS_PHYS_TO_MIU, &tAddrInfo))
        {
            ioctl(nMsysFd, IOCTL_MSYS_RELEASE_DMEM, ptMsysMem);
            fprintf(stderr, "%s [%s][%d]IOCTL_MSYS_PHYS_TO_MIU Failed!!\n", __FUNCTION__, szName, (uint32_t)nSize);
            free(ptMsysMem);
            eRet = CAM_OS_FAIL;
            break;
        }
        ASSIGN_POINTER_VALUE(ppMiuPtr, (void *)(uintptr_t)tAddrInfo.addr);
        pMmapPtr = mmap(0, ptMsysMem->length, PROT_READ | PROT_WRITE, MAP_SHARED,
                        nMemFd, ptMsysMem->phys);
        if(pMmapPtr == (void *) - 1)
        {
            ioctl(nMsysFd, IOCTL_MSYS_RELEASE_DMEM, ptMsysMem);
            fprintf(stderr, "%s failed!! physAddr<0x%x> size<0x%x> errno<%d, %s> \r\n",
                    __FUNCTION__,
                    (uint32_t)ptMsysMem->phys,
                    (uint32_t)ptMsysMem->length, errno, strerror(errno));
            free(ptMsysMem);
            eRet = CAM_OS_FAIL;
            break;
        }
        ASSIGN_POINTER_VALUE(ppVirtPtr, pMmapPtr);

        fprintf(stderr, "%s <%s> physAddr<0x%x> size<%d>\r\n",
                __FUNCTION__,
                szName, (uint32_t)ptMsysMem->phys,
                (uint32_t)ptMsysMem->length);

        pthread_mutex_lock(&_gtMemLock);
        MemoryList_t* ptNewEntry = (MemoryList_t*) malloc(sizeof(MemoryList_t));
        ptNewEntry->pPtr = pMmapPtr;
        ptNewEntry->pMemifoPtr = (void *) ptMsysMem;
        ptNewEntry->szName = strdup(szName);
        ptNewEntry->nSize = ptMsysMem->length;
        CAM_OS_LIST_ADD_TAIL(&(ptNewEntry->tList), &_gtMemList.tList);
        pthread_mutex_unlock(&_gtMemLock);
    }
    while(0);

    if(nMsysFd >= 0)
    {
        close(nMsysFd);
    }
    if(nMemFd >= 0)
    {
        close(nMemFd);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    MSYS_DMEM_INFO *ptDmem = NULL;
    MemoryList_t* ptNewEntry = NULL;
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;

    ASSIGN_POINTER_VALUE(ppVirtPtr, NULL);
    ASSIGN_POINTER_VALUE(ppPhysPtr, NULL);
    ASSIGN_POINTER_VALUE(ppMiuPtr, NULL);

    do
    {
        //Check request name to avoid allocate same dmem address.
        _CheckDmemInfoListInited();

        mutex_lock(&_gtMemLock);
        CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
        {
            ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

            if(ptTmp->pPtr && ptTmp->szName && 0 == strcmp(szName, ptTmp->szName))
            {
                printk(KERN_WARNING "%s request same dmem name: %s\n", __FUNCTION__, szName);
                eRet = CAM_OS_PARAM_ERR;
            }
        }
        mutex_unlock(&_gtMemLock);
        if(eRet == CAM_OS_PARAM_ERR)
        {
            ASSIGN_POINTER_VALUE(ppVirtPtr, NULL);
            ASSIGN_POINTER_VALUE(ppPhysPtr, NULL);
            ASSIGN_POINTER_VALUE(ppMiuPtr, NULL);
            break;
        }

        if(0 == (ptDmem = (MSYS_DMEM_INFO *)kzalloc(sizeof(MSYS_DMEM_INFO), GFP_KERNEL)))
        {
            printk(KERN_WARNING "%s kzalloc MSYS_DMEM_INFO fail\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        snprintf(ptDmem->name, 15, szName);
        ptDmem->length = nSize;

        if(0 != msys_request_dmem(ptDmem))
        {
            printk(KERN_WARNING "%s msys_request_dmem fail\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        ASSIGN_POINTER_VALUE(ppVirtPtr, (void *)(uintptr_t)ptDmem->kvirt);
        ASSIGN_POINTER_VALUE(ppMiuPtr, (void *)(uintptr_t)Chip_Phys_to_MIU(ptDmem->phys));
        ASSIGN_POINTER_VALUE(ppPhysPtr, (void *)(uintptr_t)ptDmem->phys);

        printk(KERN_INFO "%s <%s> physAddr<0x%08X> size<%d>  \r\n",
               __FUNCTION__,
               szName,
               (uint32_t)ptDmem->phys,
               (uint32_t)ptDmem->length);

        mutex_lock(&_gtMemLock);
        if(0 == (ptNewEntry = (MemoryList_t*) kzalloc(sizeof(MemoryList_t), GFP_KERNEL)))
        {
            printk(KERN_WARNING "%s kzalloc MemoryList_t fail\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }
        ptNewEntry->pPtr = (void *)(uintptr_t)ptDmem->kvirt;
        ptNewEntry->pMemifoPtr = (void *) ptDmem;
        if(0 == (ptNewEntry->szName = (char *)kzalloc(strlen(szName) + 1, GFP_KERNEL)))
        {
            printk(KERN_WARNING "%s kzalloc MemoryList_t szName fail\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }
        strncpy(ptNewEntry->szName, szName, strlen(szName));
        ptNewEntry->nSize = ptDmem->length;
        CAM_OS_LIST_ADD_TAIL(&(ptNewEntry->tList), &_gtMemList.tList);
        mutex_unlock(&_gtMemLock);
    }
    while(0);

    if(eRet == CAM_OS_FAIL)
    {
        if(ptDmem)
        {
            if(ptDmem->phys)
            {
                msys_release_dmem(ptDmem);
            }
            kfree(ptDmem);
        }

        if(ptNewEntry)
        {
            if(ptNewEntry->szName)
            {
                kfree(ptNewEntry->szName);
            }
            kfree(ptNewEntry);
        }
    }
#endif
    return eRet;
}

CamOsRet_e CamOsDirectMemRelease(void* pVirtPtr, uint32_t nSize)
{
    CamOsRet_e eRet = CAM_OS_OK;
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;
#ifdef CAM_OS_RTK
    if(pVirtPtr)
    {
        MsReleaseMemory(pVirtPtr);
        //CamOsPrintf("%s do release\n\r", __FUNCTION__);

        _CheckDmemInfoListInited();

        MsMutexLock(&_gtMemLock);
        CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
        {
            ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

            if(ptTmp->pPtr == pVirtPtr)
            {
                if(ptTmp->szName)
                    MsReleaseMemory(ptTmp->szName);
                CAM_OS_LIST_DEL(ptPos);
                MsReleaseMemory(ptTmp);
            }
        }
        MsMutexUnlock(&_gtMemLock);
    }
#elif defined(CAM_OS_LINUX_USER)
    int32_t nMsysfd = -1;
    int32_t nErr = 0;
    MSYS_DMEM_INFO *pMsysMem = NULL;

    if(pVirtPtr)
    {
        do
        {
            if(0 > (nMsysfd = open("/dev/msys", O_RDWR | O_SYNC)))
            {
                fprintf(stderr, "%s open /dev/msys failed!!\n", __FUNCTION__);
                eRet = CAM_OS_FAIL;
                break;
            }

            _CheckDmemInfoListInited();

            pthread_mutex_lock(&_gtMemLock);
            CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
            {
                ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

                if(ptTmp->pPtr == pVirtPtr)
                {
                    pMsysMem = (MSYS_DMEM_INFO *) ptTmp->pMemifoPtr;
                    break;
                }
            }
            pthread_mutex_unlock(&_gtMemLock);
            if(pMsysMem == NULL)
            {
                fprintf(stderr, "%s find Msys_DMEM_Info node failed!! <0x%08X>  \r\n", __FUNCTION__, (uint32_t)pVirtPtr);
                eRet = CAM_OS_FAIL;
                break;
            }

            if(nSize != pMsysMem->length)
            {
                nErr = munmap((void *)pVirtPtr, pMsysMem->length);
                if(0 != nErr)
                {
                    fprintf(stderr, "%s munmap failed!! <0x%08X> size<%d> err<%d> errno<%d, %s> \r\n",
                            __FUNCTION__, (uint32_t)pVirtPtr, (uint32_t)pMsysMem->length, nErr, errno, strerror(errno));
                }
            }
            else
            {
                nErr = munmap((void *)pVirtPtr, nSize);
                if(0 != nErr)
                {
                    fprintf(stderr, "%s munmap failed!! <0x%08X> size<%d> err<%d> errno<%d, %s> \r\n",
                            __FUNCTION__, (uint32_t)pVirtPtr, (uint32_t)nSize, nErr, errno, strerror(errno));
                }
            }

            if(ioctl(nMsysfd, IOCTL_MSYS_RELEASE_DMEM, pMsysMem))
            {
                fprintf(stderr, "%s : IOCTL_MSYS_RELEASE_DMEM error physAddr<0x%x>\n", __FUNCTION__, (uint32_t)pMsysMem->phys);
                eRet = CAM_OS_FAIL;
                break;
            }
            if(pMsysMem)
            {
                free(pMsysMem);
                pMsysMem = NULL;
            }
            pthread_mutex_lock(&_gtMemLock);
            CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
            {
                ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

                if(ptTmp->pPtr == pVirtPtr)
                {
                    if(ptTmp->szName)
                        free(ptTmp->szName);
                    CAM_OS_LIST_DEL(ptPos);
                    free(ptTmp);
                }
            }
            pthread_mutex_unlock(&_gtMemLock);
        }
        while(0);

        if(nMsysfd >= 0)
        {
            close(nMsysfd);
        }
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    MSYS_DMEM_INFO *tpDmem = NULL;

    if(pVirtPtr)
    {
        do
        {
            _CheckDmemInfoListInited();

            mutex_lock(&_gtMemLock);
            CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
            {
                ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

                //printk("search tmp->ptr: %08X  %s\n", (uint32_t)ptTmp->pPtr, ptTmp->szName);

                if(ptTmp->pPtr == pVirtPtr)
                {
                    tpDmem = ptTmp->pMemifoPtr;
                    //printk("search(2) pdmem->name: %s\n", tpDmem->name);
                    break;
                }
            }
            mutex_unlock(&_gtMemLock);
            if(tpDmem == NULL)
            {
                printk(KERN_WARNING "%s find Msys_DMEM_Info node failed!! <0x%08X>  \r\n", __FUNCTION__, (uint32_t)pVirtPtr);
                eRet = CAM_OS_FAIL;
                break;
            }

            msys_release_dmem(tpDmem);

            if(tpDmem)
            {
                kfree(tpDmem);
                tpDmem = NULL;
            }
            mutex_lock(&_gtMemLock);
            CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
            {
                ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

                if(ptTmp->pPtr == pVirtPtr)
                {
                    if(ptTmp->szName)
                        kfree(ptTmp->szName);
                    CAM_OS_LIST_DEL(ptPos);
                    kfree(ptTmp);
                }
            }
            mutex_unlock(&_gtMemLock);
        }
        while(0);
    }
#endif
    return eRet;
}

CamOsRet_e CamOsDirectMemFlush(void* pVirtPtr)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;

    if(pVirtPtr)
    {
        eRet = CAM_OS_FAIL;

        _CheckDmemInfoListInited();

        MsMutexLock(&_gtMemLock);
        CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
        {
            ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

            if(ptTmp->pPtr == pVirtPtr)
            {
                eRet = CAM_OS_OK;
                sys_flush_data_cache_buffer((uint32_t)MsVA2PA(ptTmp->pPtr), ptTmp->nSize);
            }
        }
        MsMutexUnlock(&_gtMemLock);
    }
#elif defined(CAM_OS_LINUX_USER)
    int32_t nMsysFd = -1;

    do
    {
        if(0 > (nMsysFd = open("/dev/msys", O_RDWR | O_SYNC)))
        {
            fprintf(stderr, "%s open /dev/msys failed!!\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        if(ioctl(nMsysFd, IOCTL_MSYS_FLUSH_MEMORY, 1))
        {
            fprintf(stderr, "%s IOCTL_MSYS_FLUSH_MEMORY Failed!!\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }
    }
    while(0);

    if(nMsysFd >= 0)
    {
        close(nMsysFd);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    flush_cache_all();
    Chip_Flush_Memory();
#endif
    return eRet;
}

CamOsRet_e CamOsDirectMemStat(void)
{
    CamOsRet_e eRet = CAM_OS_OK;
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;
#ifdef CAM_OS_RTK
    _CheckDmemInfoListInited();

    MsMutexLock(&_gtMemLock);
    CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
    {
        ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

        if(ptTmp->pPtr)
        {
            CamOsPrintf("%s memory allocated %p %s\n", __FUNCTION__, ptTmp->pPtr, ptTmp->szName);
        }
    }
    MsMutexUnlock(&_gtMemLock);
#elif defined(CAM_OS_LINUX_USER)
    _CheckDmemInfoListInited();

    pthread_mutex_lock(&_gtMemLock);
    CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
    {
        ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

        if(ptTmp->pPtr)
        {
            fprintf(stderr, "%s memory allocated 0x%08X %s\n", __FUNCTION__, (uint32_t)ptTmp->pPtr, ptTmp->szName);
        }
    }
    pthread_mutex_unlock(&_gtMemLock);
#elif defined(CAM_OS_LINUX_KERNEL)
    _CheckDmemInfoListInited();

    mutex_lock(&_gtMemLock);
    CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
    {
        ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

        if(ptTmp->pPtr)
        {
            printk(KERN_WARNING "%s memory allocated 0x%08X %s\n", __FUNCTION__, (uint32_t)ptTmp->pPtr, ptTmp->szName);
        }
    }
    mutex_unlock(&_gtMemLock);
#endif
    return eRet;
}

void* CamOsDirectMemPhysToMiu(void* pPtr)
{
#ifdef CAM_OS_RTK
    return (void *)HalUtilPHY2MIUAddr((uint32_t)pPtr);
#elif defined(CAM_OS_LINUX_USER)
    int32_t nMsysFd = -1;
    void *nMiuAddr = 0;
    MSYS_ADDR_TRANSLATION_INFO tAddrInfo;

    do
    {
        if(0 > (nMsysFd = open("/dev/msys", O_RDWR | O_SYNC)))
        {
            fprintf(stderr, "%s open /dev/msys failed!!\n", __FUNCTION__);
            break;
        }

        FILL_VERCHK_TYPE(tAddrInfo, tAddrInfo.VerChk_Version, tAddrInfo.VerChk_Size,
                         IOCTL_MSYS_VERSION);

        tAddrInfo.addr = (uintptr_t)pPtr;
        if(ioctl(nMsysFd, IOCTL_MSYS_PHYS_TO_MIU, &tAddrInfo))
        {
            fprintf(stderr, "%s IOCTL_MSYS_PHYS_TO_MIU Failed!!\n", __FUNCTION__);
            break;
        }
        nMiuAddr = (void *)(uintptr_t)tAddrInfo.addr;
    }
    while(0);

    if(nMsysFd >= 0)
    {
        close(nMsysFd);
    }

    return nMiuAddr;
#elif defined(CAM_OS_LINUX_KERNEL)
    if((uint32_t)pPtr & 0xF0000000)
    {
        pPtr = (void *)(uintptr_t)Chip_Phys_to_MIU((uintptr_t)pPtr);
    }
    else
    {
        CamOsPrintf("CamOs WARNING: PhysToMiu input a MIU addr 0x%08X\n", (uint32_t)pPtr);
    }

    return pPtr;
#endif
}

void* CamOsDirectMemMiuToPhys(void* pPtr)
{
#ifdef CAM_OS_RTK
    return (void *)HalUtilMIU2PHYAddr((uint32_t)pPtr);
#elif defined(CAM_OS_LINUX_USER)
    int32_t nMsysFd = -1;
    void *nPhysAddr = 0;
    MSYS_ADDR_TRANSLATION_INFO tAddrInfo;

    do
    {
        if(0 > (nMsysFd = open("/dev/msys", O_RDWR | O_SYNC)))
        {
            fprintf(stderr, "%s open /dev/msys failed!!\n", __FUNCTION__);
            break;
        }

        FILL_VERCHK_TYPE(tAddrInfo, tAddrInfo.VerChk_Version, tAddrInfo.VerChk_Size,
                         IOCTL_MSYS_VERSION);

        tAddrInfo.addr = (uintptr_t)pPtr;
        if(ioctl(nMsysFd, IOCTL_MSYS_MIU_TO_PHYS, &tAddrInfo))
        {
            fprintf(stderr, "%s IOCTL_MSYS_MIU_TO_PHYS Failed!!\n", __FUNCTION__);
            break;
        }
        nPhysAddr = (void *)(uintptr_t)tAddrInfo.addr;
    }
    while(0);

    if(nMsysFd >= 0)
    {
        close(nMsysFd);
    }

    return nPhysAddr;
#elif defined(CAM_OS_LINUX_KERNEL)
    if((uint32_t)pPtr & 0xF0000000)
    {
        CamOsPrintf("CamOs WARNING: MiuToPhys input a PHYS addr 0x%08X\n", (uint32_t)pPtr);
    }
    else
    {
        pPtr = (void *)(uintptr_t)Chip_MIU_to_Phys((uintptr_t)pPtr);
    }

    return pPtr;
#endif
}

void* CamOsDirectMemPhysToVirt(void* pPtr)
{
#ifdef CAM_OS_RTK
    return MsPA2VA(pPtr);
#elif defined(CAM_OS_LINUX_USER)
    // TODO: implement PhysToVirt in linux user space.
    return NULL;
#elif defined(CAM_OS_LINUX_KERNEL)
#if 0
    return (void *)phys_to_virt((unsigned long)pPtr);
#else
    MSYS_DMEM_INFO dmem;
    uint64_t kptr = 0;
    if(0 == msys_find_dmem_by_phys((uint64_t)(uint32_t)pPtr, &dmem))
    {
        kptr = dmem.kvirt;
        kptr += ((uint64_t)(uint32_t)pPtr - dmem.phys);
    }
    else
    {
        CamOsPrintf("CamOs WARNING: PhysToVirt not found 0x%08X in msys\n", (uint32_t)pPtr);
    }

    return (void*)(uint32_t)kptr;
#endif
#endif
}

void* CamOsDirectMemVirtToPhys(void* pPtr)
{
#ifdef CAM_OS_RTK
    return MsVA2PA(pPtr);
#elif defined(CAM_OS_LINUX_USER)
    // TODO: implement VirtToPhys in linux user space.
    return NULL;
#elif defined(CAM_OS_LINUX_KERNEL)
    return (void *)virt_to_phys(pPtr);
#endif
}

void* CamOsPhyMemMap(void* pPhyPtr, uint32_t nSize, bool bNonCache)
{
#ifdef CAM_OS_RTK
    return MsPA2VA(pPhyPtr);
#elif defined(CAM_OS_LINUX_USER)
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
    return NULL;
#elif defined(CAM_OS_LINUX_KERNEL)
    unsigned long long nCpuBusAddr;
    void *pVirtPtr = NULL;
    int nRet, i, j, k;
    struct sg_table *pSgTable;
    struct scatterlist *pScatterList;
    int nPageCount = 0;
    struct page **ppPages;
    pgprot_t tPgProt;

    if(!nSize)
        return NULL;

    if(bNonCache)
        tPgProt = pgprot_writecombine(PAGE_KERNEL);
    else
        tPgProt = PAGE_KERNEL;

    pSgTable = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
    if(!pSgTable)
    {
        CamOsPrintf("%s: kmalloc fail\r\n", __FUNCTION__);
        return NULL;
    }

    nRet = sg_alloc_table(pSgTable, 1, GFP_KERNEL);

    if(unlikely(nRet))
    {
        CamOsPrintf("%s: sg_alloc_table fail\r\n", __FUNCTION__);
        kfree(pSgTable);
        return NULL;
    }

    if((uint32_t)pPhyPtr & 0xF0000000)
    {
        nCpuBusAddr = (uintptr_t)Chip_Phys_to_MIU((uintptr_t)pPhyPtr);
    }
    else
    {
        CamOsPrintf("CamOs WARNING: PhysToMiu input a MIU addr 0x%08X\n", (uint32_t)pPhyPtr);
    }

    sg_set_page(pSgTable->sgl, pfn_to_page(__phys_to_pfn(nCpuBusAddr)), PAGE_ALIGN(nSize), 0);

    for_each_sg(pSgTable->sgl, pScatterList, pSgTable->nents, i)
    {
        nPageCount += pScatterList->length / PAGE_SIZE;
    }

    ppPages = vmalloc(sizeof(struct page*)*nPageCount);

    if(ppPages == NULL)
    {
        CamOsPrintf("%s: vmalloc fail\r\n", __FUNCTION__);
        sg_free_table(pSgTable);
        kfree(pSgTable);
        return NULL;
    }

    for_each_sg(pSgTable->sgl, pScatterList, pSgTable->nents, k)
    {
        nPageCount = PAGE_ALIGN(pScatterList->length) / PAGE_SIZE;
        for(j = 0; j < nPageCount; j++)
            ppPages[i + j] = sg_page(pScatterList) + j;
        i += nPageCount;
    }
    pVirtPtr = vmap(ppPages, i, VM_MAP, tPgProt);

    vfree(ppPages);
    sg_free_table(pSgTable);
    kfree(pSgTable);

    return pVirtPtr;
#endif
}

void CamOsPhyMemUnMap(void* pVirtPtr)
{
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
#elif defined(CAM_OS_LINUX_KERNEL)
    vunmap(pVirtPtr);
#endif
}

CamOsRet_e CamOsMemCacheCreate(CamOsMemCache_t *ptMemCache, char *szName, uint32_t nSize, bool bHwCacheAlign)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsMemCacheRtk_t *ptHandle = (CamOsMemCacheRtk_t *)ptMemCache;

    if(ptHandle)
    {
        if(RTK_OK == MsFindBestPool(nSize, &ptHandle->nPoolID))
        {
            ptHandle->nObjSize = nSize;
        }
        else
        {
            CamOsPrintf("%s: Can't get satisfactory memory pool\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#elif defined(CAM_OS_LINUX_USER)
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct kmem_cache *ptKmemCache;
    CamOsMemCacheLK_t *ptHandle = (CamOsMemCacheLK_t *)ptMemCache;

    if(ptHandle)
    {
        ptKmemCache = kmem_cache_create(szName, nSize, bHwCacheAlign ? SLAB_HWCACHE_ALIGN : 0, 0, NULL);

        if(ptKmemCache)
        {
            ptHandle->ptKmemCache = ptKmemCache;
        }
        else
        {
            ptHandle->ptKmemCache = NULL;
            eRet = CAM_OS_FAIL;
        }
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#endif
    return eRet;
}

void CamOsMemCacheDestroy(CamOsMemCache_t *ptMemCache)
{
#ifdef CAM_OS_RTK
    CamOsMemCacheRtk_t *ptHandle = (CamOsMemCacheRtk_t *)ptMemCache;

    if(ptHandle)
        memset(ptHandle, 0, sizeof(CamOsMemCacheRtk_t));
#elif defined(CAM_OS_LINUX_USER)
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMemCacheLK_t *ptHandle = (CamOsMemCacheLK_t *)ptMemCache;

    if(ptHandle && ptHandle->ptKmemCache)
    {
        kmem_cache_destroy(ptHandle->ptKmemCache);
        ptHandle->ptKmemCache = NULL;
    }
#endif
}

void *CamOsMemCacheAlloc(CamOsMemCache_t *ptMemCache)
{
#ifdef CAM_OS_RTK
    CamOsMemCacheRtk_t *ptHandle = (CamOsMemCacheRtk_t *)ptMemCache;

    if(ptHandle)
        return MsGetPoolIdMemory(ptHandle->nPoolID);
    else
        return NULL;
#elif defined(CAM_OS_LINUX_USER)
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
    return NULL;
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMemCacheLK_t *ptHandle = (CamOsMemCacheLK_t *)ptMemCache;

    if(ptHandle && ptHandle->ptKmemCache)
        return kmem_cache_alloc(ptHandle->ptKmemCache, GFP_KERNEL);
    else
        return NULL;
#endif
}

void CamOsMemCacheFree(CamOsMemCache_t *ptMemCache, void *pObjPtr)
{
#ifdef CAM_OS_RTK
    MsReleaseMemory(pObjPtr);
#elif defined(CAM_OS_LINUX_USER)
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMemCacheLK_t *ptHandle = (CamOsMemCacheLK_t *)ptMemCache;

    if(ptHandle && ptHandle->ptKmemCache)
        kmem_cache_free(ptHandle->ptKmemCache, pObjPtr);
#endif
}

CamOsRet_e CamOsPropertySet(const char *szKey, const char *szValue)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)
    void *pLibHandle = NULL;
    int32_t(*dlsym_property_set)(const char *szKey, const char *szValue) = NULL;

    do
    {
        pLibHandle = dlopen("libat.so", RTLD_NOW);
        if(NULL == pLibHandle)
        {
            fprintf(stderr, "%s : load libat.so error \n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        dlsym_property_set = dlsym(pLibHandle, "property_set");
        if(NULL == dlsym_property_set)
        {
            fprintf(stderr, "%s : dlsym property_set failed, %s\n", __FUNCTION__, dlerror());
            eRet = CAM_OS_FAIL;
            break;
        }

        dlsym_property_set(szKey, szValue);
    }
    while(0);

    if(pLibHandle)
    {
        dlclose(pLibHandle);
    }
#elif defined(CAM_OS_LINUX_KERNEL)

#endif
    return eRet;
}

CamOsRet_e CamOsPropertyGet(const char *szKey, char *szValue, const char *szDefaultValue)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)
    void *pLibHandle = NULL;
    int32_t(*dlsym_property_get)(const char *szKey, char *szValue, const char *szDefaultValue) = NULL;

    do
    {
        pLibHandle = dlopen("libat.so", RTLD_NOW);
        if(NULL == pLibHandle)
        {
            fprintf(stderr, "%s : load libat.so error \n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }
        dlsym_property_get = dlsym(pLibHandle, "property_get");
        if(NULL == dlsym_property_get)
        {
            fprintf(stderr, "%s : dlsym property_get failed, %s\n", __FUNCTION__, dlerror());
            eRet = CAM_OS_FAIL;
            break;
        }

        dlsym_property_get(szKey, szValue, szDefaultValue);
    }
    while(0);

    if(pLibHandle)
    {
        dlclose(pLibHandle);
    }
#elif defined(CAM_OS_LINUX_KERNEL)

#endif
    return eRet;
}

uint64_t CamOsMathDivU64(uint64_t nDividend, uint64_t nDivisor, uint64_t *pRemainder)
{
#ifdef CAM_OS_RTK
    return div64_u64_rem(nDividend, nDivisor, pRemainder);
#elif defined(CAM_OS_LINUX_USER)
    *pRemainder = nDividend % nDivisor;
    return nDividend / nDivisor;
#elif defined(CAM_OS_LINUX_KERNEL)
    return div64_u64_rem(nDividend, nDivisor, pRemainder);
#endif
}

int64_t CamOsMathDivS64(int64_t nDividend, int64_t nDivisor, int64_t *pRemainder)
{
#ifdef CAM_OS_RTK
    int64_t nQuotient = div64_s64(nDividend, nDivisor);
    *pRemainder = nDividend - nDivisor * nQuotient;
    return nQuotient;
#elif defined(CAM_OS_LINUX_USER)
    *pRemainder = nDividend % nDivisor;
    return nDividend / nDivisor;
#elif defined(CAM_OS_LINUX_KERNEL)
    int64_t nQuotient = div64_s64(nDividend, nDivisor);
    *pRemainder = nDividend - nDivisor * nQuotient;
    return nQuotient;
#endif
}

uint32_t CamOsCopyFromUpperLayer(void *pTo, const void *pFrom, uint32_t nLen)
{
#ifdef CAM_OS_RTK
    memcpy(pTo, pFrom, nLen);
    return 0;
#elif defined(CAM_OS_LINUX_USER)
    memcpy(pTo, pFrom, nLen);
    return 0;
#elif defined(CAM_OS_LINUX_KERNEL)
    return copy_from_user(pTo, pFrom, nLen);
#endif
}

uint32_t CamOsCopyToUpperLayer(void *pTo, const void * pFrom, uint32_t nLen)
{
#ifdef CAM_OS_RTK
    memcpy(pTo, pFrom, nLen);
    return 0;
#elif defined(CAM_OS_LINUX_USER)
    memcpy(pTo, pFrom, nLen);
    return 0;
#elif defined(CAM_OS_LINUX_KERNEL)
    return copy_to_user(pTo, pFrom, nLen);
#endif
}

#ifdef CAM_OS_RTK
void _CamOsTimerCallback(MsTimerId_e eTimerID, uint32_t nHandleAddr)
{
    CamOsTimerRtk_t *ptHandle = (CamOsTimerRtk_t *)nHandleAddr;

    CamOsPrintf("%s: eTimerID = 0x%x\r\n", __FUNCTION__, eTimerID);
    if(ptHandle)
    {
        ptHandle->pfnCallback((uint32_t)ptHandle->pDataPtr);
    }
}
#endif

CamOsRet_e CamOsTimerInit(CamOsTimer_t *ptTimer)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTimerRtk_t *ptHandle = (CamOsTimerRtk_t *)ptTimer;
    if(ptHandle)
    {
        if(ptHandle->eTimerID != INIT_MAGIC_NUM && MsIsTimerActive(ptHandle->eTimerID))
        {
            MsStopTimer(ptHandle->eTimerID);
        }

        ptHandle->eTimerID = INIT_MAGIC_NUM;
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#elif defined(CAM_OS_LINUX_USER)
    // TODO: implement timer in linux user space.
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTimerLK_t *ptHandle = (CamOsTimerLK_t *)ptTimer;
    if(ptHandle)
    {
        init_timer(&ptHandle->tTimerID);
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#endif
    return eRet;
}

uint32_t CamOsTimerDelete(CamOsTimer_t *ptTimer)
{
#ifdef CAM_OS_RTK
    CamOsTimerRtk_t *ptHandle = (CamOsTimerRtk_t *)ptTimer;
    if(ptHandle)
    {
        return MsStopTimer(ptHandle->eTimerID) ? 1 : 0;
    }
#elif defined(CAM_OS_LINUX_USER)
    // TODO: implement timer in linux user space.
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
    return 0;
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTimerLK_t *ptHandle = (CamOsTimerLK_t *)ptTimer;
    if(ptHandle)
    {
        return del_timer(&ptHandle->tTimerID);
    }
#endif
    return 0;
}

CamOsRet_e CamOsTimerAdd(CamOsTimer_t *ptTimer, uint32_t nMsec, void *pDataPtr,
                         void (*pfnFunc)(unsigned long nDataAddr))
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTimerRtk_t *ptHandle = (CamOsTimerRtk_t *)ptTimer;
    if(ptHandle)
    {
        if(ptHandle->eTimerID != INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s: Please init timer first!\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }

        ptHandle->pfnCallback = pfnFunc;
        ptHandle->pDataPtr = pDataPtr;

        if(CUS_OS_OK != MsStartCbTimerMs(&ptHandle->eTimerID, _CamOsTimerCallback, (uint32_t)pDataPtr, nMsec, 0))
        {
            CamOsPrintf("%s: MsStartCbTimerMs fail!\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#elif defined(CAM_OS_LINUX_USER)
    // TODO: implement timer in linux user space.
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTimerLK_t *ptHandle = (CamOsTimerLK_t *)ptTimer;
    if(ptHandle)
    {
        ptHandle->tTimerID.expires = jiffies + msecs_to_jiffies(nMsec);
        ptHandle->tTimerID.function = pfnFunc;
        ptHandle->tTimerID.data = (unsigned long)pDataPtr;
        add_timer(&ptHandle->tTimerID);
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#endif
    return eRet;
}

CamOsRet_e CamOsTimerModify(CamOsTimer_t *ptTimer, uint32_t nMsec)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTimerRtk_t *ptHandle = (CamOsTimerRtk_t *)ptTimer;
    if(ptHandle)
    {
        if(ptHandle->eTimerID == INIT_MAGIC_NUM)
        {
            CamOsPrintf("%s: Please add timer first!\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }

        if(MsIsTimerActive(ptHandle->eTimerID))
        {
            MsStopTimer(ptHandle->eTimerID);
            if(CUS_OS_OK != MsStartCbTimerMs(&ptHandle->eTimerID, _CamOsTimerCallback, (uint32_t)ptHandle, nMsec, 0))
            {
                CamOsPrintf("%s: MsStartCbTimerMs fail!\n", __FUNCTION__);
                eRet = CAM_OS_FAIL;
            }
        }
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#elif defined(CAM_OS_LINUX_USER)
    // TODO: implement timer in linux user space.
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTimerLK_t *ptHandle = (CamOsTimerLK_t *)ptTimer;
    if(ptHandle)
    {
        mod_timer(&ptHandle->tTimerID, jiffies + msecs_to_jiffies(nMsec));
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#endif
    return eRet;
}

int32_t CamOsAtomicRead(CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return ptAtomic->nCounter;
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return ptAtomic->nCounter;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_read((atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

void CamOsAtomicSet(CamOsAtomic_t *ptAtomic, int32_t nValue)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        ptAtomic->nCounter = nValue;
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        ptAtomic->nCounter = nValue;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        atomic_set((atomic_t *)ptAtomic, nValue);
    }
#endif
}

int32_t CamOsAtomicAddReturn(int32_t nValue, CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return __sync_add_and_fetch(&ptAtomic->nCounter, nValue);
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return __sync_add_and_fetch(&ptAtomic->nCounter, nValue);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_add_return(nValue, (atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

int32_t CamOsAtomicSubReturn(int32_t nValue, CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return __sync_sub_and_fetch(&ptAtomic->nCounter, nValue);
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return __sync_sub_and_fetch(&ptAtomic->nCounter, nValue);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_sub_return(nValue, (atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

int32_t CamOsAtomicSubAndTest(int32_t nValue, CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return !(__sync_sub_and_fetch(&ptAtomic->nCounter, nValue));
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return !(__sync_sub_and_fetch(&ptAtomic->nCounter, nValue));
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_sub_and_test(nValue, (atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

int32_t CamOsAtomicIncReturn(CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return __sync_fetch_and_add(&ptAtomic->nCounter, 1);
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return __sync_fetch_and_add(&ptAtomic->nCounter, 1);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_inc_return((atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

int32_t CamOsAtomicDecReturn(CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return __sync_fetch_and_sub(&ptAtomic->nCounter, 1);
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return __sync_fetch_and_sub(&ptAtomic->nCounter, 1);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_dec_return((atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

int32_t CamOsAtomicIncAndTest(CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return !(__sync_add_and_fetch(&ptAtomic->nCounter, 1));
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return !(__sync_add_and_fetch(&ptAtomic->nCounter, 1));
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_inc_and_test((atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

int32_t CamOsAtomicDecAndTest(CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return !(__sync_sub_and_fetch(&ptAtomic->nCounter, 1));
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return !(__sync_sub_and_fetch(&ptAtomic->nCounter, 1));
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_dec_and_test((atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

int32_t CamOsAtomicAddNegative(int32_t nValue, CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return (__sync_add_and_fetch(&ptAtomic->nCounter, nValue) < 0);
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return (__sync_add_and_fetch(&ptAtomic->nCounter, nValue) < 0);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_add_negative(nValue, (atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
extern CamOsRet_e _CamOsIdrInit(CamOsIdr_t *ptIdr);
extern void _CamOsIdrDestroy(CamOsIdr_t *ptIdr);
extern int32_t _CamOsIdrAlloc(CamOsIdr_t *ptIdr, void *pPtr, int32_t nStart, int32_t nEnd);
extern void _CamOsIdrRemove(CamOsIdr_t *ptIdr, int32_t nId);
extern void *_CamOsIdrFind(CamOsIdr_t *ptIdr, int32_t nId);
#endif

CamOsRet_e CamOsIdrInit(CamOsIdr_t *ptIdr)
{
    CamOsRet_e eRet = CAM_OS_OK;
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    eRet = _CamOsIdrInit(ptIdr);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsIdrLK_t *ptHandle = (CamOsIdrLK_t *)ptIdr;

    if(ptHandle)
    {
        idr_init(&ptHandle->tIdr);
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#endif
    return eRet;
}

void CamOsIdrDestroy(CamOsIdr_t *ptIdr)
{
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    _CamOsIdrDestroy(ptIdr);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsIdrLK_t *ptHandle = (CamOsIdrLK_t *)ptIdr;

    if(ptHandle)
    {
        idr_destroy(&ptHandle->tIdr);
    }
#endif
}

int32_t CamOsIdrAlloc(CamOsIdr_t *ptIdr, void *pPtr, int32_t nStart, int32_t nEnd)
{
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    return _CamOsIdrAlloc(ptIdr, pPtr, nStart, nEnd);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsIdrLK_t *ptHandle = (CamOsIdrLK_t *)ptIdr;

    if(ptHandle)
    {
        return idr_alloc(&ptHandle->tIdr, pPtr, nStart, nEnd, GFP_KERNEL);
    }
    else
        return -1;
#endif
}

void CamOsIdrRemove(CamOsIdr_t *ptIdr, int32_t nId)
{
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    _CamOsIdrRemove(ptIdr, nId);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsIdrLK_t *ptHandle = (CamOsIdrLK_t *)ptIdr;

    if(ptHandle)
    {
        idr_remove(&ptHandle->tIdr, nId);
    }
#endif
}

void *CamOsIdrFind(CamOsIdr_t *ptIdr, int32_t nId)
{
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    return _CamOsIdrFind(ptIdr, nId);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsIdrLK_t *ptHandle = (CamOsIdrLK_t *)ptIdr;

    if(ptHandle)
    {
        return idr_find(&ptHandle->tIdr, nId);
    }
    else
        return NULL;
#endif
}


CamOsMemSize_e CamOsPhysMemSize(void)
{
#ifdef CAM_OS_RTK
    return (CamOsMemSize_e)((*(volatile uint32_t *)(RIU_BASE_ADDR + RIU_MEM_SIZE_OFFSET) & 0xF000) >> 12);
#elif defined(CAM_OS_LINUX_USER)
    void *map_addr = 0;
    int32_t nMemFd = -1;
    uint32_t nRegs = 0;
    uint32_t nPageSize;

    nPageSize = getpagesize();

    do
    {
        if(0 > (nMemFd = open("/dev/mem", O_RDWR | O_SYNC)))
        {
            CamOsPrintf("%s open /dev/mem failed!!\n", __FUNCTION__);
            break;
        }

        map_addr = mmap(0, nPageSize, PROT_READ | PROT_WRITE, MAP_SHARED, nMemFd, (RIU_BASE_ADDR + RIU_MEM_SIZE_OFFSET) & ~(nPageSize - 1));

        if(map_addr == MAP_FAILED)
        {
            CamOsPrintf("%s mmap failed!!\n", __FUNCTION__);
            break;
        }
        else
        {
            nRegs = *(uint32_t *)(map_addr + RIU_MEM_SIZE_OFFSET % nPageSize);
            if(munmap(map_addr, nPageSize))
            {
                CamOsPrintf("%s: mumap %p is error(%s)\n", __func__,  map_addr, strerror(errno));
            }
        }
    }
    while(0);

    if(nMemFd >= 0)
    {
        close(nMemFd);
    }

    //CamOsPrintf("CamOsPhysMemSize: %d\n", (nRegs & 0xF000) >> 12);

    return (CamOsMemSize_e)((nRegs & 0xF000) >> 12);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMemSize_e eMemSize;
    void *pRegs;

    if((pRegs = ioremap(RIU_BASE_ADDR + RIU_MEM_SIZE_OFFSET, 4)) != NULL)
    {
        eMemSize = (CamOsMemSize_e)((readl(pRegs) & 0xF000) >> 12);
        iounmap(pRegs);
    }
    else
    {
        printk(KERN_WARNING "%s ioremap fail\n", __FUNCTION__);
    }

    return eMemSize;
#endif
}

uint32_t CamOsChipId(void)
{
#ifdef CAM_OS_RTK
    return *(volatile uint32_t *)(RIU_BASE_ADDR + RIU_CHIP_ID_OFFSET) & 0x3FFFFF;
#elif defined(CAM_OS_LINUX_USER)
    void *map_addr = 0;
    int32_t nMemFd = -1;
    uint32_t nRegs = 0;
    uint32_t nPageSize;

    nPageSize = getpagesize();

    do
    {
        if(0 > (nMemFd = open("/dev/mem", O_RDWR | O_SYNC)))
        {
            CamOsPrintf("%s open /dev/mem failed!!\n", __FUNCTION__);
            break;
        }

        map_addr = mmap(0, nPageSize, PROT_READ | PROT_WRITE, MAP_SHARED, nMemFd, (RIU_BASE_ADDR + RIU_CHIP_ID_OFFSET) & ~(nPageSize - 1));

        if(map_addr == MAP_FAILED)
        {
            CamOsPrintf("%s mmap failed!!\n", __FUNCTION__);
            break;
        }
        else
        {
            nRegs = *(uint32_t *)(map_addr + RIU_CHIP_ID_OFFSET % nPageSize);
            if(munmap(map_addr, nPageSize))
            {
                CamOsPrintf("%s: mumap %p is error(%s)\n", __func__,  map_addr, strerror(errno));
            }
        }
    }
    while(0);

    if(nMemFd >= 0)
    {
        close(nMemFd);
    }

    //CamOsPrintf("CamOsChipId: %d\n", nRegs & 0x3FFFFF);

    return nRegs & 0x3FFFFF;
#elif defined(CAM_OS_LINUX_KERNEL)
    uint32_t nId;
    void *pRegs;

    if((pRegs = ioremap(RIU_BASE_ADDR + RIU_CHIP_ID_OFFSET, 4)) != NULL)
    {
        nId = readl(pRegs) & 0x3FFFFF;
        iounmap(pRegs);
    }
    else
    {
        printk(KERN_WARNING "%s ioremap fail\n", __FUNCTION__);
    }

    return nId;
#endif
}

CamOsRet_e __always_inline CamOsInInterrupt(void)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    if(!RtkRunInIsrContext())
        eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_USER)
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
#elif defined(CAM_OS_LINUX_KERNEL)
    if(!in_interrupt())
        eRet = CAM_OS_FAIL;
#endif
    return eRet;
}

void __always_inline CamOsSmpMemoryBarrier(void)
{
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)
    CamOsPrintf("%s: This function is not supported in this platform\n\r", __FUNCTION__);
#elif defined(CAM_OS_LINUX_KERNEL)
    smp_mb();
#endif
}

#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_KERNEL)
char _szErrStrBuf[128];
#endif

char *CamOsStrError(int32_t nErrNo)
{
#ifdef CAM_OS_RTK
    sprintf(_szErrStrBuf, "errno is %d", nErrNo);
    return _szErrStrBuf;
#elif defined(CAM_OS_LINUX_USER)
    return strerror(nErrNo);
#elif defined(CAM_OS_LINUX_KERNEL)
    sprintf(_szErrStrBuf, "errno is %d", nErrNo);
    return _szErrStrBuf;
#endif
}

void CamOsPanic(const char *szMessage)
{
#ifdef CAM_OS_RTK
    CamOsPrintf("%s: %s\r\n", __FUNCTION__, szMessage);
    RtkExceptionRoutine_C(240, 0); // SYSTEM_ASSERT = 240
#elif defined(CAM_OS_LINUX_USER)
    CamOsPrintf("%s: %s\r\n", __FUNCTION__, szMessage);
    abort();
#elif defined(CAM_OS_LINUX_KERNEL)
    panic(szMessage);
#endif
}

long CamOsStrtol(const char *szStr, char** szEndptr, int32_t nBase)
{
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    return strtol(szStr, szEndptr, nBase);
#elif defined(CAM_OS_LINUX_KERNEL)
    return simple_strtol(szStr, szEndptr, nBase);
#endif
}

unsigned long CamOsStrtoul(const char *szStr, char** szEndptr, int32_t nBase)
{
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    return strtoul(szStr, szEndptr, nBase);
#elif defined(CAM_OS_LINUX_KERNEL)
    return simple_strtoul(szStr, szEndptr, nBase);
#endif
}

unsigned long long CamOsStrtoull(const char *szStr, char** szEndptr, int32_t nBase)
{
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    return strtoull(szStr, szEndptr, nBase);
#elif defined(CAM_OS_LINUX_KERNEL)
    return simple_strtoull(szStr, szEndptr, nBase);
#endif
}

unsigned long _CamOsFindFirstZeroBit(unsigned long *pAddr, unsigned long nSize,
                                     unsigned long nOffset)
{
    unsigned long *pLongBitmap = pAddr + (nOffset / CAM_OS_BITS_PER_LONG);
    unsigned long nResult = nOffset & ~(CAM_OS_BITS_PER_LONG - 1), nTemp;

    if(nOffset >= nSize)
        return nSize;

    nSize -= nResult;
    nOffset %= CAM_OS_BITS_PER_LONG;
    if(nOffset)
    {
        nTemp = *(pLongBitmap++);
        nTemp |= ~0UL >> (CAM_OS_BITS_PER_LONG - nOffset);
        if(nSize < CAM_OS_BITS_PER_LONG)
            goto IN_FIRST_BIT;

        if(~nTemp)
            goto IN_OTHER_BIT;

        nSize -= CAM_OS_BITS_PER_LONG;
        nResult += CAM_OS_BITS_PER_LONG;
    }

    //while (nSize > CAM_OS_BITS_PER_LONG)
    while(nSize & ~(CAM_OS_BITS_PER_LONG - 1))
    {
        if(~(nTemp = *pLongBitmap))
            goto IN_OTHER_BIT;

        nResult += CAM_OS_BITS_PER_LONG;
        nSize -= CAM_OS_BITS_PER_LONG;
        pLongBitmap++;
    }

    if(!nSize)
        return nResult;

    nTemp = *pLongBitmap;

IN_FIRST_BIT:
    nTemp |= ~0UL << nSize;
    if(nTemp == ~0UL)
        return nResult + nSize;

IN_OTHER_BIT:
    return nResult + CAM_OS_FFZ(nTemp);
}

static struct CamOsListHead_t *_CamOsListMerge(void *priv,
        int (*cmp)(void *priv, struct CamOsListHead_t *a,
                   struct CamOsListHead_t *b),
        struct CamOsListHead_t *a, struct CamOsListHead_t *b)
{
    struct CamOsListHead_t head, *tail = &head;

    while(a && b)
    {
        /* if equal, take 'a' -- important for sort stability */
        if((*cmp)(priv, a, b) <= 0)
        {
            tail->pNext = a;
            a = a->pNext;
        }
        else
        {
            tail->pNext = b;
            b = b->pNext;
        }
        tail = tail->pNext;
    }
    tail->pNext = a ? : b;
    return head.pNext;
}

static void _CamOsListMergeAndRestoreBackLinks(void *priv,
        int (*cmp)(void *priv, struct CamOsListHead_t *a,
                   struct CamOsListHead_t *b),
        struct CamOsListHead_t *head,
        struct CamOsListHead_t *a, struct CamOsListHead_t *b)
{
    struct CamOsListHead_t *tail = head;
    uint8_t count = 0;

    while(a && b)
    {
        /* if equal, take 'a' -- important for sort stability */
        if((*cmp)(priv, a, b) <= 0)
        {
            tail->pNext = a;
            a->pPrev = tail;
            a = a->pNext;
        }
        else
        {
            tail->pNext = b;
            b->pPrev = tail;
            b = b->pNext;
        }
        tail = tail->pNext;
    }
    tail->pNext = a ? : b;

    do
    {
        /*
         * In worst cases this loop may run many iterations.
         * Continue callbacks to the client even though no
         * element comparison is needed, so the client's cmp()
         * routine can invoke cond_resched() periodically.
         */
        if(!(++count))
            (*cmp)(priv, tail->pNext, tail->pNext);

        tail->pNext->pPrev = tail;
        tail = tail->pNext;
    }
    while(tail->pNext);

    tail->pNext = head;
    head->pPrev = tail;
}

#define CAM_OS_MAX_LIST_LENGTH_BITS 20
void CamOsListSort(void *priv, struct CamOsListHead_t *head,
                   int (*cmp)(void *priv, struct CamOsListHead_t *a,
                              struct CamOsListHead_t *b))
{
    struct CamOsListHead_t *part[CAM_OS_MAX_LIST_LENGTH_BITS + 1]; /* sorted partial lists
                        -- last slot is a sentinel */
    int lev;  /* index into part[] */
    int max_lev = 0;
    struct CamOsListHead_t *list;

    if(CAM_OS_LIST_EMPTY(head))
        return;

    memset(part, 0, sizeof(part));

    head->pPrev->pNext = NULL;
    list = head->pNext;

    while(list)
    {
        struct CamOsListHead_t *cur = list;
        list = list->pNext;
        cur->pNext = NULL;

        for(lev = 0; part[lev]; lev++)
        {
            cur = _CamOsListMerge(priv, cmp, part[lev], cur);
            part[lev] = NULL;
        }
        if(lev > max_lev)
        {
            if(lev >= CAM_OS_ARRAY_SIZE(part) - 1)
            {
                CamOsPrintf("list too long for efficiency\n");
                lev--;
            }
            max_lev = lev;
        }
        part[lev] = cur;
    }

    for(lev = 0; lev < max_lev; lev++)
        if(part[lev])
            list = _CamOsListMerge(priv, cmp, part[lev], list);

    _CamOsListMergeAndRestoreBackLinks(priv, cmp, head, part[max_lev], list);
}
