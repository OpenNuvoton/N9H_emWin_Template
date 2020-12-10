/**************************************************************************//**
* @copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*   1. Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright notice,
*      this list of conditions and the following disclaimer in the documentation
*      and/or other materials provided with the distribution.
*   3. Neither the name of Nuvoton Technology Corp. nor the names of its contributors
*      may be used to endorse or promote products derived from this software
*      without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

/* Standard includes. */
#include <stdlib.h>

/* Scheduler includes. */
/* FreeRTOS+POSIX. */
#include "FreeRTOS_POSIX.h"
#include "FreeRTOS_POSIX/pthread.h"
#include "FreeRTOS_POSIX/mqueue.h"
#include "FreeRTOS_POSIX/time.h"
#include "FreeRTOS_POSIX/errno.h"
#include "FreeRTOS_POSIX/unistd.h"

/* Nuvoton includes. */
#include "wblib.h"
#include "N9H26.h"

#include "LCDConf.h"

extern void *worker_touching(void *pvArgs);

//#define STORAGE_SD

#ifdef STORAGE_SD

#define ENABLE_SD_ONE_PART
#define ENABLE_SD_0
//#define ENABLE_SD_1
//#define ENABLE_SD_2
//#define ENABLE_SDIO_1

#define DEF_DISK_VOLUME_STR_SIZE    20
static char s_szDiskVolume[DEF_DISK_VOLUME_STR_SIZE] = {0x00};

int InitStorage_SD(void)
{
    uint32_t u32ExtFreq;
    uint32_t u32PllOutHz;
    uint32_t u32BlockSize, u32FreeSize, u32DiskSize;

    u32ExtFreq = sysGetExternalClock();
    u32PllOutHz = sysGetPLLOutputHz(eSYS_UPLL, u32ExtFreq);
    memset(s_szDiskVolume, 0x00, DEF_DISK_VOLUME_STR_SIZE);

#if defined(ENABLE_SD_ONE_PART)
    sicIoctl(SIC_SET_CLOCK, u32PllOutHz / 1000, 0, 0);
#if defined(ENABLE_SDIO_1)
    sdioOpen();
    if (sdioSdOpen1() <= 0)         //cause crash on Doorbell board SDIO slot. conflict with sensor pin
    {
        printf("Error in initializing SD card !! \n");
        s_bFileSystemInitDone = TRUE;
        goto exit_InitStorage_SD;
    }
#elif defined(ENABLE_SD_1)
    sicOpen();
    if (sicSdOpen1() <= 0)
    {
        printf("Error in initializing SD card !! \n");
        s_bFileSystemInitDone = TRUE;
        goto exit_InitStorage_SD;
    }
#elif defined(ENABLE_SD_2)
    sicOpen();
    if (sicSdOpen2() <= 0)
    {
        printf("Error in initializing SD card !! \n");
        s_bFileSystemInitDone = TRUE;
        goto exit_InitStorage_SD;
    }
#elif defined(ENABLE_SD_0)
    sicOpen();
    if (sicSdOpen0() <= 0)
    {
        printf("Error in initializing SD card !! \n");
        goto exit_InitStorage_SD;
    }
#endif
#endif

    sprintf(s_szDiskVolume, "C");

#if defined(ENABLE_SD_ONE_PART)
    fsAssignDriveNumber(s_szDiskVolume[0], DISK_TYPE_SD_MMC, 0, 1);
#endif

    if (fsDiskFreeSpace(s_szDiskVolume[0], &u32BlockSize, &u32FreeSize, &u32DiskSize) != FS_OK)
    {
        printf("Error in freespace!! \n");
        goto exit_InitStorage_SD;
    }

    printf("SD card block_size = %d\n", u32BlockSize);
    printf("SD card free_size = %dkB\n", u32FreeSize);
    printf("SD card disk_size = %dkB\n", u32DiskSize);

    return 0;

exit_InitStorage_SD:
    return -1;

}
#else
#define NAND_2      1   // comment to use 1 disk foor NAND, uncomment to use 2 disk

static NDISK_T ptNDisk;
static NDRV_T _nandDiskDriver0 =
{
    nandInit0,
    nandpread0,
    nandpwrite0,
    nand_is_page_dirty0,
    nand_is_valid_block0,
    nand_ioctl,
    nand_block_erase0,
    nand_chip_erase0,
    0
};

#define NAND1_1_SIZE     32 /* MB unit */

