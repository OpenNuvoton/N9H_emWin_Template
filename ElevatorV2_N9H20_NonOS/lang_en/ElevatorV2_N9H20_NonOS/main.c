/**************************************************************************//**
 * @file     main.c
 * @brief    To utilize emWin library to demonstrate interactive feature.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2022 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include "N9H20.h"

#include "GUI.h"
#include "LCDConf.h"

#include "WM.h"
#include "TEXT.h"
#include "FRAMEWIN.h"

#include "N9H20TouchPanel.h"

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
UINT8 u8FrameBuf[XSIZE_PHYS*YSIZE_PHYS*2];
#else
//
// Background buffer to show JPEG decoded data
//
static UINT8 u8FrameBuf[XSIZE_PHYS*YSIZE_PHYS*2] __attribute__((aligned(32)));
//
// OSD buffer to show floor number, up & down
//
static UINT8 u8OSDFrameBuf[XSIZE_PHYS_OSD*YSIZE_PHYS_OSD*4] __attribute__((aligned(32)));
#endif

UINT8 *u8FrameBufPtr;
UINT8 *u8OSDFrameBufPtr;

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
*       RTC
*/
RTC_TIME_DATA_T g_sCurTime;

char g_ai8Date[64];
char g_ai8DayOfWeek[16];
char g_ai8AMPM[16];

VOID RTC_TickISR(VOID)
{
    //sysprintf("   Current Time:%d/%02d/%02d %02d:%02d:%02d\n",g_sCurTime.u32Year,g_sCurTime.u32cMonth,g_sCurTime.u32cDay,g_sCurTime.u32cHour,g_sCurTime.u32cMinute,g_sCurTime.u32cSecond);
}

static void RTC_TimeDisplay(void)
{
    RTC_TICK_T sTick;

    sysprintf("RTC Time Display \n");

    /* Set Tick property */
    sTick.ucMode = RTC_TICK_1_SEC;
    sTick.pfnTickCallBack = RTC_TickISR;

    g_sCurTime.u32Year         = 2022;
    g_sCurTime.u32cMonth       = 10;
    g_sCurTime.u32cDay         = 13;
    g_sCurTime.u32cHour        = 6;
    g_sCurTime.u32cMinute      = 10;
    g_sCurTime.u32cSecond      = 50;
    g_sCurTime.u32cDayOfWeek   = (4 - 1);   /* -1 */
    g_sCurTime.u8cClockDisplay = RTC_CLOCK_12;
    g_sCurTime.u8cAmPm         = RTC_PM;
    RTC_Write(RTC_CURRENT_TIME, &g_sCurTime);

    /* Set Tick setting */
    RTC_Ioctl(0,RTC_IOC_SET_TICK_MODE, (UINT32)&sTick,0);

    /* Enable RTC Tick Interrupt and install tick call back function */
    RTC_Ioctl(0,RTC_IOC_ENABLE_INT, (UINT32)RTC_TICK_INT,0);

}

/*********************************************************************
*
*       _SYS_Init
*/
void VPOST_OSD_Init(void)
{
    S_DRVVPOST_OSD_SIZE sSize;
    S_DRVVPOST_OSD_POS sPos;
    S_DRVVPOST_OSD sOSD;

//    u8OSDFrameBufPtr = (UINT8 *)((UINT32)u8OSDFrameBuf | BIT31);
    u8OSDFrameBufPtr = (UINT8 *)((UINT32)u8OSDFrameBuf | BIT0);

    /* set OSD size */
    sSize.u16VSize = LCD_YSIZE_OSD;
    sSize.u16HSize = LCD_XSIZE_OSD;

    /* set OSD position */
    //
    // Bounding box position in background canvas
    //
    sPos.u16VStart_1st = (59);
    sPos.u16VEnd_1st = LCD_YSIZE_OSD + sPos.u16VStart_1st - 1;
    sPos.u16VOffset_2nd = 0;
    sPos.u16HStart_1st = (48);
    sPos.u16HEnd_1st = LCD_XSIZE_OSD + sPos.u16HStart_1st - 1;
    sPos.u16HOffset_2nd = 0;

    sOSD.eType  = eDRVVPOST_OSD_ARGB888; // eDRVVPOST_OSD_RGB565 eDRVVPOST_OSD_RGBx888 eDRVVPOST_OSD_ARGB888
    sOSD.psSize = &sSize;
    sOSD.psPos  = &sPos;

    vpostSetOSD_DataType(sOSD.eType);
    vpostSetOSD_BaseAddress((UINT32)u8OSDFrameBufPtr);
    vpostSetOSD_Size(sOSD.psSize);
    vpostSetOSD_Pos(sOSD.psPos);
    vpostSetOSD_Enable();

    vpostSetOSD_Transparent_Disable();
//    vpostSetOSD_Transparent(eDRVVPOST_OSD_TRANSPARENT_RGB888, 0x00FFFFFF);
}

