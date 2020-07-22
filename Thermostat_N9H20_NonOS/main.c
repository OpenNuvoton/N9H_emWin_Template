/***************************************************************************
 *                                                                                                               *
 * Copyright (c) Nuvoton Technolog. All rights reserved.                                    *
 *                                                                                                               *
 ***************************************************************************/

#include "N9H20.h"

#include "GUI.h"
#include "LCDConf.h"

#include "WM.h"
#include "TEXT.h"
#include "FRAMEWIN.h"

#include "N9H20TouchPanel.h"

// Modbus Master include file
#include  "def.h"

/*********************************************************************
Constraints on LCD panel N9H20_VPOST_FW050TFT_800x480.lib:

1. Resolution is 800x480. (N9H20K1 has no such target because of the DRAM size limitation)
2. VPOST OSD function must disable. (VPOST H/W limitation)
3. PLL clock is 192 MHz, the pixel clock is 192/6 = 32 MHz.
4. 32,000,000/1056/525 = 57.7 FPS (frame per second)
5. If the bus bandwidth condition is too busy it may causes blinking result.
*/

#if __DEMO_INIT_BY_DRIVER__
#ifdef __ICCARM__
#pragma data_alignment = 32
UINT8 u8FrameBuf[XSIZE_PHYS*YSIZE_PHYS*2*2];
#else
UINT8 u8FrameBuf[XSIZE_PHYS*YSIZE_PHYS*2*2] __attribute__((aligned(32)));
#endif
#endif

UINT8 *u8FrameBufPtr;

extern volatile int OS_TimeMS;
volatile int g_enable_Touch;

extern int ts_writefile(int hFile);
extern int ts_readfile(int hFile);
extern int ts_calibrate(int xsize, int ysize);
extern void TouchTask(void);

UARTDEV_T	RS485Uart;
UARTDEV_T* pUartDevISR;

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
#if __DEMO_INIT_BY_DRIVER__
    LCDFORMATEX lcdFormat;
#endif

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

    // For Uart-0
    outp32(REG_APBCLK, inp32(REG_APBCLK)| 0x08);        // enable Uart-0 clock

#ifdef NUV_BOARD
// Init GPIO PD.4 for nRTS control RS485
    outp32(REG_GPDFUN, inp32(REG_GPDFUN) & ~0x0300);
    gpio_setportval(GPIO_PORTD, 0x10, 0x10);
    gpio_setportpull(GPIO_PORTD, 0x10,0x10);
    gpio_setportdir(GPIO_PORTD, 0x010, 0x10);
#else
// Init GPIO PD.3 for nRTS control RS485
    outp32(REG_GPDFUN, inp32(REG_GPDFUN) & ~0x00C0);
    gpio_setportval(GPIO_PORTD, 0x8, 0x8);
    gpio_setportpull(GPIO_PORTD, 0x8,0x8);
    gpio_setportdir(GPIO_PORTD, 0x8, 0x8);
#endif
    // For Uart-0 (GPD.1 --> TX, GPD.2 --> RX)
    outp32(REG_GPDFUN, (inp32(REG_GPDFUN) & ~0x03C) | 0x14);

    uart.uiBaudrate = 9600;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_2;

    register_uart_device(0,&RS485Uart);
    RS485Uart.UartInitialize(&uart);
    pUartDevISR = &RS485Uart;

    sysInvalidCache();
    sysEnableCache(CACHE_WRITE_BACK);
    sysFlushCache(I_D_CACHE);

    /* start timer 0 */
    OS_TimeMS = 0;

    sysSetTimerReferenceClock(TIMER0, u32ExtFreq);  //External Crystal
    sysStartTimer(TIMER0, 1000, PERIODIC_MODE);     /* 1000 ticks/per sec ==> 1tick/1ms */
    sysSetTimerEvent(TIMER0,  1, (PVOID)TMR0_IRQHandler);           /* 1  tick   call back */
    sysSetTimerEvent(TIMER0, 10, (PVOID)TMR0_IRQHandler_TouchTask); /* 10 ticks  call back */

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

#if __DEMO_INIT_BY_DRIVER__
// LCD initial
//  LCD_initial();
    lcdFormat.ucVASrcFormat = DRVVPOST_FRAME_RGB565;
//    vpostLCMInit(&lcdFormat, (UINT32*)u8FrameBuf);
    u8FrameBufPtr  = (UINT8 *)((UINT32)u8FrameBuf | 0x80000000);
    vpostLCMInit(&lcdFormat, (UINT32*)u8FrameBufPtr);
    /* If backlight control signal is different from nuvoton's demo board,
        please don't call this function and must implement another similar one to enable LCD backlight. */
    vpostEnaBacklight();
#else
    u8FrameBufPtr  = (UINT8 *)((UINT32)0x81900000);
#endif
}

/*********************************************************************
*
*       main
*/
#if 0
WM_HWIN CreateFramewin(void);

void MainTask(void)
{
    WM_HWIN hWin;
    char     acVersion[40] = "Framewin: Version of emWin: ";

    GUI_Init();
    hWin = CreateFramewin();
    strcat(acVersion, GUI_GetVersionString());
    FRAMEWIN_SetText(hWin, acVersion);
    while (1)
    {
        GUI_Delay(500);
    }
}
#endif

extern void MainTask(void);

int main(void)
{
#if 0
    char szFileName[20];
    char szCalibrationFile[40];
    int hFile;
#endif

#if GUI_SUPPORT_TOUCH
    g_enable_Touch = 0;
#endif

    _SYS_Init();

#if GUI_SUPPORT_TOUCH
    Init_TouchPanel();
#if 0
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
    GNAND_UnMountNandDisk(&ptNDisk);
    sicClose();
#endif
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
