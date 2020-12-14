/**************************************************************************//**
 * @file     main.c
 * @brief    To utilize emWin library to demonstrate interactive feature.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include "N9H20.h"

#include "GUI.h"
#include "LCDConf.h"

#include "WM.h"
#include "TEXT.h"
#include "FRAMEWIN.h"

#include "N9H20TouchPanel.h"

#include "NVT_Config.h"

/*********************************************************************
Constraints on LCD panel N9H20_VPOST_FW050TFT_800x480.lib:

1. Resolution is 800x480. (N9H20K1 has no such target because of the DRAM size limitation)
2. VPOST OSD function must disable. (VPOST H/W limitation)
3. PLL clock is 192 MHz, the pixel clock is 192/6 = 32 MHz.
4. 32,000,000/1056/525 = 57.7 FPS (frame per second)
5. If the bus bandwidth condition is too busy it may causes blinking result.
*/

#ifdef __ICCARM__
#pragma data_alignment = 32
UINT8 u8FrameBuf[XSIZE_PHYS*YSIZE_PHYS*2*2];
#else
UINT8 u8FrameBuf[XSIZE_PHYS*YSIZE_PHYS*2*2] __attribute__((aligned(32)));
#endif

UINT8 *u8FrameBufPtr;
UINT8 *u8FrameBuf2Ptr;

extern volatile int OS_TimeMS;
volatile int g_enable_Touch;

extern int ts_writefile(int hFile);
extern int ts_readfile(int hFile);
extern int ts_calibrate(int xsize, int ysize);
extern void TouchTask(void);

#ifndef STORAGE_SD
#define NAND_2      1   // comment to use 1 disk foor NAND, uncomment to use 2 disk
UINT32 StorageForNAND(void);

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

UINT32 StorageForNAND(void)
{

    UINT32 block_size, free_size, disk_size;
    UINT32 u32TotalSize;

    fsAssignDriveNumber('C', DISK_TYPE_SMART_MEDIA, 0, 1);
#ifdef NAND_2
    fsAssignDriveNumber('D', DISK_TYPE_SMART_MEDIA, 0, 2);
#endif

    sicOpen();

    /* Initialize GNAND */
    if(GNAND_InitNAND(&_nandDiskDriver0, &ptNDisk, TRUE) < 0)
    {
        sysprintf("GNAND_InitNAND error\n");
        goto halt;
    }

    if(GNAND_MountNandDisk(&ptNDisk) < 0)
    {
        sysprintf("GNAND_MountNandDisk error\n");
        goto halt;
    }

    /* Get NAND disk information*/
    u32TotalSize = ptNDisk.nZone* ptNDisk.nLBPerZone*ptNDisk.nPagePerBlock*ptNDisk.nPageSize;
    sysprintf("Total Disk Size %d\n", u32TotalSize);
    /* Format NAND if necessery */
#ifdef NAND_2
    if ((fsDiskFreeSpace('C', &block_size, &free_size, &disk_size) < 0) ||
            (fsDiskFreeSpace('D', &block_size, &free_size, &disk_size) < 0))
    {
        sysprintf("unknow disk type, format device .....\n");
        if (fsTwoPartAndFormatAll((PDISK_T *)ptNDisk.pDisk, NAND1_1_SIZE*1024, (u32TotalSize- NAND1_1_SIZE*1024)) < 0)
        {
            sysprintf("Format failed\n");
            goto halt;
        }
        fsSetVolumeLabel('C', "NAND1-1\n", strlen("NAND1-1"));
        fsSetVolumeLabel('D', "NAND1-2\n", strlen("NAND1-2"));
    }
#endif

halt:
    sysprintf("systen exit\n");
    return 0;

}
#endif

/*********************************************************************
*
*       TMR0_IRQHandler
*/
void TMR0_IRQHandler(void)
{
    OS_TimeMS++;
}

/*********************************************************************
*
*       TMR0_IRQHandler_TouchTask
*/
void TMR0_IRQHandler_TouchTask(void)
{
    if ( g_enable_Touch == 1 )
    {
        TouchTask();
    }
}

/*********************************************************************
*
*       _SYS_Init
*/
static void _SYS_Init(void)
{
    WB_UART_T uart;
    UINT32 u32ExtFreq;
    LCDFORMATEX lcdFormat;

    u32ExtFreq = sysGetExternalClock()*1000;
    sysUartPort(1);
    uart.uart_no = WB_UART_1;
    uart.uiFreq = u32ExtFreq;   //use APB clock
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    sysInitializeUART(&uart);

    sysInvalidCache();
    sysEnableCache(CACHE_WRITE_BACK);
    sysFlushCache(I_D_CACHE);

    /* start timer 0 */
    OS_TimeMS = 0;

    sysSetTimerReferenceClock(TIMER0, u32ExtFreq);  //External Crystal
    sysStartTimer(TIMER0, 1000, PERIODIC_MODE);     /* 1000 ticks/per sec ==> 1tick/1ms */
    sysSetTimerEvent(TIMER0,  1, (PVOID)TMR0_IRQHandler);           /* 1  tick   call back */
    sysSetTimerEvent(TIMER0, 20, (PVOID)TMR0_IRQHandler_TouchTask); /* 20 ticks  call back */

    sysprintf("fsInitFileSystem.\n");
    fsInitFileSystem();

#ifdef STORAGE_SD
    /*-----------------------------------------------------------------------*/
    /*  Init SD card                                                         */
    /*-----------------------------------------------------------------------*/
    /* clock from PLL */
    sicIoctl(SIC_SET_CLOCK, sysGetPLLOutputKhz(eSYS_UPLL, u32ExtFreq/1000), 0, 0);
    sicOpen();
    sysprintf("total sectors (%x)\n", sicSdOpen0());
#else
    StorageForNAND();
#endif

// LCD initial
//  LCD_initial();
    lcdFormat.ucVASrcFormat = DRVVPOST_FRAME_RGB565;
    u8FrameBufPtr  = (UINT8 *)((UINT32)u8FrameBuf | BIT31);
    u8FrameBuf2Ptr = (UINT8 *)((UINT32)u8FrameBufPtr + 480 * 272 * 2);
    vpostLCMInit(&lcdFormat, (UINT32*)u8FrameBufPtr);
    /* If backlight control signal is different from nuvoton's demo board,
        please don't call this function and must implement another similar one to enable LCD backlight. */
//    vpostEnaBacklight();
}

