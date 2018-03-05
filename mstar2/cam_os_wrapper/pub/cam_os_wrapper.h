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
/// @file      cam_os_wrapper.h
/// @brief     Cam OS Wrapper Header File for
///            1. RTK OS
///            2. Linux User Space
///            3. Linux Kernel Space
///////////////////////////////////////////////////////////////////////////////

#ifndef __CAM_OS_WRAPPER_H__
#define __CAM_OS_WRAPPER_H__

#define CAM_OS_WRAPPER_VERSION "v1.0.0"

#if defined(__aarch64__)
#define CAM_OS_BITS_PER_LONG 64
#else
#define CAM_OS_BITS_PER_LONG 32
#endif

#if !defined(__KERNEL__) && !defined(_LINUX_TYPES_H)
typedef unsigned char       bool;
#endif

#define CAM_OS_MAX_TIMEOUT ((long)(~0UL>>1))

typedef enum
{
    CAM_OS_OK               = 0,
    CAM_OS_FAIL             = -1,
    CAM_OS_PARAM_ERR        = -2,
    CAM_OS_ALLOCMEM_FAIL    = -3,
    CAM_OS_TIMEOUT          = -4,
    CAM_OS_RESOURCE_BUSY    = -5,
} CamOsRet_e;

typedef enum
{
    CAM_OS_MEM_1MB      = 0,
    CAM_OS_MEM_2MB      = 1,
    CAM_OS_MEM_4MB      = 2,
    CAM_OS_MEM_8MB      = 3,
    CAM_OS_MEM_16MB     = 4,
    CAM_OS_MEM_32MB     = 5,
    CAM_OS_MEM_64MB     = 6,
    CAM_OS_MEM_128MB    = 7,
    CAM_OS_MEM_256MB    = 8,
    CAM_OS_MEM_512MB    = 9,
    CAM_OS_MEM_1024MB   = 10,
    CAM_OS_MEM_UNKNOWN  = 99,
} CamOsMemSize_e;

typedef struct
{
    uint8_t nPriv[44];
} CamOsMutex_t;

typedef struct
{
    uint8_t nPriv[64];
} CamOsTsem_t;

typedef struct
{
    uint8_t nPriv[80];
} CamOsRwsem_t;

typedef struct
{
    uint8_t nPriv[80];
} CamOsTcond_t;

typedef struct
{
    uint32_t nSec;
    uint32_t nNanoSec;
} CamOsTimespec_t;

typedef struct
{
    uint32_t nPriority;      /* From 1(lowest) to 99(highest), use OS default priority if set 0 */
    uint32_t nStackSize;     /* If nStackSize is zero, use OS default value */
} CamOsThreadAttrb_t, *pCamOsThreadAttrb;

typedef struct
{
    uint8_t nPriv[32];
} CamOsTimer_t;

typedef struct
{
    uint8_t nPriv[8];
} CamOsMemCache_t;

typedef struct
{
    volatile int32_t nCounter;
} CamOsAtomic_t;

typedef struct
{
    uint8_t nPriv[80];
} CamOsIdr_t;

typedef void * CamOsThread;

//=============================================================================
// Description:
//      Get cam_os_wrapper version with C string format.
// Parameters:
//      N/A
// Return:
//      C string type of version information.
//=============================================================================
char *CamOsVersion(void);

//=============================================================================
// Description:
//      Writes the C string pointed by format to the standard output.
// Parameters:
//      [in]  szFmt: C string that contains the text to be written, it can
//                   optionally contain embedded format specifiers.
// Return:
//      N/A
//=============================================================================
void CamOsPrintf(const char *szFmt, ...);

//=============================================================================
// Description:
//      Reads data from stdin and stores them according to the parameter format
//      into the locations pointed by the additional arguments.
// Parameters:
//      [in]  szFmt: C string that contains the text to be parsing, it can
//                   optionally contain embedded format specifiers.
// Return:
//      The number of items of the argument list successfully filled.
//=============================================================================
int32_t CamOsScanf(const char *szFmt, ...);

//=============================================================================
// Description:
//      Returns the next character from the standard input.
// Parameters:
//      N/A
// Return:
//      the character read is returned.
//=============================================================================
int32_t CamOsGetChar(void);

//=============================================================================
// Description:
//      Reads data from stdin and stores them according to the parameter format
//      into the locations pointed by the additional arguments.
// Parameters:
//      [in]  szBuf: Pointer to a buffer where the resulting C-string is stored.
//                   The buffer should have a size of at least nSize characters.
//      [in]  nSize: Maximum number of bytes to be used in the buffer.
//                   The generated string has a length of at most nSize-1,
//                   leaving space for the additional terminating null character.
//      [in]  szFmt: C string that contains a format string, it can optionally
//                   contain embedded format specifiers.
// Return:
//      The number of items of the argument list successfully filled.
//=============================================================================
int32_t CamOsSnprintf(char *szBuf, uint32_t nSize, const char *szFmt, ...);

