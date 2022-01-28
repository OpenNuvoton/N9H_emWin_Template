/**************************************************************************//**
 * @file     main.c
 * @brief    N9H30 Meter Sample Code
 *
 * @note
 * Copyright (C) 2022 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "N9H30.h"
#include "sys.h"
#include "uart.h"

#ifdef __USE_SD__
#include "sdh.h"
#else
#include "spi.h"
#include "spilib.h"
#endif

#include "gpio.h"
#include "lcd.h"

#include "TouchPanel.h"
#include "GUI.h"
#include "WM.h"
#include "FRAMEWIN.h"
#include "LCD_Protected.h"
#include "LCDConf.h"
#if GUI_WINSUPPORT
#include "WM.h"
#include "TEXT.h"
#endif

#ifdef __USE_SD__
#include "diskio.h"
#include "ff.h"
#endif

uint8_t *g_VAFrameBuf;

#define uartprintf      sysprintf
#define uartgetchar     sysGetChar
static  UART_T param;

//volatile int ts_flag;
extern void TouchTask(void);

unsigned int RX_FIFO_Cnt=0;
#ifdef __USE_SD__
FATFS FatFs[_VOLUMES];      /* File system object for logical drive */

/* Working buffer */
#ifdef __ICCARM__
#pragma data_alignment = 32
BYTE Buff[1024];
#else
BYTE Buff[1024] __attribute__((aligned(32)));
#endif

FIL hFile;
#else
uint32_t * g_pu32Res;
uint8_t DestArray[1024];
#endif

extern int ts_writefile(void);
extern int ts_readfile(void);
extern int ts_calibrate(int xsize, int ysize);
extern int ts_phy2log(int *sumx, int *sumy);
extern int ts_test(int xsize, int ysize);
WM_HWIN CreateWindow(void);

#ifdef __USE_SD__
/*---------------------------------------------------------*/
/* User Provided RTC Function for FatFs module             */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support an RTC.                     */
/* This function is not required in read-only cfg.         */

unsigned long get_fattime (void)
{
    unsigned long tmr;

    tmr=0x00000;

    return tmr;
}
#endif

void LCD_initial(void)
{
    // Configure multi-function pin for LCD interface
    //GPG6 (CLK), GPG7 (HSYNC)
    outpw(REG_SYS_GPG_MFPL, (inpw(REG_SYS_GPG_MFPL)& ~0xFF000000) | 0x22000000);
    //GPG8 (VSYNC), GPG9 (DEN)
    outpw(REG_SYS_GPG_MFPH, (inpw(REG_SYS_GPG_MFPH)& ~0xFF) | 0x22);

    //DATA pin
    //GPA0 ~ GPA7 (DATA0~7)
    outpw(REG_SYS_GPA_MFPL, 0x22222222);
    //GPA8 ~ GPA15 (DATA8~15)
    outpw(REG_SYS_GPA_MFPH, 0x22222222);
#ifdef _PANEL_FW070TFT_24BPP_
    //GPD8 ~ GPD15 (DATA16~23)
    outpw(REG_SYS_GPD_MFPH, 0x22222222);
#endif

#ifdef _PANEL_E50A2V1_16BPP_
    // LCD clock is selected from UPLL and divide to 20MHz
    outpw(REG_CLK_DIVCTL1, (inpw(REG_CLK_DIVCTL1) & ~0xff1f) | 0xe18);
#endif
#ifdef _PANEL_FW070TFT_24BPP_
    // LCD clock is selected from UPLL and divide to 30MHz
    outpw(REG_CLK_DIVCTL1, (inpw(REG_CLK_DIVCTL1) & ~0xff1f) | 0x918);
#endif

#ifdef _PANEL_E50A2V1_16BPP_
    // Init LCD interface for E50A2V1 LCD module
    vpostLCMInit(DIS_PANEL_E50A2V1);
#endif
#ifdef _PANEL_FW070TFT_24BPP_
    // Init LCD interface for FW070TFT LCD module
    vpostLCMInit(DIS_PANEL_FW070TFT);
#endif
    // Set scale to 1:1
    vpostVAScalingCtrl(1, 0, 1, 0, VA_SCALE_INTERPOLATION);

    // Set display color depth
#ifdef _PANEL_E50A2V1_16BPP_
    vpostSetVASrc(VA_SRC_RGB565);
#endif
#ifdef _PANEL_FW070TFT_24BPP_
//    vpostSetVASrc(VA_SRC_RGB888);
    vpostSetVASrc(VA_SRC_RGB565);
#endif

    // Get pointer of video frame buffer
    // Note: before get pointer of frame buffer, must set display color depth first
    g_VAFrameBuf = vpostGetFrameBuffer();
    if(g_VAFrameBuf == NULL)
    {
        sysprintf("Get buffer error !!\n");
        while(1);
    }

#ifdef _PANEL_E50A2V1_16BPP_
    memset((void *)g_VAFrameBuf, 0, LCD_XSIZE*LCD_YSIZE*2);
#endif
#ifdef _PANEL_FW070TFT_24BPP_
//    memset((void *)g_VAFrameBuf, 0, LCD_XSIZE*LCD_YSIZE*4);
    memset((void *)g_VAFrameBuf, 0, LCD_XSIZE*LCD_YSIZE*2);
#endif

    // Start video
    //vpostVAStartTrigger();
}