/*********************************************************************
*
*       main
*/

volatile static I16 s_i16PendingBuf;
volatile static I16 s_i16VsyncFlag;

PFN_DRVVPOST_INT_CALLBACK fun_ptr;

void VPOST_InterruptServiceRiuntine(void)
{
    vpostDisableInt(eDRVVPOST_VINT);

    if (s_i16PendingBuf == 0)
    {
        vpostSetFrameBuffer((U32)u8FrameBuf2Ptr);
        s_i16PendingBuf = 1;
    }
    else
    {
        vpostSetFrameBuffer((U32)u8FrameBufPtr);
        s_i16PendingBuf = 0;
    }
    s_i16VsyncFlag = 1;
}

extern int Vehicle_Speed, Fuel_Volume, IntakeAir_Temp;

static char s_au8Fps[16];

void MainTask(void)
{
    int t0, t1;

    vpostInstallCallBack(eDRVVPOST_VINT, (PFN_DRVVPOST_INT_CALLBACK)VPOST_InterruptServiceRiuntine,  (PFN_DRVVPOST_INT_CALLBACK*)&fun_ptr);
    vpostDisableInt(eDRVVPOST_VINT);
    sysEnableInterrupt(IRQ_VPOST);

    GUI_Init();

    Vehicle_Speed = 0;
    Fuel_Volume = 0;
    IntakeAir_Temp = -40;

    while (1)
    {
        t0 = GUI_X_GetTime();

        Vehicle_Speed += 3;
        if (Vehicle_Speed > 240)
            Vehicle_Speed = 0;
        IntakeAir_Temp += 2;
        if (IntakeAir_Temp > 150)
            IntakeAir_Temp = -40;
        Fuel_Volume += 1;
        if (Fuel_Volume > 240)
            Fuel_Volume = 0;

        if (s_i16PendingBuf == 0)
        {
            LCD_SetVRAMAddrEx(0, (void *)u8FrameBuf2Ptr);
        }
        else
        {
            LCD_SetVRAMAddrEx(0, (void *)u8FrameBufPtr);
        }

        NVT_Meter1(0, 0);
//        NVT_Logo1(8, 8);

        NVT_CxtUpdate();

        GUI_SetFont(GUI_FONT_20F_ASCII);
        sprintf(s_au8Fps, "FPS=%02d", t1);
        GUI_DispStringAt(s_au8Fps, 48, 272-16);

        s_i16VsyncFlag = 0;
        vpostEnableInt(eDRVVPOST_VINT);
        while (s_i16VsyncFlag == 0);
        t0 = GUI_X_GetTime() - t0;
//        sysprintf("%d %s\n", t0, s_au8Fps);
        t1 = (U32)((1.0f / (float)t0) * 1000.0f);
    }
}

int main(void)
{
    char szFileName[20];
    char szCalibrationFile[40];
    int hFile;

#if GUI_SUPPORT_TOUCH
    g_enable_Touch = 0;
#endif

    _SYS_Init();

#if GUI_SUPPORT_TOUCH
    Init_TouchPanel();

    sprintf(szFileName, "C:\\ts_calib");
    fsAsciiToUnicode(szFileName, szCalibrationFile, TRUE);
    hFile = fsOpenFile(szCalibrationFile, szFileName, O_RDONLY | O_FSEEK);
    sysprintf("file = %d\n", hFile);
    if (hFile < 0)
    {
        // file does not exists, so do calibration
        hFile = fsOpenFile(szCalibrationFile, szFileName, O_CREATE|O_RDWR | O_FSEEK);
        if ( hFile < 0 )
        {
            sysprintf("CANNOT create the calibration file\n");
            return -1;
        }
        GUI_Init();
        ts_calibrate(LCD_XSIZE, LCD_YSIZE);
        ts_writefile(hFile);
    }
    else
    {
        ts_readfile(hFile);
    }
    fsCloseFile(hFile);

#ifndef STORAGE_SD
//    GNAND_UnMountNandDisk(&ptNDisk);
//    sicClose();
#endif
    g_enable_Touch = 1;
#endif

    MainTask();
//    while(1);

//      Uninit_TouchPanel();
//      sysStopTimer(TIMER0);
//    sysDisableCache();
    return 0;
}

/*************************** End of file ****************************/