//=============================================================================
// Description:
//      Suspend execution for millisecond intervals.
// Parameters:
//      [in]  nMsec: Millisecond to suspend.
// Return:
//      N/A
//=============================================================================
void CamOsMsSleep(uint32_t nMsec);

//=============================================================================
// Description:
//      Get the number of seconds and nanoseconds since the Epoch.
// Parameters:
//      [out] ptRes: A pointer to a CamOsTimespec_t structure where
//                   CamOsGetTimeOfDay() can store the time.
// Return:
//      N/A
//=============================================================================
void CamOsGetTimeOfDay(CamOsTimespec_t *ptRes);

//=============================================================================
// Description:
//      Set the number of seconds and nanoseconds since the Epoch.
// Parameters:
//      [in] ptRes: A pointer to a CamOsTimespec_t structure.
// Return:
//      N/A
//=============================================================================
void CamOsSetTimeOfDay(const CamOsTimespec_t *ptRes);

//=============================================================================
// Description:
//      Gets the current time of the clock specified, and puts it into the
//      buffer pointed to by ptRes.
// Parameters:
//      [out] ptRes: A pointer to a CamOsTimespec_t structure where
//                   CamOsGetMonotonicTime() can store the time.
// Return:
//      N/A
//=============================================================================
void CamOsGetMonotonicTime(CamOsTimespec_t *ptRes);

//=============================================================================
// Description:
//      The CamOsThreadCreate() function is used to create a new thread/task,
//      with attributes specified by ptAttrb. If ptAttrb is NULL, the default
//      attributes are used.
// Parameters:
//      [out] ptThread: A successful call to CamOsThreadCreate() stores the handle
//                     of the new thread.
//      [in]  ptAttrb: Argument points to a CamOsThreadAttrb_t structure whose
//                     contents are used at thread creation time to determine
//                     thread priority, stack size and thread name. Thread
//                     priority range from 1(lowest) to 99(highest), use OS
//                     default priority if set 0.
//      [in]  pfnStartRoutine(): The new thread starts execution by invoking it.
//      [in]  pArg: It is passed as the sole argument of pfnStartRoutine().
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsThreadCreate(CamOsThread *ptThread,
                             CamOsThreadAttrb_t *ptAttrb,
                             void *(*pfnStartRoutine)(void *),
                             void *pArg);

// [FIX ME]:sched_setscheduler seems not to work after pthread run in Linux
//          kernel and user space
#if 0
//=============================================================================
// Description:
//      Change priority of a thread created by CamOsThreadCreate.
// Parameters:
//      [in]  pThread: Handle of target thread.
//      [in]  nPriority: New priority of target thread.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsThreadChangePriority(CamOsThread pThread, uint32_t nPriority);
#endif

//=============================================================================
// Description:
//      Schedule out a thread created by CamOsThreadCreate.
// Parameters:
//      [in]  bInterruptible: Setup if schedule method with timeout is
//                            interruptible. This parameter is only applicable
//                            to Linux kernel space.
//      [in]  nMsec: The value of delay for the timeout.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsThreadSchedule(bool bInterruptible, uint32_t nMsec);

//=============================================================================
// Description:
//      Wake up the thread specified by pThread to run.
// Parameters:
//      [in]  tThread: Handle of target thread.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsThreadWakeUp(CamOsThread tThread);

//=============================================================================
// Description:
//      Waits for the thread specified by tThread to terminate. If that thread
//      has already terminated, then CamOsThreadJoin() returns immediately. This
//      function is not applicable to Linux kernel space.
// Parameters:
//      [in]  tThread: Handle of target thread.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsThreadJoin(CamOsThread tThread);

//=============================================================================
// Description:
//      Stop a thread created by CamOsThreadCreate in Linux kernel space. This
//      function is not applicable to RTK and Linux user space.
// Parameters:
//      [in]  tThread: Handle of target thread.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsThreadStop(CamOsThread tThread);

//=============================================================================
// Description:
//      When someone calls CamOsThreadStop, it will be woken and this will
//      return true. You should then return from the thread. This function is
//      not applicable to RTK and Linux user space.
// Parameters:
//      N/A
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsThreadShouldStop(void);

//=============================================================================
// Description:
//      Set the name of a thread. The thread name is a meaningful C language
//      string, whose length is restricted to 16 characters, including the
//      terminating null byte ('\0').
// Parameters:
//      [in]  tThread: Handle of target thread.
//      [in]  szName: specifies the new name.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsThreadSetName(CamOsThread tThread, const char *szName);

//=============================================================================
// Description:
//      Get the name of a thread. The buffer specified by name should be at
//      least 16 characters in length.
// Parameters:
//      [in]  tThread: Handle of target thread.
//      [out] szName: Buffer used to return the thread name.
//      [in]  nLen: Specifies the number of bytes available in szName
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsThreadGetName(CamOsThread tThread, char *szName, uint32_t nLen);

