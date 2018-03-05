/*
 * cam_os_wrapper_test.c
 *
 *  Created on: Mar 20, 2017
 *      Author: giggs.huang
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "cam_os_wrapper.h"
#include "cam_os_util_list.h"
#include "cam_os_util_bug.h"
#include "cam_os_util_hash.h"
#include "cam_os_util_bitmap.h"

#ifdef CAM_OS_RTK
#include "sys_sys_core.h"
#include "sys_sys_isw_cli.h"
#endif

static uint32_t _gMutexTestCnt = 0;
static uint32_t _gTestCnt = 0;
static void _TestCamOsThread(void);
static void _TestCamOsMutex(void);
static void _TestCamOsDmem(void);
static void _TestCamOsMem(void);
static void _TestCamOsTimer(void);
static void _TestCamOsTsem(void);
static void _TestCamOsRwsem(void);
static void _TestCamOsDiv64(void);
static void _TestCamOsSystemTime(void);
static void _TestCamOsPhysMemSize(void);
static void _TestCamOsChipId(void);
static void _TestCamOsTcond(void);
static void _TestCamOsBitmap(void);
static void _TestCamOsHash(void);
static void _TestCamOsIdr(void);

typedef void (*_TEST_FUNC_PTR)(void);

typedef struct
{
    _TEST_FUNC_PTR pFnTest;
    uint8_t             *pTestDesc;
} CamOsWrapperTestItem_t, *pCamOsWrapperTestItem;

static const CamOsWrapperTestItem_t aCamOsWrapperTestItemTbl[] =
{
    {_TestCamOsThread,       (uint8_t*)"test thread function"},
    {_TestCamOsMutex,        (uint8_t*)"test mutex function"},
    {_TestCamOsDmem,         (uint8_t*)"test direct memory allocation"},
    {_TestCamOsMem,          (uint8_t*)"test memory allocation"},
    {_TestCamOsTimer,        (uint8_t*)"test timer function"},
    {_TestCamOsTsem,         (uint8_t*)"test thread semaphore function"},
    {_TestCamOsRwsem,        (uint8_t*)"test rw semaphore function"},
    {_TestCamOsTcond,        (uint8_t*)"test thread condition wait function"},
    {_TestCamOsBitmap,       (uint8_t*)"test bitmap operation"},
    {_TestCamOsHash,         (uint8_t*)"test hash operation"},
    {_TestCamOsIdr,          (uint8_t*)"test IDR operation"},
    {_TestCamOsDiv64,        (uint8_t*)"test 64-bit division"},
    {_TestCamOsSystemTime,   (uint8_t*)"test system time function"},
    {_TestCamOsPhysMemSize,  (uint8_t*)"test physical memory size"},
    {_TestCamOsChipId,       (uint8_t*)"test chip ID"},
    {NULL,                   (uint8_t*)"test all function"},
};

#define TEST_ITEM_NUM (sizeof(aCamOsWrapperTestItemTbl)/sizeof(CamOsWrapperTestItem_t))

static void _ShowTestMenu(void)
{
    int32_t i;

    CamOsPrintf("cam_os_wrapper test menu: \n");

    for (i = 0; i < TEST_ITEM_NUM; i++)
    {
        CamOsPrintf("\t%2d) %s\r\n", i, aCamOsWrapperTestItemTbl[i].pTestDesc);
    }
}

#ifdef CAM_OS_RTK
int32_t CamOsWrapperTest(CLI_t *pCli, char *p)
{
    int32_t i, nParamCnt;
    uint32_t  nCaseNum = 0;
    char *pEnd;


    nParamCnt = CliTokenCount(pCli);

    if(nParamCnt < 1)
    {
        _ShowTestMenu();
        return eCLI_PARSE_INPUT_ERROR;
    }

    for(i = 0; i < nParamCnt; i++)
    {
        pCli->tokenLvl++;
        p = CliTokenPop(pCli);
        if(i == 0)
        {
            //CLIDEBUG(("p: %s, len: %d\n", p, strlen(p)));
            //*pV = _strtoul(p, &pEnd, base);
            nCaseNum = strtoul(p, &pEnd, 10);
            //CLIDEBUG(("*pEnd = %d\n", *pEnd));
            if(p == pEnd || *pEnd)
            {
                CamOsPrintf("Invalid input\n");
                return eCLI_PARSE_ERROR;
            }
        }
    }

    if (nCaseNum < TEST_ITEM_NUM)
    {
        if (nCaseNum == (TEST_ITEM_NUM - 1))
        {
            for (i = 0; i < TEST_ITEM_NUM; i++)
            {
                if (aCamOsWrapperTestItemTbl[i].pFnTest)
                {
                    CamOsPrintf("===============================================\r\n");
                    CamOsPrintf("%s\r\n", aCamOsWrapperTestItemTbl[i].pTestDesc);
                    CamOsPrintf("===============================================\r\n");
                    aCamOsWrapperTestItemTbl[i].pFnTest();
                    CamOsPrintf("\r\n");
                }
            }
        }
        else
        {
            if (aCamOsWrapperTestItemTbl[nCaseNum].pFnTest)
            {
                CamOsPrintf("===============================================\r\n");
                CamOsPrintf("%s\r\n", aCamOsWrapperTestItemTbl[nCaseNum].pTestDesc);
                CamOsPrintf("===============================================\r\n");
                aCamOsWrapperTestItemTbl[nCaseNum].pFnTest();
            }
        }
    }
    else
    {
        _ShowTestMenu();
        return eCLI_PARSE_ERROR;
    }

    return eCLI_PARSE_OK;
}
#else
int main(int argc, char *argv[])
{
    uint32_t nCaseNum = 0;
    int32_t i;

    if(argc < 2)
    {
        _ShowTestMenu();
        return -1;
    }

    nCaseNum = atoi(argv[1]);

    if (nCaseNum < TEST_ITEM_NUM)
    {
        if (nCaseNum == (TEST_ITEM_NUM - 1))
        {
            for (i = 0; i < TEST_ITEM_NUM; i++)
            {
                if (aCamOsWrapperTestItemTbl[i].pFnTest)
                {
                    CamOsPrintf("===============================================\r\n");
                    CamOsPrintf("%s\r\n", aCamOsWrapperTestItemTbl[i].pTestDesc);
                    CamOsPrintf("===============================================\r\n");
                    aCamOsWrapperTestItemTbl[i].pFnTest();
                    CamOsPrintf("\r\n");
                }
            }
        }
        else
        {
            if (aCamOsWrapperTestItemTbl[nCaseNum].pFnTest)
            {
                CamOsPrintf("===============================================\r\n");
                CamOsPrintf("%s\r\n", aCamOsWrapperTestItemTbl[nCaseNum].pTestDesc);
                CamOsPrintf("===============================================\r\n");
                aCamOsWrapperTestItemTbl[nCaseNum].pFnTest();
            }
        }
    }
    else
    {
        _ShowTestMenu();
        return -1;
    }

    return 0;
}
#endif

CamOsAtomic_t _gtThreadAtomic;

static void _CamOsThreadTestEntry0(void *pUserdata)
{
    int32_t *pnArg = (int32_t *)pUserdata;
    int32_t i = 0;

    for(i = 0; i < 5; i++)
    {
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _CamOsThreadTestEntry0 (sleep %dms)  count: %d\n", __LINE__, *pnArg, i);
        CamOsMsSleep(*pnArg);
    }
}

static int _CamOsThreadTestEntry1(void *pUserData)
{
    int32_t nTestCounter=0;

    CamOsMsSleep(10);

    while (nTestCounter < 10000)
    {
        nTestCounter++;
        if ((nTestCounter % 500) == 0)
            CamOsPrintf("%s run...\n", __FUNCTION__);
    }

    CamOsAtomicIncReturn(&_gtThreadAtomic);
    while (CamOsAtomicRead(&_gtThreadAtomic) < 3)
        CamOsMsSleep(100);

    CamOsPrintf("%s free run end\n", __FUNCTION__);
#ifdef CAM_OS_RTK
    while (CamOsAtomicRead(&_gtThreadAtomic) < 5)
    {
        if (CamOsAtomicRead(&_gtThreadAtomic) == 3)
        {
            CamOsPrintf("%s enter CamOsThreadSchedule\n", __FUNCTION__);
            CamOsThreadSchedule(0, 2000);
            CamOsPrintf("%s leave CamOsThreadSchedule\n", __FUNCTION__);
            CamOsAtomicIncReturn(&_gtThreadAtomic);
        }

        CamOsPrintf("%s run...\n", __FUNCTION__);
        CamOsMsSleep(300);
    }
#endif
    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static int _CamOsThreadTestEntry2(void *pUserData)
{
    int32_t nTestCounter=0;

    CamOsMsSleep(10);

    while (nTestCounter < 10000)
    {
        nTestCounter++;
        if ((nTestCounter % 500) == 0)
            CamOsPrintf("%s run...\n", __FUNCTION__);
    }

    CamOsAtomicIncReturn(&_gtThreadAtomic);
    while (CamOsAtomicRead(&_gtThreadAtomic) < 3)
        CamOsMsSleep(100);

    CamOsPrintf("%s free run end\n", __FUNCTION__);
#ifdef CAM_OS_RTK
    while (CamOsAtomicRead(&_gtThreadAtomic) < 6)
    {
        if (CamOsAtomicRead(&_gtThreadAtomic) == 5)
        {
            CamOsPrintf("%s enter CamOsThreadSchedule\n", __FUNCTION__);
            CamOsThreadSchedule(1, CAM_OS_MAX_TIMEOUT);
            CamOsPrintf("%s leave CamOsThreadSchedule\n", __FUNCTION__);
            CamOsAtomicIncReturn(&_gtThreadAtomic);
        }
        CamOsPrintf("%s run...\n", __FUNCTION__);
        CamOsMsSleep(300);
    }
#endif
    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static void _TestCamOsThread(void)
{
    CamOsThread TaskHandle0, TaskHandle1;
    CamOsThreadAttrb_t tAttr = {0};
    int32_t nTaskArg0 = 1000, nTaskArg1 = 1500;
    char szTaskName0[32], szTaskName1[32];

    tAttr.nPriority = 50;
    tAttr.nStackSize = 0;
    CamOsThreadCreate(&TaskHandle0, &tAttr, (void *)_CamOsThreadTestEntry0, (void *)&nTaskArg0);
    //CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsThread get taskid: %d\n", __LINE__, TaskHandle0.eHandleObj);

    tAttr.nPriority = 50;
    tAttr.nStackSize = 0;
    CamOsThreadCreate(&TaskHandle1, &tAttr, (void *)_CamOsThreadTestEntry0, (void *)&nTaskArg1);
    //CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsThread get taskid: %d\n", __LINE__, TaskHandle1.eHandleObj);

    CamOsMsSleep(1000);
    CamOsThreadGetName(TaskHandle0, szTaskName0, sizeof(szTaskName0));
    CamOsThreadGetName(TaskHandle1, szTaskName1, sizeof(szTaskName1));
    CamOsPrintf("Test Get Name: %s %s\n", szTaskName0, szTaskName1);

    CamOsMsSleep(1000);
    CamOsThreadSetName(TaskHandle0, "ABCDEFGHIJKLMNO");
    CamOsThreadSetName(TaskHandle1, "abcdefghijklmno");

    CamOsMsSleep(1000);
    CamOsThreadGetName(TaskHandle0, szTaskName0, sizeof(szTaskName0));
    CamOsThreadGetName(TaskHandle1, szTaskName1, sizeof(szTaskName1));
    CamOsPrintf("Test Get Name: %s %s\n", szTaskName0, szTaskName1);

    CamOsThreadJoin(TaskHandle0);
    CamOsThreadJoin(TaskHandle1);

    CamOsAtomicSet(&_gtThreadAtomic, 0);
    CamOsPrintf("### Priority: ThreadTest1 < ThreadTest2\n");
    tAttr.nPriority = 20;
    tAttr.nStackSize = 0;
    CamOsThreadCreate(&TaskHandle0, &tAttr, (void *)_CamOsThreadTestEntry1, NULL);
    tAttr.nPriority = 50;
    tAttr.nStackSize = 0;
    CamOsThreadCreate(&TaskHandle1, &tAttr, (void *)_CamOsThreadTestEntry2, NULL);

    while (CamOsAtomicRead(&_gtThreadAtomic) < 2)
        CamOsMsSleep(100);

    CamOsMsSleep(1000);

    CamOsAtomicIncReturn(&_gtThreadAtomic);
#ifdef CAM_OS_RTK
    while (CamOsAtomicRead(&_gtThreadAtomic) < 4)
        CamOsMsSleep(100);

    CamOsMsSleep(2000);

    CamOsPrintf("### Wake up _CamOsThreadTestEntry2\n");
    CamOsThreadWakeUp(TaskHandle1);

    CamOsMsSleep(1000);

    CamOsAtomicIncReturn(&_gtThreadAtomic);
#endif
    CamOsThreadJoin(TaskHandle0);
    CamOsThreadJoin(TaskHandle1);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsThread delete task\n", __LINE__);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsThread test end!!!\n", __LINE__);
}

static void _CamOsMutexTestEntry0(void *pUserdata)
{
    CamOsMutex_t *ptMutex = (CamOsMutex_t *)pUserdata;
    uint32_t i = 0;

    for(i = 0; i < 100; i++)
    {
        CamOsMutexLock(ptMutex);
        _gMutexTestCnt++;
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _CamOsThreadTestEntry0 start  count: %d\n", __LINE__, _gMutexTestCnt);
        CamOsMsSleep(3);
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _CamOsThreadTestEntry0 end    count: %d\n", __LINE__, _gMutexTestCnt);
        CamOsMutexUnlock(ptMutex);
    }
}

static void _CamOsMutexTestEntry1(void *pUserdata)
{
    CamOsMutex_t *ptMutex = (CamOsMutex_t *)pUserdata;
    uint32_t i = 0;

    for(i = 0; i < 100; i++)
    {
        CamOsMutexLock(ptMutex);
        _gMutexTestCnt++;
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _CamOsThreadTestEntry1 start  count: %d\n", __LINE__, _gMutexTestCnt);
        CamOsMsSleep(2);
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _CamOsThreadTestEntry1 end    count: %d\n", __LINE__, _gMutexTestCnt);
        CamOsMutexUnlock(ptMutex);
    }
}

static void _CamOsMutexTestEntry2(void *pUserdata)
{
    CamOsMutex_t *ptMutex = (CamOsMutex_t *)pUserdata;
    uint32_t i = 0;

    for(i = 0; i < 100; i++)
    {
        CamOsMutexLock(ptMutex);
        _gMutexTestCnt++;
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _CamOsThreadTestEntry2 start  count: %d\n", __LINE__, _gMutexTestCnt);
        CamOsMsSleep(5);
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _CamOsThreadTestEntry2 end    count: %d\n", __LINE__, _gMutexTestCnt);
        CamOsMutexUnlock(ptMutex);
    }
}

static void _TestCamOsMutex(void)
{
    CamOsThread TaskHandle0, TaskHandle1, TaskHandle2;
    CamOsThreadAttrb_t tAttr = {0};
    CamOsMutex_t tCamOsMutex;

    CamOsMutexInit(&tCamOsMutex);

    tAttr.nPriority = 50;
    tAttr.nStackSize = 0;
    CamOsThreadCreate(&TaskHandle0, &tAttr, (void *)_CamOsMutexTestEntry0, &tCamOsMutex);

    tAttr.nPriority = 50;
    tAttr.nStackSize = 0;
    CamOsThreadCreate(&TaskHandle1, &tAttr, (void *)_CamOsMutexTestEntry1, &tCamOsMutex);

    tAttr.nPriority = 100;
    tAttr.nStackSize = 0;
    CamOsThreadCreate(&TaskHandle2, &tAttr, (void *)_CamOsMutexTestEntry2, &tCamOsMutex);

    CamOsThreadJoin(TaskHandle0);
    CamOsThreadJoin(TaskHandle1);
    CamOsThreadJoin(TaskHandle2);

    CamOsMutexDestroy(&tCamOsMutex);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsMutex delete task\n", __LINE__);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsMutex test end!!!\n", __LINE__);
}

static void _TestCamOsDmem(void)
{
#ifdef CAM_OS_RTK
    void *pVirtPtr = NULL;
    void *pMiuAddr = NULL;
    void *pPhysAddr = NULL;

    CamOsDirectMemAlloc("TESTDMEM", 1025, &pVirtPtr, &pPhysAddr, &pMiuAddr);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsDirectMemAlloc get addr:  pVirtPtr 0x%08X  pPhysAddr 0x%08X  pMiuAddr 0x%08X\n",
                __LINE__, (uint32_t)pVirtPtr, (uint32_t)pPhysAddr, (uint32_t)pMiuAddr);


    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsDmem: write cached ptr=0x%x, value = 0x%x\n", __LINE__, (uint32_t)pPhysAddr, 0x12345678);
    *(uint32_t *)pPhysAddr = 0x12345678;
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsDmem: read cached ptr=0x%x, value = 0x%x\n", __LINE__, (uint32_t)pPhysAddr, *(uint32_t *)pPhysAddr);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsDmem: read uncached ptr=0x%x, value = 0x%x\n", __LINE__, (uint32_t)pVirtPtr, *(uint32_t *)pVirtPtr);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsDmem: write uncached ptr=0x%x, value = 0x%x\n", __LINE__, (uint32_t)pVirtPtr, 0x87654321);
    *(uint32_t *)pVirtPtr = 0x87654321;
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsDmem: read cached ptr=0x%x, value = 0x%x\n", __LINE__, (uint32_t)pPhysAddr, *(uint32_t *)pPhysAddr);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsDmem: read uncached ptr=0x%x, value = 0x%x\n", __LINE__, (uint32_t)pVirtPtr, *(uint32_t *)pVirtPtr);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsDmem: flush cached ptr=0x%x\n", __LINE__, (uint32_t)pPhysAddr);
    CamOsDirectMemFlush(pVirtPtr);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsDmem: read cached ptr=0x%x, value = 0x%x\n", __LINE__, (uint32_t)pPhysAddr, *(uint32_t *)pPhysAddr);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsDmem: read uncached ptr=0x%x, value = 0x%x\n", __LINE__, (uint32_t)pVirtPtr, *(uint32_t *)pVirtPtr);

    CamOsDirectMemRelease((uint8_t *)pVirtPtr, 1025);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d]_TestCamOsDmem test end!!!\n", __LINE__);
#else
    void *pVirtPtr = NULL;
    void *pMiuAddr = NULL;

    CamOsDirectMemAlloc("TESTDMEM", 1025, &pVirtPtr, NULL, &pMiuAddr);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsDirectMemAlloc get addr:  pVirtPtr 0x%08X  pMiuAddr 0x%08X\n",
                __LINE__, (uint32_t)pVirtPtr, (uint32_t)pMiuAddr);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsDmem: read uncached ptr=0x%x, value = 0x%x\n", __LINE__, (uint32_t)pVirtPtr, *(uint32_t *)pVirtPtr);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsDmem: write uncached ptr=0x%x, value = 0x%x\n", __LINE__, (uint32_t)pVirtPtr, 0x87654321);
    *(uint32_t *)pVirtPtr = 0x87654321;

    CamOsDirectMemFlush(pVirtPtr);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsDmem: read uncached ptr=0x%x, value = 0x%x\n", __LINE__, (uint32_t)pVirtPtr, *(uint32_t *)pVirtPtr);

    CamOsDirectMemRelease((uint8_t *)pVirtPtr, 1025);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d]_TestCamOsDmem test end!!!\n", __LINE__);
#endif
}

static void _TestCamOsMem(void)
{
    void *pUserPtr = NULL;

    // test CamOsMemAlloc
    pUserPtr = CamOsMemAlloc(2048);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsMemAlloc get addr:  pUserPtr 0x%08X\n", __LINE__, (uint32_t)pUserPtr);

    memset(pUserPtr, 0x5A, 2048);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] value in address 0x%08X is 0x%08X\n", __LINE__, (uint32_t)pUserPtr, *(uint32_t *)pUserPtr);

    CamOsMemRelease(pUserPtr);
    pUserPtr = NULL;
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsMemRelease free buffer\n", __LINE__);

    // test CamOsMemCalloc
    pUserPtr = CamOsMemCalloc(2048, 1);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsMemCalloc get addr:  pUserPtr 0x%08X\n", __LINE__, (uint32_t)pUserPtr);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] value in address 0x%08X is 0x%08X\n", __LINE__, (uint32_t)pUserPtr, *(uint32_t *)pUserPtr);

    CamOsMemRelease(pUserPtr);
    pUserPtr = NULL;
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsMemRelease free buffer\n", __LINE__);

    // test CamOsMemRealloc
    pUserPtr = CamOsMemAlloc(2048);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CAM_OS_MemMalloc get addr:  pUserPtr 0x%08X\n", __LINE__, (uint32_t)pUserPtr);

    pUserPtr = CamOsMemRealloc(pUserPtr, 4096);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsMemRealloc get addr:  pUserPtr 0x%08X\n", __LINE__, (uint32_t)pUserPtr);

    CamOsMemRelease(pUserPtr);
    pUserPtr = NULL;
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsMemRelease free buffer\n", __LINE__);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsMem test end!!!\n", __LINE__);
}

static void _TestCamOsTimer(void)
{
    int32_t nCnt = 0;
    CamOsTimespec_t tTv = {0}, tTv_prev = {0};

    for(nCnt = 0; nCnt <= 10; nCnt++)
    {
        CamOsGetMonotonicTime(&tTv);
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] rtk_os_time: %d.%09d    diff: %d\n", __LINE__, tTv.nSec, tTv.nNanoSec,
                    (tTv.nSec - tTv_prev.nSec)*1000000+((int32_t)tTv.nNanoSec - (int32_t)tTv_prev.nNanoSec)/1000);
        memcpy((void *)&tTv_prev, (void *)&tTv, sizeof(tTv));
        CamOsMsSleep(1000);
    }
}

static int32_t _CamOsTsemTestEntry0(void *pUserData)
{
    CamOsTsem_t *pSem = (CamOsTsem_t *)pUserData;
    CamOsRet_e eRet;

    CamOsMsSleep(1000);

    _gTestCnt++;//1

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(10);

    CamOsPrintf("%s CamOsTsemDown start\n", __FUNCTION__);
    CamOsTsemDown(pSem);
    CamOsPrintf("%s CamOsTsemDown end\n", __FUNCTION__);

    if (_gTestCnt != 2)
    {
        CamOsPrintf("%s: step 1 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsMsSleep(300);

    _gTestCnt++;//3

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(100);

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(100);

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    _gTestCnt++;//4

    CamOsMsSleep(5000);

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(10);

    CamOsPrintf("%s CamOsTsemTimedDown start\n", __FUNCTION__);
    eRet = CamOsTsemTimedDown(pSem, 3000);
    CamOsPrintf("%s CamOsTsemTimedDown end (%s)\n", __FUNCTION__, (eRet == CAM_OS_OK) ? "wakened" : "timeout");

    if (_gTestCnt != 5)
    {
        CamOsPrintf("%s: step 2 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsMsSleep(1000);

    _gTestCnt++;//6

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(10);

    CamOsPrintf("%s CamOsTsemTimedDown start\n", __FUNCTION__);
    eRet = CamOsTsemTimedDown(pSem, 5000);
    CamOsPrintf("%s CamOsTsemTimedDown end (%s)\n", __FUNCTION__, (eRet == CAM_OS_OK) ? "wakened" : "timeout");

    if (eRet != CAM_OS_OK)
    {
        CamOsPrintf("%s: step 3 fail!(eRet=%d)\n", __FUNCTION__, eRet);
        return -1;
    }

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static int32_t _CamOsTsemTestEntry1(void *pUserData)
{
    CamOsTsem_t *pSem = (CamOsTsem_t *)pUserData;
    CamOsRet_e eRet;

    CamOsPrintf("%s CamOsTsemDown start\n", __FUNCTION__);
    CamOsTsemDown(pSem);
    CamOsPrintf("%s CamOsTsemDown end\n", __FUNCTION__);

    if (_gTestCnt != 1)
    {
        CamOsPrintf("%s: step 1 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsMsSleep(1000);

    _gTestCnt++;//2

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(100);

    CamOsPrintf("%s CamOsTsemDown start\n", __FUNCTION__);
    CamOsTsemDown(pSem);
    CamOsPrintf("%s CamOsTsemDown end\n", __FUNCTION__);

    if (_gTestCnt != 3)
    {
        CamOsPrintf("%s: step 2 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsPrintf("%s CamOsTsemDown start\n", __FUNCTION__);
    CamOsTsemDown(pSem);
    CamOsPrintf("%s CamOsTsemDown end\n", __FUNCTION__);

    CamOsPrintf("%s CamOsTsemDown start\n", __FUNCTION__);
    CamOsTsemDown(pSem);
    CamOsPrintf("%s CamOsTsemDown end\n", __FUNCTION__);

    CamOsMsSleep(3000);

    CamOsPrintf("%s CamOsTsemDown start\n", __FUNCTION__);
    CamOsTsemDown(pSem);
    CamOsPrintf("%s CamOsTsemDown end\n", __FUNCTION__);

    if (_gTestCnt != 4)
    {
        CamOsPrintf("%s: step 3 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsMsSleep(1000);

    _gTestCnt++;//5

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(100);

    CamOsPrintf("%s CamOsTsemTimedDown start\n", __FUNCTION__);
    eRet = CamOsTsemTimedDown(pSem, 3000);
    CamOsPrintf("%s CamOsTsemTimedDown end (%s)\n", __FUNCTION__, (eRet == CAM_OS_OK) ? "wakened" : "timeout");

    if (_gTestCnt != 6)
    {
        CamOsPrintf("%s: step 4 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsPrintf("%s CamOsTsemTimedDown start\n", __FUNCTION__);
    eRet = CamOsTsemTimedDown(pSem, 3000);
    CamOsPrintf("%s CamOsTsemTimedDown end (%s)\n", __FUNCTION__, (eRet == CAM_OS_OK) ? "wakened" : "timeout");

    if (eRet != CAM_OS_TIMEOUT)
    {
        CamOsPrintf("%s: step 5 fail!(eRet=%d)\n", __FUNCTION__, eRet);
        return -1;
    }

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);


    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static int32_t _CamOsRwsemTestEntry0(void *pUserData)
{
    CamOsRwsem_t *tpRwsem = (CamOsRwsem_t *)pUserData;

    CamOsPrintf("%s CamOsRwsemDownRead start\n", __FUNCTION__);
    CamOsRwsemDownRead(tpRwsem);
    CamOsPrintf("%s CamOsRwsemDownRead end\n", __FUNCTION__);

    CamOsMsSleep(2000);

    if (_gTestCnt != 0)
    {
        CamOsPrintf("%s: step 1 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsPrintf("%s CamOsRwsemUpRead\n", __FUNCTION__);
    CamOsRwsemUpRead(tpRwsem);

    CamOsMsSleep(50);

    if (_gTestCnt != 1)
    {
        CamOsPrintf("%s: step 2 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsPrintf("%s CamOsRwsemDownWrite start\n", __FUNCTION__);
    CamOsRwsemDownWrite(tpRwsem);
    CamOsPrintf("%s CamOsRwsemDownWrite end\n", __FUNCTION__);

    CamOsMsSleep(1500);

    _gTestCnt++;

    CamOsPrintf("%s CamOsRwsemUpWrite\n", __FUNCTION__);
    CamOsRwsemUpWrite(tpRwsem);

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static int32_t _CamOsRwsemTestEntry1(void *pUserData)
{
    CamOsRwsem_t *tpRwsem = (CamOsRwsem_t *)pUserData;
    CamOsRet_e eRet;

    CamOsMsSleep(20);
    CamOsPrintf("%s CamOsRwsemTryDownRead start\n", __FUNCTION__);
    if (CAM_OS_OK != (eRet = CamOsRwsemTryDownRead(tpRwsem)))
    {
        CamOsPrintf("%s: step 1 fail!(eRet=%d)\n", __FUNCTION__, eRet);
        return -1;
    }
    CamOsPrintf("%s CamOsRwsemTryDownRead end\n", __FUNCTION__);

    CamOsMsSleep(2000);

    if (_gTestCnt != 0)
    {
        CamOsPrintf("%s: step 2 fail!\n", __FUNCTION__);
        return -1;
    }

    CamOsPrintf("%s CamOsRwsemUpRead\n", __FUNCTION__);
    CamOsRwsemUpRead(tpRwsem);

    CamOsMsSleep(50);

    if (_gTestCnt != 1)
    {
        CamOsPrintf("%s: step 3 fail!\n", __FUNCTION__);
        return -1;
    }

    CamOsMsSleep(1000);

    CamOsPrintf("%s CamOsRwsemDownWrite start\n", __FUNCTION__);
    CamOsRwsemDownWrite(tpRwsem);
    CamOsPrintf("%s CamOsRwsemDownWrite end\n", __FUNCTION__);

    if (_gTestCnt != 2)
    {
        CamOsPrintf("%s: step 4 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    _gTestCnt++;

    CamOsMsSleep(100);

    CamOsPrintf("%s CamOsRwsemUpWrite\n", __FUNCTION__);
    CamOsRwsemUpWrite(tpRwsem);

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static int32_t _CamOsRwsemTestEntry2(void *pUserData)
{
    CamOsRwsem_t *tpRwsem = (CamOsRwsem_t *)pUserData;
    CamOsRet_e eRet;

    CamOsMsSleep(50);

    CamOsPrintf("%s CamOsRwsemTryDownWrite start\n", __FUNCTION__);
    if (CAM_OS_RESOURCE_BUSY != (eRet = CamOsRwsemTryDownWrite(tpRwsem)))
    {
        CamOsPrintf("%s: step 1 fail!(eRet=%d)\n", __FUNCTION__, eRet);
        return -1;
    }
    CamOsPrintf("%s CamOsRwsemTryDownWrite end\n", __FUNCTION__);

    CamOsPrintf("%s CamOsRwsemDownWrite start\n", __FUNCTION__);
    CamOsRwsemDownWrite(tpRwsem);
    CamOsPrintf("%s CamOsRwsemDownWrite end\n", __FUNCTION__);

    _gTestCnt++;

    CamOsPrintf("%s CamOsRwsemUpWrite\n", __FUNCTION__);
    CamOsRwsemUpWrite(tpRwsem);

    CamOsMsSleep(200);

    CamOsPrintf("%s CamOsRwsemDownRead start\n", __FUNCTION__);
    CamOsRwsemDownRead(tpRwsem);
    CamOsPrintf("%s CamOsRwsemDownRead end\n", __FUNCTION__);

    if (_gTestCnt != 2 && _gTestCnt != 3)
    {
        CamOsPrintf("%s: step 2 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsPrintf("%s CamOsRwsemUpRead\n", __FUNCTION__);
    CamOsRwsemUpRead(tpRwsem);

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static void _TestCamOsTsem(void)
{
    static CamOsThread TaskHandle0, TaskHandle1;
    CamOsTsem_t tSem;

    _gTestCnt = 0;

    CamOsTsemInit(&tSem, 0);
    CamOsThreadCreate(&TaskHandle0, NULL, (void *)_CamOsTsemTestEntry0, &tSem);
    CamOsThreadCreate(&TaskHandle1, NULL, (void *)_CamOsTsemTestEntry1, &tSem);

    CamOsThreadJoin(TaskHandle0);
    CamOsThreadJoin(TaskHandle1);

    CamOsTsemDeinit(&tSem);
}

static void _TestCamOsRwsem(void)
{
    static CamOsThread TaskHandle0, TaskHandle1, TaskHandle2;
    CamOsRwsem_t tRwsem;

    _gTestCnt = 0;

    CamOsRwsemInit(&tRwsem);
    CamOsThreadCreate(&TaskHandle0, NULL, (void *)_CamOsRwsemTestEntry0, &tRwsem);
    CamOsThreadCreate(&TaskHandle1, NULL, (void *)_CamOsRwsemTestEntry1, &tRwsem);
    CamOsThreadCreate(&TaskHandle2, NULL, (void *)_CamOsRwsemTestEntry2, &tRwsem);

    CamOsThreadJoin(TaskHandle0);
    CamOsThreadJoin(TaskHandle1);
    CamOsThreadJoin(TaskHandle2);

    CamOsRwsemDeinit(&tRwsem);
}

static void _TestCamOsDiv64(void)
{
    uint64_t nDividendU64 = 0, nDivisorU64 = 0, nResultU64 = 0, nRemainderU64 = 0;
    int64_t nDividendS64 = 0, nDivisorS64 = 0, nResultS64 = 0, nRemainderS64 = 0;

    CamOsPrintf("Unsigned 64 bit dividend:");
    CamOsScanf("%llu", &nDividendU64);
    CamOsPrintf("Unsigned 64 bit divisor:");
    CamOsScanf("%llu", &nDivisorU64);

    CamOsPrintf("Directly: %llu / %llu = %llu    remaind %llu\n", nDividendU64, nDivisorU64, nDividendU64 / nDivisorU64, nDividendU64 % nDivisorU64);
    nResultU64 = CamOsMathDivU64(nDividendU64, nDivisorU64, &nRemainderU64);
    CamOsPrintf("By Div64: %llu / %llu = %llu    remaind %llu\n", nDividendU64, nDivisorU64, nResultU64, nRemainderU64);


    CamOsPrintf("Signed 64 bit dividend:");
    CamOsScanf("%lld", &nDividendS64);
    CamOsPrintf("Signed 64 bit divisor:");
    CamOsScanf("%lld", &nDivisorS64);

    CamOsPrintf("Directly: %lld / %lld = %lld    remaind %lld\n", nDividendS64, nDivisorS64, nDividendS64 / nDivisorS64, nDividendS64 % nDivisorS64);
    nResultS64 = CamOsMathDivS64(nDividendS64, nDivisorS64, &nRemainderS64);
    CamOsPrintf("By Div64: %lld / %lld = %lld    remaind %lld\n", nDividendS64, nDivisorS64, nResultS64, nRemainderS64);
}

static void _TestCamOsSystemTime(void)
{
    int32_t nCnt = 0;
    CamOsTimespec_t tTs;
    struct tm * tTm;

    for(nCnt = 0; nCnt < 10; nCnt++)
    {
        CamOsGetTimeOfDay(&tTs);
        tTm = localtime ((time_t *)&tTs.nSec);
        CamOsPrintf("RawSecond: %d  ->  %d/%02d/%02d [%d]  %02d:%02d:%02d\n",
                    tTs.nSec,
                    tTm->tm_year+1900,
                    tTm->tm_mon+1,
                    tTm->tm_mday,
                    tTm->tm_wday,
                    tTm->tm_hour,
                    tTm->tm_min,
                    tTm->tm_sec);

        tTs.nSec += 90000;
        CamOsSetTimeOfDay(&tTs);

        CamOsMsSleep(3000);
    }

    for(nCnt = 0; nCnt < 10; nCnt++)
    {
        CamOsGetTimeOfDay(&tTs);
        tTm = localtime ((time_t *)&tTs.nSec);
        CamOsPrintf("RawSecond: %d  ->  %d/%02d/%02d [%d]  %02d:%02d:%02d\n",
                    tTs.nSec,
                    tTm->tm_year+1900,
                    tTm->tm_mon+1,
                    tTm->tm_mday,
                    tTm->tm_wday,
                    tTm->tm_hour,
                    tTm->tm_min,
                    tTm->tm_sec);

        tTs.nSec -= 90000;
        CamOsSetTimeOfDay(&tTs);

        CamOsMsSleep(3000);
    }
}

static void _TestCamOsPhysMemSize(void)
{
    CamOsMemSize_e eMemSize;
    eMemSize = CamOsPhysMemSize();
    CamOsPrintf("System has %dMB physical memory\n", 1<<(uint32_t)eMemSize);
}

static void _TestCamOsChipId(void)
{
    CamOsPrintf("Chip ID: 0x%X\n", CamOsChipId());
}

static int32_t _CamOsTcondTestEntry0(void *pUserData)
{
    CamOsTcond_t *pCond = (CamOsTcond_t *)pUserData;

    CamOsMsSleep(300);

    _gTestCnt++;//1

    CamOsPrintf("%s CamOsTcondSignal\n", __FUNCTION__);
    CamOsTcondSignal(pCond);

    CamOsMsSleep(100);

    CamOsPrintf("%s CamOsTcondSignal\n", __FUNCTION__);
    CamOsTcondSignal(pCond);

    CamOsMsSleep(100);

    _gTestCnt++;//3

    CamOsPrintf("%s CamOsTcondSignalAll\n", __FUNCTION__);
    CamOsTcondSignalAll(pCond);

    CamOsPrintf("%s CamOsTcondTimedWait start\n", __FUNCTION__);
    if (CamOsTcondTimedWait(pCond, 500) != CAM_OS_TIMEOUT)
    {
        CamOsPrintf("%s: step 1 fail!\n", __FUNCTION__);
        return -1;
    }
    CamOsPrintf("%s CamOsTcondTimedWait end(timeout)\n", __FUNCTION__);

    CamOsMsSleep(2000);

    _gTestCnt++;

    CamOsPrintf("%s CamOsTcondSignal\n", __FUNCTION__);
    CamOsTcondSignal(pCond);

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static int32_t _CamOsTcondTestEntry1(void *pUserData)
{
    CamOsTcond_t *pCond = (CamOsTcond_t *)pUserData;
    CamOsRet_e eRet;

    CamOsPrintf("%s CamOsTcondWait start\n", __FUNCTION__);
    CamOsTcondWait(pCond);
    CamOsPrintf("%s CamOsTcondWait end\n", __FUNCTION__);

    if (_gTestCnt != 1)
    {
        CamOsPrintf("%s: step 1 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    _gTestCnt++;//2

    CamOsPrintf("%s CamOsTcondWait start\n", __FUNCTION__);
    CamOsTcondWait(pCond);
    CamOsPrintf("%s CamOsTcondWait end\n", __FUNCTION__);

    if (_gTestCnt != 3)
    {
        CamOsPrintf("%s: step 2 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsPrintf("%s CamOsTcondTimedWait start\n", __FUNCTION__);
    eRet = CamOsTcondTimedWait(pCond, 1000);
    if (eRet != CAM_OS_TIMEOUT || _gTestCnt != 3)
    {
        CamOsPrintf("%s: step 3 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }
    CamOsPrintf("%s CamOsTcondTimedWait end(timeout)\n", __FUNCTION__);

    _gTestCnt++;

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static int32_t _CamOsTcondTestEntry2(void *pUserData)
{
    CamOsRet_e eRet;

    CamOsTcond_t *pCond = (CamOsTcond_t *)pUserData;

    CamOsMsSleep(10);

    CamOsPrintf("%s CamOsTcondWait start\n", __FUNCTION__);
    CamOsTcondWait(pCond);
    CamOsPrintf("%s CamOsTcondWait end\n", __FUNCTION__);

    if (_gTestCnt != 2)
    {
        CamOsPrintf("%s: step 1 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsPrintf("%s CamOsTcondWait start\n", __FUNCTION__);
    CamOsTcondWait(pCond);
    CamOsPrintf("%s CamOsTcondWait end\n", __FUNCTION__);

    if (_gTestCnt != 3)
    {
        CamOsPrintf("%s: step 2 fail!\n", __FUNCTION__);
        return -1;
    }

    CamOsPrintf("%s CamOsTcondTimedWait start\n", __FUNCTION__);
    eRet = CamOsTcondTimedWait(pCond, 5000);
    if (eRet != CAM_OS_OK || _gTestCnt != 5)
    {
        CamOsPrintf("%s: step 3 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }
    CamOsPrintf("%s CamOsTcondTimedWait end(wakend)\n", __FUNCTION__);

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static void _TestCamOsTcond(void)
{
    static CamOsThread TaskHandle0, TaskHandle1, TaskHandle2;
    CamOsTcond_t tCond;

    _gTestCnt = 0;
    CamOsTcondInit(&tCond);

    CamOsThreadCreate(&TaskHandle0, NULL, (void *)_CamOsTcondTestEntry0, &tCond);
    CamOsThreadCreate(&TaskHandle1, NULL, (void *)_CamOsTcondTestEntry1, &tCond);
    CamOsThreadCreate(&TaskHandle2, NULL, (void *)_CamOsTcondTestEntry2, &tCond);

    CamOsThreadJoin(TaskHandle0);
    CamOsThreadJoin(TaskHandle1);
    CamOsThreadJoin(TaskHandle2);

    CamOsTcondDeinit(&tCond);
}

static void _TestCamOsBitmap(void)
{
    #define BITMAP_BITS 128
    CAM_OS_DECLARE_BITMAP(aBitmap, BITMAP_BITS);

    CAM_OS_BITMAP_CLEAR(aBitmap);
    CamOsPrintf("Set bit 0, 1, 2, 37, 98\n");
    CAM_OS_SET_BIT(0, aBitmap);
    CAM_OS_SET_BIT(1, aBitmap);
    CAM_OS_SET_BIT(2, aBitmap);
    CAM_OS_SET_BIT(37, aBitmap);
    CAM_OS_SET_BIT(98, aBitmap);
    CamOsPrintf("\ttest bit 0:   %d\n", CAM_OS_TEST_BIT(0, aBitmap));
    CamOsPrintf("\ttest bit 1:   %d\n", CAM_OS_TEST_BIT(1, aBitmap));
    CamOsPrintf("\ttest bit 2:   %d\n", CAM_OS_TEST_BIT(2, aBitmap));
    CamOsPrintf("\ttest bit 3:   %d\n", CAM_OS_TEST_BIT(3, aBitmap));
    CamOsPrintf("\ttest bit 30:  %d\n", CAM_OS_TEST_BIT(30, aBitmap));
    CamOsPrintf("\ttest bit 37:  %d\n", CAM_OS_TEST_BIT(37, aBitmap));
    CamOsPrintf("\ttest bit 80:  %d\n", CAM_OS_TEST_BIT(80, aBitmap));
    CamOsPrintf("\ttest bit 98:  %d\n", CAM_OS_TEST_BIT(98, aBitmap));
    CamOsPrintf("\ttest bit 127: %d\n", CAM_OS_TEST_BIT(127, aBitmap));

    CamOsPrintf("\tfirst zero bit: %u\n", CAM_OS_FIND_FIRST_ZERO_BIT(aBitmap, BITMAP_BITS));
    CamOsPrintf("Clear bit 2, 98\n");
    CAM_OS_CLEAR_BIT(2, aBitmap);
    CAM_OS_CLEAR_BIT(98, aBitmap);
    CamOsPrintf("\ttest bit 2:   %d\n", CAM_OS_TEST_BIT(2, aBitmap));
    CamOsPrintf("\ttest bit 98:  %d\n", CAM_OS_TEST_BIT(98, aBitmap));
    CamOsPrintf("\tfirst zero bit: %u\n", CAM_OS_FIND_FIRST_ZERO_BIT(aBitmap, BITMAP_BITS));
}

struct HashTableElement_t
{
    struct CamOsHListNode_t tHentry;
    uint32_t nKey;
    uint32_t nData;
};

static void _TestCamOsHash(void)
{
    uint32_t nItemNum;
    CAM_OS_DEFINE_HASHTABLE(aHashTable, 8);
    struct HashTableElement_t tHListNode0, tHListNode1, tHListNode2, tHListNode3, tHListNode4, *ptHListNode;

    tHListNode0.nKey = 102;
    tHListNode0.nData = 1021;
    tHListNode1.nKey = 1872;
    tHListNode1.nData = 18721;
    tHListNode2.nKey = 102;
    tHListNode2.nData = 1022;
    tHListNode3.nKey = 1872;
    tHListNode3.nData = 18722;
    tHListNode4.nKey = 102;
    tHListNode4.nData = 1023;
    CamOsPrintf("Add 3 items with key 102 and 2 items with key 1872\r\n");
    CAM_OS_HASH_ADD(aHashTable, &tHListNode0.tHentry, tHListNode0.nKey);
    CAM_OS_HASH_ADD(aHashTable, &tHListNode1.tHentry, tHListNode1.nKey);
    CAM_OS_HASH_ADD(aHashTable, &tHListNode2.tHentry, tHListNode2.nKey);
    CAM_OS_HASH_ADD(aHashTable, &tHListNode3.tHentry, tHListNode3.nKey);
    CAM_OS_HASH_ADD(aHashTable, &tHListNode4.tHentry, tHListNode4.nKey);
    CamOsPrintf("Get items with key 102: \r\n");
    nItemNum = 0;
    CAM_OS_HASH_FOR_EACH_POSSIBLE(aHashTable, ptHListNode, tHentry, 102)
    {
        CamOsPrintf("\titem %u: data=%u\r\n", nItemNum, ptHListNode->nData);
        nItemNum++;
    }

    CamOsPrintf("Get items with key 1872: \r\n");
    nItemNum = 0;
    CAM_OS_HASH_FOR_EACH_POSSIBLE(aHashTable, ptHListNode, tHentry, 1872)
    {
        CamOsPrintf("\titem %u: data=%u\r\n", nItemNum, ptHListNode->nData);
        nItemNum++;
    }

    CamOsPrintf("Delete one items with key 1872.\r\n");
    CAM_OS_HASH_DEL(&tHListNode3.tHentry);
    CamOsPrintf("Get items with key 1872: \r\n");
    nItemNum = 0;
    CAM_OS_HASH_FOR_EACH_POSSIBLE(aHashTable, ptHListNode, tHentry, 1872)
    {
        CamOsPrintf("\titem %u: data=%u\n", nItemNum, ptHListNode->nData);
        nItemNum++;
    }
}

static void _TestCamOsIdr(void)
{
    CamOsIdr_t tIdr;
    uint32_t nIdrData1=11111, nIdrData2=22222, nIdrData3=33333, *pnIdrDataPtr;
    int32_t nIdrId1, nIdrId2, nIdrId3;

    if (CAM_OS_OK == CamOsIdrInit(&tIdr))
    {
        CamOsPrintf("Alloc data1(=%u) in 100~200\r\n", nIdrData1);
        nIdrId1 = CamOsIdrAlloc(&tIdr, (void *)&nIdrData1, 100, 200);
        CamOsPrintf("Alloc data2(=%u) in 100~200\r\n", nIdrData2);
        nIdrId2 = CamOsIdrAlloc(&tIdr, (void *)&nIdrData2, 100, 200);
        CamOsPrintf("Alloc data3(=%u) in 500~\r\n", nIdrData3);
        nIdrId3 = CamOsIdrAlloc(&tIdr, (void *)&nIdrData3, 500, 0);
        pnIdrDataPtr = (uint32_t*)CamOsIdrFind(&tIdr, nIdrId1);
        CamOsPrintf("ID1 = %d, find data = %u\r\n", nIdrId1, *pnIdrDataPtr);
        pnIdrDataPtr = (uint32_t*)CamOsIdrFind(&tIdr, nIdrId2);
        CamOsPrintf("ID2 = %d, find data = %u\r\n", nIdrId2, *pnIdrDataPtr);
        pnIdrDataPtr = (uint32_t*)CamOsIdrFind(&tIdr, nIdrId3);
        CamOsPrintf("ID3 = %d, find data = %u\r\n", nIdrId3, *pnIdrDataPtr);

        CamOsPrintf("Remove ID3(=%d) ... ", nIdrId3);
        CamOsIdrRemove(&tIdr, nIdrId3);
        if (NULL == CamOsIdrFind(&tIdr, nIdrId3))
            CamOsPrintf("success!\n");
        else
            CamOsPrintf("fail!\n");

        CamOsIdrDestroy(&tIdr);
    }
    else
        CamOsPrintf("CamOsIdrInit fail!\n");
}