int InitStorage_NAND(void)
{

    UINT32 block_size, free_size, disk_size;
    UINT32 u32TotalSize;

    fsAssignDriveNumber('C', DISK_TYPE_SMART_MEDIA, 0, 1);
#ifdef NAND_2
    fsAssignDriveNumber('D', DISK_TYPE_SMART_MEDIA, 0, 2);
#endif

    sicOpen();

    /* Initialize GNAND */
    if (GNAND_InitNAND(&_nandDiskDriver0, &ptNDisk, TRUE) < 0)
    {
        printf("GNAND_InitNAND error\n");
        goto exit_InitStorage_NAND;
    }

    if (GNAND_MountNandDisk(&ptNDisk) < 0)
    {
        printf("GNAND_MountNandDisk error\n");
        goto exit_InitStorage_NAND;
    }

    /* Get NAND disk information*/
    u32TotalSize = ptNDisk.nZone * ptNDisk.nLBPerZone * ptNDisk.nPagePerBlock * ptNDisk.nPageSize;
    printf("Total Disk Size %d\n", u32TotalSize);
    /* Format NAND if necessery */
#ifdef NAND_2
    if ((fsDiskFreeSpace('C', &block_size, &free_size, &disk_size) < 0) ||
            (fsDiskFreeSpace('D', &block_size, &free_size, &disk_size) < 0))
    {
        printf("unknow disk type, format device .....\n");
        if (fsTwoPartAndFormatAll((PDISK_T *)ptNDisk.pDisk, NAND1_1_SIZE * 1024, (u32TotalSize - NAND1_1_SIZE * 1024)) < 0)
        {
            printf("Format failed\n");
            goto exit_InitStorage_NAND;
        }
        fsSetVolumeLabel('C', "NAND1-1\n", strlen("NAND1-1"));
        fsSetVolumeLabel('D', "NAND1-2\n", strlen("NAND1-2"));
    }
#endif

    return 0;

exit_InitStorage_NAND:
    return -1;
}
#endif

static void device_init(void)
{
    WB_UART_T uart;
    UINT32 u32ExtFreq;

    u32ExtFreq = sysGetExternalClock();
    sysUartPort(1);
    uart.uart_no = WB_UART_1;
    uart.uiFreq = u32ExtFreq;   //use APB clock
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    sysInitializeUART(&uart);

    sysEnableCache(CACHE_WRITE_BACK);

    printf("Display_Init start.\n");
    Display_Init();

// Filesystem initilization
    printf("fsInitFileSystem.\n");
    fsInitFileSystem();

#ifdef STORAGE_SD
    InitStorage_SD();
#else
    InitStorage_NAND();
#endif
}

static void device_fini(void)
{
    fsCloseFileSystem();
// Filesystem finalization
#ifdef STORAGE_SD

#else
    GNAND_UnMountNandDisk(&ptNDisk);
    sicClose();
#endif
}

void *worker_guiman(void *pvArgs);

void dump_clocks(void)
{
    printf("**** sysGetSystemClock = %d KHz\n", sysGetSystemClock() / 1000);
    printf("**** sysGetCPUClock = %d KHz\n", sysGetCPUClock() / 1000);
    printf("**** sysGetHCLK1Clock = %d KHz\n", sysGetHCLK1Clock() / 1000);
    printf("**** sysGetAPBClock = %d KHz\n", sysGetAPBClock() / 1000);
    printf("**** sysGetDramClock = %d KHz\n", sysGetDramClock() / 1000);
}

int main(void)
{
    pthread_t pxID_worker;
    pthread_attr_t attr;

    device_init();

    dump_clocks();

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 32 * 1024);

#if GUI_SUPPORT_TOUCH
    pthread_create(&pxID_worker, &attr, worker_touching, NULL);
#else
    pthread_create(&pxID_worker, &attr, worker_guiman, NULL);
#endif

    vTaskStartScheduler();
    for (;;);

    device_fini();
    return 0;
}

/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    (void) pcTaskName;
    (void) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    printf("MMMMMMMMMMM vApplicationStackOverflowHook %s MMMMMMMMMMMMM\n", pcTaskName);

    taskDISABLE_INTERRUPTS();
    for (;;);
}

void vApplicationMallocFailedHook(void)
{
    printf("MMMMMMMMMMM vApplicationMallocFailedHook MMMMMMMMMMMMM\n");
}

// We need this when configSUPPORT_STATIC_ALLOCATION is enabled
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{

// This is the static memory (TCB and stack) for the idle task
    static StaticTask_t xIdleTaskTCB; // __attribute__ ((section (".rtos_heap")));
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE]; // __attribute__ ((section (".rtos_heap"))) __attribute__((aligned (8)));


    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
    /* The buffers used by the Timer/Daemon task must be static so they are
    persistent, and so exist after this function returns. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* configUSE_STATIC_ALLOCATION is set to 1, so the application has the
    opportunity to supply the buffers that will be used by the Timer/RTOS daemon
    task as its stack and to hold its TCB.  If these are set to NULL then the
    buffers will be allocated dynamically, just as if xTaskCreate() had been
    called. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH; /* In words.  NOT in bytes! */
}

/*************************** End of file ****************************/