//=============================================================================
// Description:
//      Get thread identification.
// Parameters:
//      N/A
// Return:
//      On success, returns the thread ID of the calling process.
//=============================================================================
uint32_t CamOsThreadGetID(void);

//=============================================================================
// Description:
//      Initializes the mutex.
// Parameters:
//      [in]  ptMutex: The mutex to initialize.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsMutexInit(CamOsMutex_t *ptMutex);

//=============================================================================
// Description:
//      Destroys the mutex.
// Parameters:
//      [in]  ptMutex: The mutex to destroy.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsMutexDestroy(CamOsMutex_t *ptMutex);

//=============================================================================
// Description:
//      Lock the mutex.
// Parameters:
//      [in]  ptMutex: The mutex to lock.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsMutexLock(CamOsMutex_t *ptMutex);

//=============================================================================
// Description:
//      Try lock the mutex, and return as non-blocking mode.
// Parameters:
//      [in]  ptMutex: The mutex to lock.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsMutexTryLock(CamOsMutex_t *ptMutex);

//=============================================================================
// Description:
//      Unlock the mutex.
// Parameters:
//      [in]  ptMutex: The mutex to unlock.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsMutexUnlock(CamOsMutex_t *ptMutex);

//=============================================================================
// Description:
//      Initializes the semaphore at a given value.
// Parameters:
//      [in]  ptTsem: The semaphore to initialize.
//      [in]  nVal: the initial value of the semaphore.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsTsemInit(CamOsTsem_t *ptTsem, uint32_t nVal);

//=============================================================================
// Description:
//      Destroy the semaphore.
// Parameters:
//      [in]  ptTsem: The semaphore to destroy.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsTsemDeinit(CamOsTsem_t *ptTsem);

//=============================================================================
// Description:
//      Increases the value of the semaphore.
// Parameters:
//      [in]  ptTsem: The semaphore to increase.
// Return:
//      N/A
//=============================================================================
void CamOsTsemUp(CamOsTsem_t *ptTsem);

//=============================================================================
// Description:
//      Decreases the value of the semaphore. Blocks if the semaphore value is
//      zero.
// Parameters:
//      [in]  ptTsem: The semaphore to decrease.
// Return:
//      N/A
//=============================================================================
void CamOsTsemDown(CamOsTsem_t *ptTsem);

//=============================================================================
// Description:
//      Decreases the value of the semaphore. Blocks if the semaphore value is
//      zero.
// Parameters:
//      [in]  ptTsem: The semaphore to decrease.
//      [in]  nMsec: The value of delay for the timeout.
// Return:
//      If the timeout is reached the function exits with error CAM_OS_TIMEOUT.
//      CAM_OS_OK is returned if down successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsTsemTimedDown(CamOsTsem_t *ptTsem, uint32_t nMsec);

//=============================================================================
// Description:
//      Always return as non-blocking mode. Decreases the value of the semaphore
//      if it is bigger than zero. If the semaphore value is less than or equal
//      to zero, return directly.
// Parameters:
//      [in]  ptTsem: The semaphore to decrease.
// Return:
//      CAM_OS_OK is returned if down successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsTsemTryDown(CamOsTsem_t *ptTsem);

//=============================================================================
// Description:
//      Initializes the rw semaphore.
// Parameters:
//      [in]  ptRwsem: The rw semaphore to initialize.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsRwsemInit(CamOsRwsem_t *ptRwsem);

//=============================================================================
// Description:
//      Destroys the read-write semaphore.
// Parameters:
//      [in]  ptRwsem: The rw semaphore to destroy.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsRwsemDeinit(CamOsRwsem_t *ptRwsem);

//=============================================================================
// Description:
//      Unlock the read semaphore.
// Parameters:
//      [in]  ptRwsem: The rw semaphore to unlock.
// Return:
//      N/A
//=============================================================================
void CamOsRwsemUpRead(CamOsRwsem_t *ptRwsem);

//=============================================================================
// Description:
//      Unlock the write semaphore.
// Parameters:
//      [in]  ptRwsem: The rw semaphore to unlock.
// Return:
//      N/A
//=============================================================================
void CamOsRwsemUpWrite(CamOsRwsem_t *ptRwsem);

//=============================================================================
// Description:
//      Lock the read semaphore.
// Parameters:
//      [in]  ptRwsem: The rw semaphore to lock.
// Return:
//      N/A
//=============================================================================
void CamOsRwsemDownRead(CamOsRwsem_t *ptRwsem);

//=============================================================================
// Description:
//      Lock the write semaphore.
// Parameters:
//      [in]  ptRwsem: The rw semaphore to lock.
// Return:
//      N/A
//=============================================================================
void CamOsRwsemDownWrite(CamOsRwsem_t *ptRwsem);