#define DEF_OSD_COLORKEY    GUI_TRANSPARENT
//#define DEF_OSD_COLORKEY    GUI_RED

GUI_MEMDEV_Handle hMemM0S0;
GUI_MEMDEV_Handle hMemM0S02;
GUI_MEMDEV_Handle hMemFloorNumber;

volatile int g_flagM0S0;
volatile int g_flag2M0S0;

volatile unsigned int * g_prt1M0S0;
volatile unsigned int * g_prt2M0S0;

extern GUI_CONST_STORAGE GUI_BITMAP bmf0;
extern GUI_CONST_STORAGE GUI_BITMAP bmf1;
extern GUI_CONST_STORAGE GUI_BITMAP bmf2;
extern GUI_CONST_STORAGE GUI_BITMAP bmf3;
extern GUI_CONST_STORAGE GUI_BITMAP bmf4;
extern GUI_CONST_STORAGE GUI_BITMAP bmf5;
extern GUI_CONST_STORAGE GUI_BITMAP bmf6;
extern GUI_CONST_STORAGE GUI_BITMAP bmf7;
extern GUI_CONST_STORAGE GUI_BITMAP bmf8;
extern GUI_CONST_STORAGE GUI_BITMAP bmf9;

extern GUI_CONST_STORAGE GUI_BITMAP bmu00;
extern GUI_CONST_STORAGE GUI_BITMAP bmu01;
extern GUI_CONST_STORAGE GUI_BITMAP bmu02;
extern GUI_CONST_STORAGE GUI_BITMAP bmu03;
extern GUI_CONST_STORAGE GUI_BITMAP bmu04;
extern GUI_CONST_STORAGE GUI_BITMAP bmu05;
extern GUI_CONST_STORAGE GUI_BITMAP bmu06;
extern GUI_CONST_STORAGE GUI_BITMAP bmu07;
extern GUI_CONST_STORAGE GUI_BITMAP bmu08;
extern GUI_CONST_STORAGE GUI_BITMAP bmu09;

extern GUI_CONST_STORAGE GUI_BITMAP bmu10;
extern GUI_CONST_STORAGE GUI_BITMAP bmu11;
extern GUI_CONST_STORAGE GUI_BITMAP bmu12;
extern GUI_CONST_STORAGE GUI_BITMAP bmu13;
extern GUI_CONST_STORAGE GUI_BITMAP bmu14;
extern GUI_CONST_STORAGE GUI_BITMAP bmu15;
extern GUI_CONST_STORAGE GUI_BITMAP bmu16;
extern GUI_CONST_STORAGE GUI_BITMAP bmu17;
extern GUI_CONST_STORAGE GUI_BITMAP bmu18;
extern GUI_CONST_STORAGE GUI_BITMAP bmu19;

extern GUI_CONST_STORAGE GUI_BITMAP bmu20;
extern GUI_CONST_STORAGE GUI_BITMAP bmu21;
extern GUI_CONST_STORAGE GUI_BITMAP bmu22;
extern GUI_CONST_STORAGE GUI_BITMAP bmu23;
extern GUI_CONST_STORAGE GUI_BITMAP bmu24;
extern GUI_CONST_STORAGE GUI_BITMAP bmu25;