extern volatile int OS_TimeMS;
volatile int g_enable_Touch;

void TMR0_IRQHandler(void)
{
    OS_TimeMS++;
}

void TMR0_IRQHandler_TouchTask(void)
{
    //sysprintf("g_enable_Touch=%d\n", g_enable_Touch);
    if ( g_enable_Touch == 1 )
    {
        //sysprintf("enable_Touch=%d\n", g_enable_Touch);
        TouchTask();
    }
}


#ifdef __USE_SD__
BYTE SD_Drv; // select SD0
unsigned int volatile gCardInit = 0;
void SDH_IRQHandler(void)
{
    unsigned int volatile isr;

    // FMI data abort interrupt
    if (inpw(REG_SDH_GINTSTS) & SDH_GINTSTS_DTAIF_Msk)
    {
        /* ResetAllEngine() */
        outpw(REG_SDH_GCTL, inpw(REG_SDH_GCTL) | SDH_GCTL_GCTLRST_Msk);
        outpw(REG_SDH_GINTSTS, SDH_GINTSTS_DTAIF_Msk);
    }

    //----- SD interrupt status
    isr = inpw(REG_SDH_INTSTS);
    if (isr & SDH_INTSTS_BLKDIF_Msk)        // block down
    {
        _sd_SDDataReady = TRUE;
        outpw(REG_SDH_INTSTS, SDH_INTSTS_BLKDIF_Msk);
    }

    if (isr & SDH_INTSTS_CDIF0_Msk)   // port 0 card detect
    {
        //----- SD interrupt status
        // it is work to delay 50 times for SD_CLK = 200KHz
        {
            volatile int i;         // delay 30 fail, 50 OK
            for (i=0; i<0x500; i++);   // delay to make sure got updated value from REG_SDISR.
            isr = inpw(REG_SDH_INTSTS);
        }

#ifdef _USE_DAT3_DETECT_
        if (!(isr & SDH_INTSTS_CDSTS0_Msk))
        {
            SD0.IsCardInsert = FALSE;
            sysprintf("\nCard Remove!\n");
            SD_Close_Disk(0);
        }
        else
        {
            SD_Open_Disk(SD_PORT0 | CardDetect_From_DAT3);
        }
#else
        if (isr & SDH_INTSTS_CDSTS0_Msk)
        {
            SD0.IsCardInsert = FALSE;   // SDISR_CD_Card = 1 means card remove for GPIO mode
            gCardInit = 0;
            sysprintf("\nCard Remove!\n");
            SD_Close_Disk(0);
        }
        else
        {
            gCardInit = 1;
            //SD_Open_Disk(SD_PORT0 | CardDetect_From_GPIO);
        }
#endif

        outpw(REG_SDH_INTSTS, SDH_INTSTS_CDIF0_Msk);
    }

    if (isr & SDH_INTSTS_CDIF1_Msk)   // port 1 card detect
    {
        //----- SD interrupt status
        // it is work to delay 50 times for SD_CLK = 200KHz
        {
            volatile int i;         // delay 30 fail, 50 OK
            for (i=0; i<0x500; i++);   // delay to make sure got updated value from REG_SDISR.
            isr = inpw(REG_SDH_INTSTS);
        }

#ifdef _USE_DAT3_DETECT_
        if (!(isr & SDH_INTSTS_CDSTS1_Msk))
        {
            SD0.IsCardInsert = FALSE;
            sysprintf("\nCard Remove!\n");
            SD_Close_Disk(1);
        }
        else
        {
            SD_Open_Disk(SD_PORT1 | CardDetect_From_DAT3);
        }
#else
        if (isr & SDH_INTSTS_CDSTS1_Msk)
        {
            SD0.IsCardInsert = FALSE;   // SDISR_CD_Card = 1 means card remove for GPIO mode
            sysprintf("\nCard Remove!\n");
            SD_Close_Disk(1);
        }
        else
        {
            SD_Open_Disk(SD_PORT1 | CardDetect_From_GPIO);
        }
#endif

        outpw(REG_SDH_INTSTS, SDH_INTSTS_CDIF1_Msk);
    }

    // CRC error interrupt
    if (isr & SDH_INTSTS_CRCIF_Msk)
    {
        if (!(isr & SDH_INTSTS_CRC16_Msk))
        {
            //sysprintf("***** ISR sdioIntHandler(): CRC_16 error !\n");
            // handle CRC error
        }
        else if (!(isr & SDH_INTSTS_CRC7_Msk))
        {
            extern unsigned int _sd_uR3_CMD;
            if (! _sd_uR3_CMD)
            {
                //sysprintf("***** ISR sdioIntHandler(): CRC_7 error !\n");
                // handle CRC error
            }
        }
        outpw(REG_SDH_INTSTS, SDH_INTSTS_CRCIF_Msk);      // clear interrupt flag
    }
}
#endif