//=============================================================================
// Description:
//      Try lock the read semaphore, and return as non-blocking mode.
// Parameters:
//      [in]  ptRwsem: The rw semaphore to lock.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsRwsemTryDownRead(CamOsRwsem_t *ptRwsem);

//=============================================================================
// Description:
//      Try lock the write semaphore, and return as non-blocking mode.
// Parameters:
//      [in]  ptRwsem: The rw semaphore to lock.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsRwsemTryDownWrite(CamOsRwsem_t *ptRwsem);

//=============================================================================
// Description:
//      Initializes the condition.
// Parameters:
//      [in]  ptTcond: The condition to Initialize.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsTcondInit(CamOsTcond_t *ptTcond);

//=============================================================================
// Description:
//      Destroys the condition.
// Parameters:
//      [in]  ptTcond: The condition to destroy.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsTcondDeinit(CamOsTcond_t *ptTcond);

//=============================================================================
// Description:
//      Signal the condition, if anyone waiting.
// Parameters:
//      [in]  ptTcond: The condition to signal
// Return:
//      N/A
//=============================================================================
void CamOsTcondSignal(CamOsTcond_t *ptTcond);

//=============================================================================
// Description:
//      Signal the condition for all waitings totally.
// Parameters:
//      [in]  ptTcond: The condition to signal
// Return:
//      N/A
//=============================================================================
void CamOsTcondSignalAll(CamOsTcond_t *ptTcond);

//=============================================================================
// Description:
//      Wait on the condition.
// Parameters:
//      [in]  ptTcond: The condition to wait.
// Return:
//      N/A
//=============================================================================
void CamOsTcondWait(CamOsTcond_t *ptTcond);

//=============================================================================
// Description:
//      Wait on the condition.
// Parameters:
//      [in]  ptTcond: The condition to wait.
//      [in]  nMsec: The value of delay for the timeout.
// Return:
//      If the timeout is reached the function exits with error CAM_OS_TIMEOUT.
//      CAM_OS_OK is returned if wait successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsTcondTimedWait(CamOsTcond_t *ptTcond, uint32_t nMsec);

//=============================================================================
// Description:
//      Signal the condition, if anyone waiting(it's interruptible).
//      This API is the same with CamOsTcondSignal in RTK and Linux user space.
// Parameters:
//      [in]  ptTcond: The condition to signal
// Return:
//      N/A
//=============================================================================
void CamOsTcondSignalInterruptible(CamOsTcond_t *ptTcond);

//=============================================================================
// Description:
//      Signal the condition for all waitings totally(it's interruptible).
//      This API is the same with CamOsTcondSignal in RTK and Linux user space.
// Parameters:
//      [in]  ptTcond: The condition to signal
// Return:
//      N/A
//=============================================================================
void CamOsTcondSignalAllInterruptible(CamOsTcond_t *ptTcond);

//=============================================================================
// Description:
//      Wait on the condition(it's interruptible).
//      This API is the same with CamOsTcondSignal in RTK and Linux user space.
// Parameters:
//      [in]  ptTcond: The condition to wait.
// Return:
//      N/A
//=============================================================================
void CamOsTcondWaitInterruptible(CamOsTcond_t *ptTcond);

//=============================================================================
// Description:
//      Wait on the condition with time limitation(it's interruptible).
//      This API is the same with CamOsTcondSignal in RTK and Linux user space.
// Parameters:
//      [in]  ptTcond: The condition to wait.
//      [in]  nMsec: The value of delay for the timeout.
// Return:
//      If the timeout is reached the function exits with error CAM_OS_TIMEOUT.
//      CAM_OS_OK is returned if wait successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsTcondTimedWaitInterruptible(CamOsTcond_t *ptTcond, uint32_t nMsec);

//=============================================================================
// Description:
//      Check if any task wait for this condition.
//      This API is not supported to Linux user space.
// Parameters:
//      [in]  ptTcond: The condition to check.
// Return:
//      If any task waits for this condition, CAM_OS_OK is returned.
//      CAM_OS_FAIL is returned if no one waits; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsTcondWaitActive(CamOsTcond_t *ptTcond);

//=============================================================================
// Description:
//      Allocates a block of nSize bytes of memory, returning a pointer to the
//      beginning of the block.
// Parameters:
//      [in]  nSize: Size of the memory block, in bytes.
// Return:
//      On success, a pointer to the memory block allocated by the function. If
//      failed to allocate, a null pointer is returned.
//=============================================================================
void* CamOsMemAlloc(uint32_t nSize);