extern GUI_CONST_STORAGE GUI_BITMAP bmd00;
extern GUI_CONST_STORAGE GUI_BITMAP bmd01;
extern GUI_CONST_STORAGE GUI_BITMAP bmd02;
extern GUI_CONST_STORAGE GUI_BITMAP bmd03;
extern GUI_CONST_STORAGE GUI_BITMAP bmd04;
extern GUI_CONST_STORAGE GUI_BITMAP bmd05;
extern GUI_CONST_STORAGE GUI_BITMAP bmd06;
extern GUI_CONST_STORAGE GUI_BITMAP bmd07;
extern GUI_CONST_STORAGE GUI_BITMAP bmd08;
extern GUI_CONST_STORAGE GUI_BITMAP bmd09;

extern GUI_CONST_STORAGE GUI_BITMAP bmd10;
extern GUI_CONST_STORAGE GUI_BITMAP bmd11;
extern GUI_CONST_STORAGE GUI_BITMAP bmd12;
extern GUI_CONST_STORAGE GUI_BITMAP bmd13;
extern GUI_CONST_STORAGE GUI_BITMAP bmd14;
extern GUI_CONST_STORAGE GUI_BITMAP bmd15;
extern GUI_CONST_STORAGE GUI_BITMAP bmd16;
extern GUI_CONST_STORAGE GUI_BITMAP bmd17;
extern GUI_CONST_STORAGE GUI_BITMAP bmd18;
extern GUI_CONST_STORAGE GUI_BITMAP bmd19;

extern GUI_CONST_STORAGE GUI_BITMAP bmd20;
extern GUI_CONST_STORAGE GUI_BITMAP bmd21;
extern GUI_CONST_STORAGE GUI_BITMAP bmd22;
extern GUI_CONST_STORAGE GUI_BITMAP bmd23;
extern GUI_CONST_STORAGE GUI_BITMAP bmd24;
extern GUI_CONST_STORAGE GUI_BITMAP bmd25;

void VPOST_InterruptServicerRoutine()
{
    if (g_flag2M0S0 == 1)
    {
        if (g_flagM0S0 == 0)
        {
            vpostSetOSD_BaseAddress((UINT32)g_prt1M0S0);
            g_flagM0S0 = 1;
        }
        else if (g_flagM0S0 == 1)
        {
            vpostSetOSD_BaseAddress((UINT32)g_prt2M0S0);
            g_flagM0S0 = 0;
        }
        else
        {
            vpostSetOSD_BaseAddress((UINT32)u8OSDFrameBufPtr);
            g_flagM0S0 = 0;
        }

        g_flag2M0S0 = 0;
    }
}

static int s_i32ArrowX;
static int s_i32ArrowY;
static int s_i32FloorNumberX;
static int s_i32FloorNumberY;
static int s_i32FloorNumber10X;
static int s_i32FloorNumber10Y;
static int s_i32FloorNumber01X;
static int s_i32FloorNumber01Y;

void NVT_SetupRes(void)
{
    PFN_DRVVPOST_INT_CALLBACK fun_ptr;
    //
    // Canvas 1 for ping-pong buffer flow
    //
    if (hMemM0S0 == 0)
        hMemM0S0 = GUI_MEMDEV_CreateFixed32(0, 0, LCD_XSIZE_OSD, LCD_YSIZE_OSD);
    //
    // Canvas 2 for ping-pong buffer flow
    //
    if (hMemM0S02 == 0)
        hMemM0S02 = GUI_MEMDEV_CreateFixed32(0, 0, LCD_XSIZE_OSD, LCD_YSIZE_OSD);
    //
    // Floor source buffer for scaling
    //
    if (hMemFloorNumber == 0)
        hMemFloorNumber = GUI_MEMDEV_CreateFixed32(0, 0, (bmf0.XSize + 4 + bmf0.XSize), bmf0.YSize);

    g_flagM0S0 = 2;
    g_flag2M0S0 = 1;

    //Init VPOST
    vpostInstallCallBack(eDRVVPOST_VINT, (PFN_DRVVPOST_INT_CALLBACK)VPOST_InterruptServicerRoutine, (PFN_DRVVPOST_INT_CALLBACK *)&fun_ptr);
    vpostEnableInt(eDRVVPOST_VINT);
    sysEnableInterrupt(IRQ_VPOST);

    while(g_flag2M0S0 == 1);

    GUI_MEMDEV_Select(hMemM0S0);

    GUI_SetBkColor(DEF_OSD_COLORKEY);
    GUI_Clear();

    g_prt1M0S0 = GUI_MEMDEV_GetDataPtr(hMemM0S0);

    GUI_MEMDEV_Select(hMemM0S02);

    GUI_SetBkColor(DEF_OSD_COLORKEY);
    GUI_Clear();

    g_prt2M0S0 = GUI_MEMDEV_GetDataPtr(hMemM0S02);
    //
    // Arrow position in bounding box
    //
    s_i32ArrowX = (0);
    s_i32ArrowY = (99 - 59);
    //
    // Floor number position for units digit in bounding box
    //
    s_i32FloorNumberX = (bmu00.XSize + 37 + (57));
    s_i32FloorNumberY = (0);
    //
    // Floor number position for tens digit in bounding box
    //
    s_i32FloorNumber10X = (bmu00.XSize + 37);
    s_i32FloorNumber10Y = (0);
    s_i32FloorNumber01X = (bmu00.XSize + 37 + bmf0.XSize + 4);
    s_i32FloorNumber01Y = (0);

    GUI_SetFont(GUI_FONT_32B_ASCII);
    GUI_SetColor(GUI_WHITE);

    RTC_Init();
    RTC_TimeDisplay();
}
//
// Ping-pong buffer control
//
void NVT_Rot(void)
{
    sysFlushCache(D_CACHE);

    g_flag2M0S0 = 1;

    while(g_flag2M0S0 == 1);
}