void SYS_Init(void)
{
#ifdef __USE_SD__
    /* enable SDH */
    outpw(REG_CLK_HCLKEN, inpw(REG_CLK_HCLKEN) | 0x40000000);

    /* select multi-function-pin */
    /* SD Port 0 -> PD0~7 */
    outpw(REG_SYS_GPD_MFPL, 0x66666666);
    SD_Drv = 0;
#endif

#if 0 // port 1
    /* initial SD1 pin -> PI5~10, 12~13 */
    outpw(REG_SYS_GPI_MFPL, inpw(REG_SYS_GPI_MFPL) & ~0xfff00000 | 0x44400000);
    outpw(REG_SYS_GPI_MFPH, inpw(REG_SYS_GPI_MFPH) & ~0x00ff0fff | 0x00440444);
    SD_Drv = 1;
#endif

}

void UART_Init(void)
{
    int retval;

    /* configure UART */
    param.uFreq = 12000000;
    param.uBaudRate = 115200;
    param.ucUartNo = UART0;
    param.ucDataBits = DATA_BITS_8;
    param.ucStopBits = STOP_BITS_1;
    param.ucParity = PARITY_NONE;
    param.ucRxTriggerLevel = UART_FCR_RFITL_8BYTES;
    retval = uartOpen(&param);
    if(retval != 0)
    {
        uartprintf("Open UART error!\n");
        return;
    }
    /* set RX interrupt mode */
    retval = uartIoctl(param.ucUartNo, UART_IOC_SETRXMODE, UARTINTMODE, 0);
    if (retval != 0)
    {
        uartprintf("Set RX interrupt mode fail!\n");
        return;
    }

}

int main(void)
{
#if 0
#ifdef __USE_SD__
    FRESULT     res;
#else
    uint16_t u16ID;
#endif
#endif

    *((volatile unsigned int *)REG_AIC_MDCR)=0xFFFFFFFF;  // disable all interrupt channel
    *((volatile unsigned int *)REG_AIC_MDCRH)=0xFFFFFFFF;  // disable all interrupt channel
    *(volatile unsigned int *)(CLK_BA+0x18) |= (1<<16); /* Enable UART0 */
    *(volatile unsigned int *)(CLK_BA+0x18) |= (1<<3); /* Enable GPIO */
    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();
    UART_Init();
#if 0
//#if GUI_SUPPORT_TOUCH
    g_enable_Touch = 0;
#endif

    OS_TimeMS = 0;

    SYS_Init();

    sysSetTimerReferenceClock(TIMER0, 12000000);
    sysStartTimer(TIMER0, 1000, PERIODIC_MODE);         /* 1000 ticks/per sec ==> 1tick/1ms */
    sysSetTimerEvent(TIMER0,  1, (PVOID)TMR0_IRQHandler);           /*  1 tick  per call back */

#ifdef __USE_SD__
    sysInstallISR(HIGH_LEVEL_SENSITIVE|IRQ_LEVEL_1, SDH_IRQn, (PVOID)SDH_IRQHandler);
    sysEnableInterrupt(SDH_IRQn);
#endif

    sysSetLocalInterrupt(ENABLE_IRQ);

    sysprintf("+-------------------------------------------------+\n");
    sysprintf("|            N9H30 Meter NonOS Code               |\n");
    sysprintf("+-------------------------------------------------+\n\n");
    LCD_initial();

    MainTask();
    return 0;
}