//=============================================================================
// Description:
//      Allocates a block of memory for an array of nNum elements, each of them
//      nSize bytes long, and initializes all its bits to zero.
// Parameters:
//      [in]  nNum: Number of elements to allocate.
//      [in]  nSize: Size of each element.
// Return:
//      On success, a pointer to the memory block allocated by the function. If
//      failed to allocate, a null pointer is returned.
//=============================================================================
void* CamOsMemCalloc(uint32_t nNum, uint32_t nSize);

//=============================================================================
// Description:
//      Changes the size of the memory block pointed to by pPtr. The function
//      may move the memory block to a new location (whose address is returned
//      by the function).
// Parameters:
//      [in]  pPtr: Pointer to a memory block previously allocated with
//                  CamOsMemAlloc, CamOsMemCalloc or CamOsMemRealloc.
//      [in]  nSize: New size for the memory block, in bytes.
// Return:
//      A pointer to the reallocated memory block, which may be either the same
//      as pPtr or a new location.
//=============================================================================
void* CamOsMemRealloc(void* pPtr, uint32_t nSize);

//=============================================================================
// Description:
//      A block of memory previously allocated by a call to CamOsMemAlloc,
//      CamOsMemCalloc or CamOsMemRealloc is deallocated, making it available
//      again for further allocations. If pPtr is a null pointer, the function
//      does nothing.
// Parameters:
//      [in]  pPtr: Pointer to a memory block previously allocated with
//                  CamOsMemAlloc, CamOsMemCalloc or CamOsMemRealloc.
// Return:
//      N/A
//=============================================================================
void CamOsMemRelease(void* pPtr);

//=============================================================================
// Description:
//      Allocates a block of nSize bytes of direct memory (non-cached memory),
//      returning three pointer for different address domain to the beginning
//      of the block.
// Parameters:
//      [in]  szName: Name of the memory block, whose length is restricted to
//                    16 characters.
//      [in]  nSize: Size of the memory block, in bytes.
//      [out] ppVirtPtr: Virtual address pointer to the memory block.
//      [out] ppPhysPtr: Physical address pointer to the memory block.
//      [out] ppMiuPtr: Memory Interface Unit address pointer to the memory block.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsDirectMemAlloc(const char* szName,
                               uint32_t nSize,
                               void** ppVirtPtr,
                               void** ppPhysPtr,
                               void** ppMiuPtr);

//=============================================================================
// Description:
//      A block of memory previously allocated by a call to CamOsDirectMemAlloc,
//      is deallocated, making it available again for further allocations.
// Parameters:
//      [in]  pVirtPtr: Virtual address pointer to a memory block previously
//                      allocated with CamOsDirectMemAlloc.
//      [in]  nSize: Size of the memory block, in bytes.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsDirectMemRelease(void* pVirtPtr, uint32_t nSize);

//=============================================================================
// Description:
//      Flush chche of a block of memory previously allocated by a call to
//      CamOsDirectMemAlloc.
// Parameters:
//      [in]  pVirtPtr: Virtual address pointer to a memory block previously
//                      allocated with CamOsDirectMemAlloc.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsDirectMemFlush(void* pVirtPtr);

//=============================================================================
// Description:
//      Print all allocated direct memory information to the standard output.
// Parameters:
//      N/A
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsDirectMemStat(void);

//=============================================================================
// Description:
//      Transfer Physical address to MIU address.
// Parameters:
//      [in]  pPtr: Physical address.
// Return:
//      MIU address.
//=============================================================================
void* CamOsDirectMemPhysToMiu(void* pPtr);

//=============================================================================
// Description:
//      Transfer MIU address to Physical address.
// Parameters:
//      [in]  pPtr: MIU address.
// Return:
//      Physical address.
//=============================================================================
void* CamOsDirectMemMiuToPhys(void* pPtr);

//=============================================================================
// Description:
//      Transfer Physical address to Virtual address.
// Parameters:
//      [in]  pPtr: Physical address.
// Return:
//      Virtual address.
//=============================================================================
void* CamOsDirectMemPhysToVirt(void* pPtr);

//=============================================================================
// Description:
//      Transfer Virtual address to Physical address.
// Parameters:
//      [in]  pPtr: Virtual address.
// Return:
//      Physical address.
//=============================================================================
void* CamOsDirectMemVirtToPhys(void* pPtr);

//=============================================================================
// Description:
//      Map Physical address to Virtual address.
// Parameters:
//      [in]  pPhyPtr: Physical address.
//      [in]  nSize: Size of the memory block, in bytes.
//      [in]  bNonCache: Map to cache or non-cache area.
// Return:
//      Virtual address.
//=============================================================================
void* CamOsPhyMemMap(void* pPhyPtr, uint32_t nSize, bool bNonCache);

//=============================================================================
// Description:
//      Unmap Virtual address that was mapped by CamOsPhyMemMap.
// Parameters:
//      [in]  pVirtPtr: Virtual address.
// Return:
//      N/A
//=============================================================================
void CamOsPhyMemUnMap(void* pVirtPtr);