extern int NVT_DecodeBGJPEG(void);

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

//    u8FrameBufPtr = (UINT8 *)((UINT32)u8FrameBuf | BIT31);
    u8FrameBufPtr = (UINT8 *)((UINT32)u8FrameBuf | BIT0);
    NVT_DecodeBGJPEG();

// LCD initial
//  LCD_initial();
    lcdFormat.ucVASrcFormat = DRVVPOST_FRAME_YCBYCR;    // DRVVPOST_FRAME_YCBYCR DRVVPOST_FRAME_RGB565 DRVVPOST_FRAME_RGBx888
    vpostLCMInit(&lcdFormat, (UINT32*)u8FrameBuf);
//    u8FrameBufPtr  = (UINT8 *)((UINT32)u8FrameBuf | 0x80000000);
    /* If backlight control signal is different from nuvoton's demo board,
        please don't call this function and must implement another similar one to enable LCD backlight. */
//    vpostEnaBacklight();

    VPOST_OSD_Init();
}

/*********************************************************************
*
*       main
*/

GUI_CONST_STORAGE GUI_BITMAP *pbmf[10];

GUI_CONST_STORAGE GUI_BITMAP *pbmu[26];

GUI_CONST_STORAGE GUI_BITMAP *pbmd[26];

void NVT_ShowFloorNumber(const GUI_BITMAP * pBM, int dx, int dy, int Mag)
{
    if (g_flagM0S0 == 0)
    {
        GUI_MEMDEV_Select(hMemM0S0);
    }
    else
    {
        GUI_MEMDEV_Select(hMemM0S02);
    }

    GUI_SetBkColor(DEF_OSD_COLORKEY);
    GUI_ClearRect(dx, dy, dx + pBM->XSize + 4 + pBM->XSize - 1, dy + pBM->YSize - 1);

    if (g_flagM0S0 == 0)
    {
        GUI_MEMDEV_RotateHR(hMemFloorNumber, hMemM0S0, dx * 8, dy * 8, 0, Mag);
    }
    else
    {
        GUI_MEMDEV_RotateHR(hMemFloorNumber, hMemM0S02, dx * 8, dy * 8, 0, Mag);
    }
}

void NVT_ShowUp(const GUI_BITMAP * pBM, int x0, int y0)
{
    if (g_flagM0S0 == 0)
    {
        GUI_MEMDEV_Select(hMemM0S0);
    }
    else
    {
        GUI_MEMDEV_Select(hMemM0S02);
    }

    GUI_SetBkColor(DEF_OSD_COLORKEY);
    GUI_ClearRect(x0, y0, x0 + pBM->XSize - 1, y0 + pBM->YSize - 1);

    GUI_DrawBitmap(pBM, x0, y0);
}