//=============================================================================
// Description:
//      Create a memory cache(memory pool) and allocate with specified size
//      to ignore internal fragmentation.
// Parameters:
//      [out] ptMemCache: Get memory cache information if create successfully.
//      [in]  szName: A string which is used in /proc/slabinfo to identify
//                    this cache(It's significant only in linux kernel).
//      [in]  nSize: Object size in this cache.
//      [in]  bHwCacheAlign: Align objs on cache lines(Only for Linux)
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e  CamOsMemCacheCreate(CamOsMemCache_t *ptMemCache, char *szName, uint32_t nSize, bool bHwCacheAlign);

//=============================================================================
// Description:
//      Destroy the memory cache.
// Parameters:
//      [in]  ptMemCache: The cache to destroy.
// Return:
//      N/A
//=============================================================================
void CamOsMemCacheDestroy(CamOsMemCache_t *ptMemCache);

//=============================================================================
// Description:
//      Allocate a memory block(object) from this memory cache.
// Parameters:
//      [in]  ptMemCache: The cache to be allocated.
// Return:
//      On success, a pointer to the memory block allocated by the function. If
//      failed to allocate, a null pointer is returned.
//=============================================================================
void *CamOsMemCacheAlloc(CamOsMemCache_t *ptMemCache);

//=============================================================================
// Description:
//      Release a memory block(object) to this memory cache.
// Parameters:
//      [in]  ptMemCache: The cache to be released to.
//      [in]  pObjPtr: Pointer to a memory block(object) previously allocated by
//                     CamOsMemCacheAlloc.
// Return:
//      N/A
//=============================================================================
void CamOsMemCacheFree(CamOsMemCache_t *ptMemCache, void *pObjPtr);

//=============================================================================
// Description:
//      Set property value by property name.
// Parameters:
//      [in]  szKey: Name of property.
//      [in]  szValue: Value if property.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsPropertySet(const char *szKey, const char *szValue);

//=============================================================================
// Description:
//      Get property value by property name.
// Parameters:
//      [in]  szKey: Name of property.
//      [out] szValue: Value if property.
//      [in]  szDefaultValue: If the property read fails or returns an empty
//                            value, the default value is used
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsPropertyGet(const char *szkey, char *szValue, const char *szDefaultValue);

//=============================================================================
// Description:
//      Unsigned 64bit divide with Unsigned 64bit divisor with remainder.
// Parameters:
//      [in]  nDividend: Dividend.
//      [in]  nDivisor: Divisor.
//      [out]  pRemainder: Pointer to the remainder.
// Return:
//      Quotient of division.
//=============================================================================
uint64_t CamOsMathDivU64(uint64_t nDividend, uint64_t nDivisor, uint64_t *pRemainder);

//=============================================================================
// Description:
//      Signed 64bit divide with signed 64bit divisor with remainder.
// Parameters:
//      [in]  nDividend: Dividend.
//      [in]  nDivisor: Divisor.
//      [out]  pRemainder: Pointer to the remainder.
// Return:
//      Quotient of division.
//=============================================================================
int64_t CamOsMathDivS64(int64_t nDividend, int64_t nDivisor, int64_t *pRemainder);

//=============================================================================
// Description:
//      Copy a block of data from user space in Linux kernel space, it just
//      memory copy in RTOS.
// Parameters:
//      [in]  pTo: Destination address, in kernel space.
//      [in]  pFrom: Source address, in user space.
//      [in]  nLen: Number of bytes to copy.
// Return:
//      Number of bytes that could not be copied. On success, this will be zero.
//=============================================================================
uint32_t CamOsCopyFromUpperLayer(void *pTo, const void *pFrom, uint32_t nLen);

//=============================================================================
// Description:
//      Copy a block of data into user space in Linux kernel space, it just
//      memory copy in RTOS.
// Parameters:
//      [in]  pTo: Destination address, in user space.
//      [in]  pFrom: Source address, in kernel space.
//      [in]  nLen: Number of bytes to copy.
// Return:
//      Number of bytes that could not be copied. On success, this will be zero.
//=============================================================================
uint32_t CamOsCopyToUpperLayer(void *pTo, const void * pFrom, uint32_t nLen);

//=============================================================================
// Description:
//      Init timer.
// Parameters:
//      [in]  ptTimer: Pointer of type CamOsTimer_t.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsTimerInit(CamOsTimer_t *ptTimer);

//=============================================================================
// Description:
//      Delete timer.
// Parameters:
//      [in]  ptTimer: Pointer of type CamOsTimer_t.
// Return:
//      0 is returned if timer has expired; otherwise, returns 1.
//=============================================================================
uint32_t CamOsTimerDelete(CamOsTimer_t *ptTimer);

//=============================================================================
// Description:
//      Start timer.
// Parameters:
//      [in]  ptTimer: Pointer of type CamOsTimer_t.
//      [in]  nMsec: The value of timer for the timeout.
//      [in]  pDataPtr: Pointer of user data for callback function.
//      [in]  pfnFunc: Pointer of callback function.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsTimerAdd(CamOsTimer_t *ptTimer, uint32_t nMsec, void *pDataPtr, void (*pfnFunc)(unsigned long nDataAddr));

//=============================================================================
// Description:
//      Restart timer that has been added with new timeout value.
// Parameters:
//      [in]  ptTimer: Pointer of type CamOsTimer_t.
//      [in]  nMsec: The value of timer for the timeout.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsTimerModify(CamOsTimer_t *ptTimer, uint32_t nMsec);

//=============================================================================
// Description:
//      Read atomic variable.
// Parameters:
//      [in]  ptAtomic: Pointer of type CamOsAtomic_t.
// Return:
//      The value of ptAtomic.
//=============================================================================
int32_t CamOsAtomicRead(CamOsAtomic_t *ptAtomic);

//=============================================================================
// Description:
//      Set atomic variable.
// Parameters:
//      [in]  ptAtomic: Pointer of type CamOsAtomic_t.
//      [in]  nValue: Required value.
// Return:
//      N/A
//=============================================================================
void CamOsAtomicSet(CamOsAtomic_t *ptAtomic, int32_t nValue);

//=============================================================================
// Description:
//      Add to the atomic variable and return value.
// Parameters:
//      [in]  nValue: Integer value to add.
//      [in]  ptAtomic: Pointer of type CamOsAtomic_t.
// Return:
//      The value of ptAtomic.
//=============================================================================
int32_t CamOsAtomicAddReturn(int32_t nValue, CamOsAtomic_t *ptAtomic);

//=============================================================================
// Description:
//      Subtract the atomic variable and return value.
// Parameters:
//      [in]  nValue: Integer value to subtract.
//      [in]  ptAtomic: Pointer of type CamOsAtomic_t.
// Return:
//      The value of ptAtomic.
//=============================================================================
int32_t CamOsAtomicSubReturn(int32_t nValue, CamOsAtomic_t *ptAtomic);

//=============================================================================
// Description:
//      Subtract value from variable and test result.
// Parameters:
//      [in]  nValue: Integer value to subtract.
//      [in]  ptAtomic: Pointer of type CamOsAtomic_t.
// Return:
//      Returns true if the result is zero, or false for all other cases.
//=============================================================================
int32_t CamOsAtomicSubAndTest(int32_t nValue, CamOsAtomic_t *ptAtomic);

//=============================================================================
// Description:
//      Increment atomic variable and return value.
// Parameters:
//      [in]  ptAtomic: Pointer of type CamOsAtomic_t.
// Return:
//      The value of ptAtomic.
//=============================================================================
int32_t CamOsAtomicIncReturn(CamOsAtomic_t *ptAtomic);

//=============================================================================
// Description:
//      decrement atomic variable and return value.
// Parameters:
//      [in]  ptAtomic: Pointer of type CamOsAtomic_t.
// Return:
//      The value of ptAtomic.
//=============================================================================
int32_t CamOsAtomicDecReturn(CamOsAtomic_t *ptAtomic);

//=============================================================================
// Description:
//      Increment and test result.
// Parameters:
//      [in]  ptAtomic: Pointer of type CamOsAtomic_t.
// Return:
//      Returns true if the result is zero, or false for all other cases.
//=============================================================================
int32_t CamOsAtomicIncAndTest(CamOsAtomic_t *ptAtomic);

//=============================================================================
// Description:
//      Decrement and test result.
// Parameters:
//      [in]  ptAtomic: Pointer of type CamOsAtomic_t.
// Return:
//      Returns true if the result is zero, or false for all other cases.
//=============================================================================
int32_t CamOsAtomicDecAndTest(CamOsAtomic_t *ptAtomic);

//=============================================================================
// Description:
//      Add to the atomic variable and test if negative.
// Parameters:
//      [in]  nValue: Integer value to subtract.
//      [in]  ptAtomic: Pointer of type CamOsAtomic_t.
// Return:
//      Returns true if the result is negative, or false when result is greater
//      than or equal to zero.
//=============================================================================
int32_t CamOsAtomicAddNegative(int32_t nValue, CamOsAtomic_t *ptAtomic);

//=============================================================================
// Description:
//      Init IDR data structure.
// Parameters:
//      [in]  ptIdr: Pointer of type CamOsIdr_t.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsIdrInit(CamOsIdr_t *ptIdr);

//=============================================================================
// Description:
//      Destroy the IDR data structure.
// Parameters:
//      [in] ptIdr: Pointer of type CamOsIdr_t.
// Return:
//      N/A
//=============================================================================
void CamOsIdrDestroy(CamOsIdr_t *ptIdr);