void NVT_ShowDown(const GUI_BITMAP * pBM, int x0, int y0)
{
    if (g_flagM0S0 == 0)
    {
        GUI_MEMDEV_Select(hMemM0S0);
    }
    else
    {
        GUI_MEMDEV_Select(hMemM0S02);
    }

    GUI_SetBkColor(DEF_OSD_COLORKEY);
    GUI_ClearRect(x0, y0, x0 + pBM->XSize - 1, y0 + pBM->YSize - 1);

    GUI_DrawBitmap(pBM, x0, y0);
}

static char s_szFileNameSrc[32];
static char s_szFileNameSrc2[64];
static char s_szFileNameDst[32];
static char s_szFileNameDst2[64];

extern void _WAV_Decode(char * szFileName);
extern void _NVTDiskInfo(void);

static int t;

void MainTask(void)
{
    int fi, ui, di, Mag, fiFlag;
    unsigned int key;   // Key for action
    //
    // Init key for action
    //
    kpi_init();
    kpi_open(3); // use nIRQ0 as external interrupt source

    GUI_Init();

    GUI_EnableAlpha(0);

    NVT_SetupRes();
    //
    // Floor number
    //
    pbmf[0] = &bmf0;
    pbmf[1] = &bmf1;
    pbmf[2] = &bmf2;
    pbmf[3] = &bmf3;
    pbmf[4] = &bmf4;
    pbmf[5] = &bmf5;
    pbmf[6] = &bmf6;
    pbmf[7] = &bmf7;
    pbmf[8] = &bmf8;
    pbmf[9] = &bmf9;
    //
    // Up arrow
    //
    pbmu[0] = &bmu00;
    pbmu[1] = &bmu01;
    pbmu[2] = &bmu02;
    pbmu[3] = &bmu03;
    pbmu[4] = &bmu04;
    pbmu[5] = &bmu05;
    pbmu[6] = &bmu06;
    pbmu[7] = &bmu07;
    pbmu[8] = &bmu08;
    pbmu[9] = &bmu09;

    pbmu[10] = &bmu10;
    pbmu[11] = &bmu11;
    pbmu[12] = &bmu12;
    pbmu[13] = &bmu13;
    pbmu[14] = &bmu14;
    pbmu[15] = &bmu15;
    pbmu[16] = &bmu16;
    pbmu[17] = &bmu17;
    pbmu[18] = &bmu18;
    pbmu[19] = &bmu19;

    pbmu[20] = &bmu20;
    pbmu[21] = &bmu21;
    pbmu[22] = &bmu22;
    pbmu[23] = &bmu23;
    pbmu[24] = &bmu24;
    pbmu[25] = &bmu25;
    //
    // Down arrow
    //
    pbmd[0] = &bmd00;
    pbmd[1] = &bmd01;
    pbmd[2] = &bmd02;
    pbmd[3] = &bmd03;
    pbmd[4] = &bmd04;
    pbmd[5] = &bmd05;
    pbmd[6] = &bmd06;
    pbmd[7] = &bmd07;
    pbmd[8] = &bmd08;
    pbmd[9] = &bmd09;

    pbmd[10] = &bmd10;
    pbmd[11] = &bmd11;
    pbmd[12] = &bmd12;
    pbmd[13] = &bmd13;
    pbmd[14] = &bmd14;
    pbmd[15] = &bmd15;
    pbmd[16] = &bmd16;
    pbmd[17] = &bmd17;
    pbmd[18] = &bmd18;
    pbmd[19] = &bmd19;

    pbmd[20] = &bmd20;
    pbmd[21] = &bmd21;
    pbmd[22] = &bmd22;
    pbmd[23] = &bmd23;
    pbmd[24] = &bmd24;
    pbmd[25] = &bmd25;

    while (1)
    {
        //
        // Floor up from 1 to 10
        //
        //
        // Ping-pong buffer flow: draw canvas
        //
        if (g_flagM0S0 == 0)
        {
            GUI_MEMDEV_Select(hMemM0S0);
        }
        else
        {
            GUI_MEMDEV_Select(hMemM0S02);
        }

        GUI_SetBkColor(DEF_OSD_COLORKEY);
        GUI_Clear();

        ui = 0;
        GUI_DrawBitmap(pbmu[ui], s_i32ArrowX, s_i32ArrowY);

        fi = 1;
        GUI_DrawBitmap(pbmf[fi], s_i32FloorNumberX, s_i32FloorNumberY);

        /* Get the current time */
        RTC_Read(RTC_CURRENT_TIME, &g_sCurTime);
        memset(g_ai8Date, 0x00, 64);
        memset(g_ai8DayOfWeek, 0x00, 16);
        memset(g_ai8AMPM, 0x00, 16);
        switch (g_sCurTime.u32cDayOfWeek)
        {
        case 0:
            sprintf(g_ai8DayOfWeek, "Mon");
            break;
        case 1:
            sprintf(g_ai8DayOfWeek, "Tue");
            break;
        case 2:
            sprintf(g_ai8DayOfWeek, "Wed");
            break;
        case 3:
            sprintf(g_ai8DayOfWeek, "Thu");
            break;
        case 4:
            sprintf(g_ai8DayOfWeek, "Fri");
            break;
        case 5:
            sprintf(g_ai8DayOfWeek, "Sat");
            break;
        default:
            sprintf(g_ai8DayOfWeek, "Sun");
            break;
        }

        switch (g_sCurTime.u8cAmPm)
        {
        case 1:
            sprintf(g_ai8AMPM, "AM");
            break;
        default:
            sprintf(g_ai8AMPM, "PM");
            break;
        }
        sprintf(g_ai8Date, "%02d/%02d %s. %02d:%02d %s", g_sCurTime.u32cMonth, g_sCurTime.u32cDay, g_ai8DayOfWeek, g_sCurTime.u32cHour, g_sCurTime.u32cMinute, g_ai8AMPM);

        GUI_DispStringAt(g_ai8Date, 60, 160);
        //
        // Ping-pong buffer flow: switch canvas
        //
        NVT_Rot();
#if 0
        GUI_Delay(500);
#else
        _WAV_Decode("D:\\WAV\\FL\\001.wav");
        _WAV_Decode("D:\\WAV\\DO\\open001.wav");
#endif
        //
        // Ping-pong buffer flow: draw another canvas
        //
        if (g_flagM0S0 == 0)
        {
            GUI_MEMDEV_Select(hMemM0S0);
        }
        else
        {
            GUI_MEMDEV_Select(hMemM0S02);
        }

        GUI_SetBkColor(DEF_OSD_COLORKEY);
        GUI_Clear();

        ui = 0;
        GUI_DrawBitmap(pbmu[ui], s_i32ArrowX, s_i32ArrowY);

        fi = 1;
        GUI_DrawBitmap(pbmf[fi], s_i32FloorNumberX, s_i32FloorNumberY);

        GUI_DispStringAt(g_ai8Date, 60, 160);

        //
        // Ping-pong buffer flow: switch canvas
        //
        NVT_Rot();
#if 0
        GUI_Delay(500);
#else
        _WAV_Decode("D:\\WAV\\DO\\close001.wav");
#endif
        ui = 0;
        fi = 1;
        Mag = 700;  /* Scaling factor 700 mean 0.7X */
        fiFlag = 0;

        while (1)
        {
            t = GUI_X_GetTime();
            //
            // Read key value and action later
            //
            key = kpi_read(KPI_NONBLOCK);
            //
            // Key action to load & run lang_ko FW
            //
            if (key != 0)
            {
                if (key == 1)
                {
                    goto __NVT_EXIT__;
                }
                else if (key == 2)
                {
                    goto __NVT_EXIT__;
                }
                else if ((key == 4) || (key == 8))
                {
                    goto __NVT_EXIT__;
                }
                else
                {
                    goto __NVT_EXIT__;
                }
            }

            NVT_ShowUp(pbmu[ui], s_i32ArrowX, s_i32ArrowY);
            ui++;
            if (ui > 25)
            {
                ui = 0;
            }

            if (fiFlag == 0)
            {
                fi++;
                GUI_MEMDEV_Select(hMemFloorNumber);
                GUI_SetBkColor(DEF_OSD_COLORKEY);
                GUI_Clear();
                if (fi < 10 )
                    GUI_DrawBitmap(pbmf[fi], ((bmf0.XSize + 4 + bmf0.XSize - bmf0.XSize) / 2), 0);
                else
                {
                    GUI_DrawBitmap(pbmf[fi / 10], 0, 0);
                    GUI_DrawBitmap(pbmf[fi % 10], (bmf0.XSize + 4), 0);
                }
                fiFlag = 1;
            }

            if (fiFlag == 1)
            {
                if (Mag > 1000)
                {
                    NVT_ShowFloorNumber(pbmf[0], s_i32FloorNumber10X, 0, 1000);   /* scaling factor 1000 means 1.0X */
                    fiFlag = 2;
                }
                else
                    NVT_ShowFloorNumber(pbmf[0], s_i32FloorNumber10X, 0, Mag);
                Mag += 100;
            }
            else if (fiFlag >= 2)
            {
                fiFlag++;
            }

            if (fiFlag >= 21)
            {
                if (ui == 3)
                {
                    Mag = 700;
                    fiFlag = 0;
                    if (fi == 10)
                        break;
                }
            }

            NVT_Rot();

            t = GUI_X_GetTime() - t;

            if (t >= 40)
            {
                sysprintf("[%d]\r\n", t);
            }
            else
            {
                t = 40 - t;
                GUI_X_Delay(t);
            }
        }
        //
        // Floor down from 10 to 1
        //
        if (g_flagM0S0 == 0)
        {
            GUI_MEMDEV_Select(hMemM0S0);
        }
        else
        {
            GUI_MEMDEV_Select(hMemM0S02);
        }

        GUI_SetBkColor(DEF_OSD_COLORKEY);
        GUI_Clear();

        di = 0;
        GUI_DrawBitmap(pbmd[di], s_i32ArrowX, s_i32ArrowY);

        fi = 10;
        GUI_DrawBitmap(pbmf[fi / 10], s_i32FloorNumber10X, s_i32FloorNumber10Y);
        GUI_DrawBitmap(pbmf[fi % 10], s_i32FloorNumber01X, s_i32FloorNumber01Y);

        /* Get the current time */
        RTC_Read(RTC_CURRENT_TIME, &g_sCurTime);
        memset(g_ai8Date, 0x00, 64);
        memset(g_ai8DayOfWeek, 0x00, 16);
        memset(g_ai8AMPM, 0x00, 16);
        switch (g_sCurTime.u32cDayOfWeek)
        {
        case 0:
            sprintf(g_ai8DayOfWeek, "Mon");
            break;
        case 1:
            sprintf(g_ai8DayOfWeek, "Tue");
            break;
        case 2:
            sprintf(g_ai8DayOfWeek, "Wed");
            break;
        case 3:
            sprintf(g_ai8DayOfWeek, "Thu");
            break;
        case 4:
            sprintf(g_ai8DayOfWeek, "Fri");
            break;
        case 5:
            sprintf(g_ai8DayOfWeek, "Sat");
            break;
        default:
            sprintf(g_ai8DayOfWeek, "Sun");
            break;
        }

        switch (g_sCurTime.u8cAmPm)
        {
        case 1:
            sprintf(g_ai8AMPM, "AM");
            break;
        default:
            sprintf(g_ai8AMPM, "PM");
            break;
        }
        sprintf(g_ai8Date, "%02d/%02d %s. %02d:%02d %s", g_sCurTime.u32cMonth, g_sCurTime.u32cDay, g_ai8DayOfWeek, g_sCurTime.u32cHour, g_sCurTime.u32cMinute, g_ai8AMPM);

        GUI_DispStringAt(g_ai8Date, 60, 160);

        NVT_Rot();
#if 0
        GUI_Delay(500);
#else
        _WAV_Decode("D:\\WAV\\FL\\010.wav");
        _WAV_Decode("D:\\WAV\\DO\\open001.wav");
#endif
        if (g_flagM0S0 == 0)
        {
            GUI_MEMDEV_Select(hMemM0S0);
        }
        else
        {
            GUI_MEMDEV_Select(hMemM0S02);
        }

        GUI_SetBkColor(DEF_OSD_COLORKEY);
        GUI_Clear();

        di = 0;
        GUI_DrawBitmap(pbmd[di], s_i32ArrowX, s_i32ArrowY);

        fi = 10;
        GUI_DrawBitmap(pbmf[fi / 10], s_i32FloorNumber10X, s_i32FloorNumber10Y);
        GUI_DrawBitmap(pbmf[fi % 10], s_i32FloorNumber01X, s_i32FloorNumber01Y);

        GUI_DispStringAt(g_ai8Date, 60, 160);

        NVT_Rot();
#if 0
        GUI_Delay(500);
#else
        _WAV_Decode("D:\\WAV\\DO\\close001.wav");
#endif
        di = 0;
        fi = 10;
        Mag = 700;
        fiFlag = 0;

        while (1)
        {
            t = GUI_X_GetTime();
            //
            // Read key value and action later
            //
            key = kpi_read(KPI_NONBLOCK);
            //
            // Key action
            //
            if (key != 0)
            {
                if (key == 1)
                {
                    goto __NVT_EXIT__;
                }
                else if (key == 2)
                {
                    goto __NVT_EXIT__;
                }
                else if ((key == 4) || (key == 8))
                {
                    goto __NVT_EXIT__;
                }
                else
                {
                    goto __NVT_EXIT__;
                }
            }

            NVT_ShowDown(pbmd[di], s_i32ArrowX, s_i32ArrowY);
            di++;
            if (di > 25)
            {
                di = 0;
            }

            if (fiFlag == 0)
            {
                fi--;
                GUI_MEMDEV_Select(hMemFloorNumber);
                GUI_SetBkColor(DEF_OSD_COLORKEY);
                GUI_Clear();
                if (fi < 10 )
                    GUI_DrawBitmap(pbmf[fi], ((bmf0.XSize + 4 + bmf0.XSize - bmf0.XSize) / 2), 0);
                else
                {
                    GUI_DrawBitmap(pbmf[fi / 10], 0, 0);
                    GUI_DrawBitmap(pbmf[fi % 10], (bmf0.XSize + 4), 0);
                }
                fiFlag = 1;
            }

            if (fiFlag == 1)
            {
                if (Mag > 1000)
                {
                    NVT_ShowFloorNumber(pbmf[0], s_i32FloorNumber10X, 0, 1000);
                    fiFlag = 2;
                }
                else
                    NVT_ShowFloorNumber(pbmf[0], s_i32FloorNumber10X, 0, Mag);
                Mag += 100;
            }
            else if (fiFlag >= 2)
            {
                fiFlag++;
            }

            if (fiFlag >= 21)
            {
                if (di == 8)
                {
                    Mag = 700;
                    fiFlag = 0;
                    if (fi == 1)
                        break;
                }
            }

            NVT_Rot();

            t = GUI_X_GetTime() - t;

            if (t >= 40)
            {
                sysprintf("[%d]\r\n", t);
            }
            else
            {
                t = 40 - t;
                GUI_X_Delay(t);
            }
        }

        sysprintf("\r\n");
        sysprintf("############## max[%d] free[%d] used[%d]\r\n", GUI_ALLOC_GetMaxUsedBytes(), GUI_ALLOC_GetNumFreeBytes(), GUI_ALLOC_GetNumUsedBytes());
        _NVTDiskInfo();
        sysprintf("\r\n");
    }

__NVT_EXIT__:

    sprintf(s_szFileNameSrc, "C:\\conprog.bin");
    fsAsciiToUnicode(s_szFileNameSrc, s_szFileNameSrc2, TRUE);

    sprintf(s_szFileNameDst, "C:\\3onprog.bin");
    fsAsciiToUnicode(s_szFileNameDst, s_szFileNameDst2, TRUE);

    fsRenameFile(s_szFileNameSrc2, s_szFileNameSrc, s_szFileNameDst2, s_szFileNameDst, FALSE);

    sprintf(s_szFileNameSrc, "C:\\1onprog.bin");
    fsAsciiToUnicode(s_szFileNameSrc, s_szFileNameSrc2, TRUE);

    sprintf(s_szFileNameDst, "C:\\conprog.bin");
    fsAsciiToUnicode(s_szFileNameDst, s_szFileNameDst2, TRUE);

    fsRenameFile(s_szFileNameSrc2, s_szFileNameSrc, s_szFileNameDst2, s_szFileNameDst, FALSE);

    sysSetWatchDogTimerInterval(3);  /* The reset time WDT_INTERVAL_3 is about 22.391s base on 12MHz*/
    sysEnableWatchDogTimer();
    sysEnableWatchDogTimerReset();
    sysSetWatchDogTimerInterval(0);

    while(1);
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