//=============================================================================
// Description:
//      Increment atomic variable and return value.
// Parameters:
//      [in]  ptIdr: Pointer of type CamOsIdr_t.
//      [in]  pPtr: Pointer of the data to store in IDR structure.
//      [in]  nStart: Start number of requested ID range.
//      [in]  nEnd: End number of requested ID range.
// Return:
//      The allocated ID number. If allocation fail, negative integer will
//      be returned.
//=============================================================================
int32_t CamOsIdrAlloc(CamOsIdr_t *ptIdr, void *pPtr, int32_t nStart, int32_t nEnd);

//=============================================================================
// Description:
//      Remove data from the IDR structure by ID.
// Parameters:
//      [in] ptIdr: Pointer of type CamOsIdr_t.
//      [in] nId: Data ID number.
// Return:
//      N/A
//=============================================================================
void CamOsIdrRemove(CamOsIdr_t *ptIdr, int32_t nId);

//=============================================================================
// Description:
//      Find data from the IDR structure by ID.
// Parameters:
//      [in] ptIdr: Pointer of type CamOsIdr_t.
//      [in] nId: Data ID number.
// Return:
//      On success, a pointer to the data stored in IDR structure. If
//      failed to find, a null pointer is returned.
//=============================================================================
void *CamOsIdrFind(CamOsIdr_t *ptIdr, int32_t nId);

//=============================================================================
// Description:
//      Get physical memory size of system.
// Parameters:
//      N/A
// Return:
//      Enumeration of memory size.
//=============================================================================
CamOsMemSize_e CamOsPhysMemSize(void);

//=============================================================================
// Description:
//      Get Chip ID.
// Parameters:
//      N/A
// Return:
//      Chip ID.
//=============================================================================
uint32_t CamOsChipId(void);

//=============================================================================
// Description:
//      Check if current function runs in ISR.
// Parameters:
//      N/A
// Return:
//      CAM_OS_OK is returned if in ISR; otherwise, returns CAM_OS_FAIL.
//=============================================================================
CamOsRet_e CamOsInInterrupt(void);

//=============================================================================
// Description:
//      Symmetric multiprocessing memory barrier.
// Parameters:
//      N/A
// Return:
//      N/A.
//=============================================================================
void CamOsSmpMemoryBarrier(void);

//=============================================================================
// Description:
//      Return string describing error number.
// Parameters:
//      [in] nErrNo: Error number to be converted.
// Return:
//      Character pointer to string of description.
//=============================================================================
char *CamOsStrError(int32_t nErrNo);

//=============================================================================
// Description:
//      Put system into panic.
// Parameters:
//      [in] szMessage: message to output in console.
// Return:
//      N/A
//=============================================================================
void CamOsPanic(const char *szMessage);

//=============================================================================
// Description:
//      Convert string to long integer with specific base.
// Parameters:
//      [in] szStr: String beginning with the representation of
//                  an integral number.
//      [in] szEndptr: Reference to an object of type char*, whose value
//                     is set by the function to the next character in szStr
//                     after the numerical value.
//                     This parameter can also be a null pointer, in which
//                     case it is not used.
//      [in] nBase: Numerical base (radix) that determines the valid characters
//                  and their interpretation. If this is 0, the base used is
//                  determined by the format in the sequence (see above).
// Return:
//      Converted long integer.
//=============================================================================
long CamOsStrtol(const char *szStr, char** szEndptr, int32_t nBase);

//=============================================================================
// Description:
//      Convert string to unsigned long integer with specific base.
// Parameters:
//      [in] szStr: String beginning with the representation of
//                  an integral number.
//      [in] szEndptr: Reference to an object of type char*, whose value
//                     is set by the function to the next character in szStr
//                     after the numerical value.
//                     This parameter can also be a null pointer, in which
//                     case it is not used.
//      [in] nBase: Numerical base (radix) that determines the valid characters
//                  and their interpretation. If this is 0, the base used is
//                  determined by the format in the sequence (see above).
// Return:
//      Converted long integer.
//=============================================================================
unsigned long CamOsStrtoul(const char *szStr, char** szEndptr, int32_t nBase);

//=============================================================================
// Description:
//      Convert string to unsigned long long integer with specific base.
// Parameters:
//      [in] szStr: String beginning with the representation of
//                  an integral number.
//      [in] szEndptr: Reference to an object of type char*, whose value
//                     is set by the function to the next character in szStr
//                     after the numerical value.
//                     This parameter can also be a null pointer, in which
//                     case it is not used.
//      [in] nBase: Numerical base (radix) that determines the valid characters
//                  and their interpretation. If this is 0, the base used is
//                  determined by the format in the sequence (see above).
// Return:
//      Converted long integer.
//=============================================================================
unsigned long long CamOsStrtoull(const char *szStr, char** szEndptr, int32_t nBase);

#endif /* __CAM_OS_WRAPPER_H__ */
