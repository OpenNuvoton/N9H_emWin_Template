/*********************************************************************
*                 SEGGER Software GmbH                               *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2018  SEGGER Microcontroller GmbH                *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.48 - Graphical user interface for embedded applications **
All  Intellectual Property rights in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product. This file may
only be used in accordance with the following terms:

The  software has  been licensed by SEGGER Software GmbH to Nuvoton Technology Corporationat the address: No. 4, Creation Rd. III, Hsinchu Science Park, Taiwan
for the purposes  of  creating  libraries  for its
Arm Cortex-M and  Arm9 32-bit microcontrollers, commercialized and distributed by Nuvoton Technology Corporation
under  the terms and conditions  of  an  End  User
License  Agreement  supplied  with  the libraries.
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
Licensing information
Licensor:                 SEGGER Software GmbH
Licensed to:              Nuvoton Technology Corporation, No. 4, Creation Rd. III, Hsinchu Science Park, 30077 Hsinchu City, Taiwan
Licensed SEGGER software: emWin
License number:           GUI-00735
License model:            emWin License Agreement, signed February 27, 2018
Licensed platform:        Cortex-M and ARM9 32-bit series microcontroller designed and manufactured by or for Nuvoton Technology Corporation
----------------------------------------------------------------------
Support and Update Agreement (SUA)
SUA period:               2018-03-26 - 2019-03-27
Contact to extend SUA:    sales@segger.com
----------------------------------------------------------------------
File        : Thermostat.c
Purpose     : Thermostat HMI and Modbus Master via RS485
Requirements: WindowManager - (x)
              MemoryDevices - (x)
              AntiAliasing  - (x)
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )
----------------------------------------------------------------------
*/

/***************************************************************************
 *                                                                                                               *
 * Copyright (c) Nuvoton Technolog. All rights reserved.                                    *
 *                                                                                                               *
 ***************************************************************************/

/*********************************************************************
*
*       Includes
*
**********************************************************************
*/
#include "DIALOG.h"

#ifdef WIN32
#else
#include "N9H20.h"
#endif

#include  "def.h"

#if NVT_WIFI   /* WiFi */
#include "protocol.h"
#include "mcu_api.h"
#include "wifi.h"
#endif

static const char s_ach2ThermostatVersion[] = {'V', '2', '.', '0', 0};


//extern GUI_CONST_STORAGE GUI_BITMAP bmlogo;

//extern GUI_CONST_STORAGE GUI_BITMAP bmmenu1;
//extern GUI_CONST_STORAGE GUI_BITMAP bmmenu1up1;
//extern GUI_CONST_STORAGE GUI_BITMAP bmmenu1down1;
//extern GUI_CONST_STORAGE GUI_BITMAP bmmenu1back1;
//extern GUI_CONST_STORAGE GUI_BITMAP bmdehumidify1;
//extern GUI_CONST_STORAGE GUI_BITMAP bmdehumidify1disable1;
//extern GUI_CONST_STORAGE GUI_BITMAP bmfan1;
//extern GUI_CONST_STORAGE GUI_BITMAP bmfan1disable1;
//extern GUI_CONST_STORAGE GUI_BITMAP bmcool1;
//extern GUI_CONST_STORAGE GUI_BITMAP bmcool1disable1;
//extern GUI_CONST_STORAGE GUI_BITMAP bmheat1;
//extern GUI_CONST_STORAGE GUI_BITMAP bmheat1disable1;

//extern GUI_CONST_STORAGE GUI_BITMAP bmmenu2;
//extern GUI_CONST_STORAGE GUI_BITMAP bmmenu2up1;
//extern GUI_CONST_STORAGE GUI_BITMAP bmmenu2down1;
//extern GUI_CONST_STORAGE GUI_BITMAP bmmenu2back1;

//extern GUI_CONST_STORAGE GUI_BITMAP bmmenu3;
//extern GUI_CONST_STORAGE GUI_BITMAP bmmenu3back1;

//extern GUI_CONST_STORAGE GUI_BITMAP bmonoffselect1;
//extern GUI_CONST_STORAGE GUI_BITMAP bmon1;
//extern GUI_CONST_STORAGE GUI_BITMAP bmoff1;

static UINT8 s_au8BMP[700 * 1024] __attribute__((aligned(32)));
static UINT8 s_au8BMP1[700 * 1024] __attribute__((aligned(32)));
static UINT8 s_au8BMP2[700 * 1024] __attribute__((aligned(32)));
static UINT8 s_au8BMP3[700 * 1024] __attribute__((aligned(32)));

/*********************************************************************
*
*       _aMenu1
*/
#if 0
const struct MENU1_ITEM
{
    const GUI_BITMAP * pBm;
    const char * pText;
} _aMenu1[] =
{
    { &bmdehumidify1, "Dehumidify" },
    { &bmfan1, "Fan" },
    { &bmcool1, "Cool" },
    { &bmheat1, "Heat" }
};
#endif

char         buf[30];

#ifdef WIN32
#else

RTC_TIME_DATA_T g_sCurTime;
RTC_TIME_DATA_T g_sCurTime2;

volatile static int s_i32RTCFlag;

VOID RTC_AlarmISR(VOID)
{
//    sysprintf("   Alarm!!\n");

    s_i32RTCFlag = 1; 
}

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

    g_sCurTime.u32Year         = 2021;
    g_sCurTime.u32cMonth       = 2;
    g_sCurTime.u32cDay         = 25;
    g_sCurTime.u32cHour        = 17;
    g_sCurTime.u32cMinute      = 30;
    g_sCurTime.u32cSecond      = 50;
    g_sCurTime.u32cDayOfWeek   = 3;
    g_sCurTime.u8cClockDisplay = RTC_CLOCK_24;
    RTC_Write(RTC_CURRENT_TIME, &g_sCurTime);

    /* Set Tick setting */
    RTC_Ioctl(0,RTC_IOC_SET_TICK_MODE, (UINT32)&sTick,0);

    /* Enable RTC Tick Interrupt and install tick call back function */
    RTC_Ioctl(0,RTC_IOC_ENABLE_INT, (UINT32)RTC_TICK_INT,0);

}

#if NVT_WIFI   /* WiFi */
volatile UINT8 wifi_connected = 0;
static int g_RTCDateflag;

void NVT_UpdateDate(unsigned char time[])
{
    if (g_RTCDateflag == 0)
    {
//        sysprintf("### start ###\n");
//        sysprintf("20%d/%02d/%02d %02d:%02d:%02d\n",time[1], time[2], time[3], time[4], time[5], time[6]);
//        sysprintf("%d %d %d %d %d %d\n",time[1], time[2], time[3], time[4], time[5], time[6]);
        g_RTCDateflag = 1;
        g_sCurTime2.u32Year         = time[1] + 2000;
        g_sCurTime2.u32cMonth       = time[2];
        g_sCurTime2.u32cDay         = time[3];
        g_sCurTime2.u32cHour        = time[4];
        g_sCurTime2.u32cMinute      = time[5];
        g_sCurTime2.u32cSecond      = time[6];
        g_sCurTime2.u32cDayOfWeek   = time[7] - 1;
        g_sCurTime2.u8cClockDisplay = RTC_CLOCK_24;
        RTC_Write(RTC_CURRENT_TIME, &g_sCurTime2);
//        sysprintf("### end ###\n");
    }
}
#endif

#endif

#if 0
static char _apStrings[] =
{
    ".0\xe2\x84\x83",
};
#else
static char s_achT1[] =
{
    '2', '3', 0
};
static char s_achT2[] =
{
    '.', '5', 0xB0, 'c', 0
};
#endif

static TEXT_Handle s_hText1;
static TEXT_Handle s_hText2;

//static int s_iT1;
//static int s_iT2;

extern int c_div, c_rem, f_div, f_rem;
extern short c_temp, f_temp;

int g_i32SetTemp = 235;
char g_au8SetTemp[32];

int g_ModbusTempIsEnabled;
int g_ModbusLedIsEnabled;
int g_ModbusMeterIsEnabled;

#define ADDR_READSENSOR             0x2000
#define ADDR_SENSORDEVICE           0x2002
#define ADDR_LED_1                      0x2003

// function from
extern  void begin(uint8_t);
extern  uint8_t  readInputRegisters(uint16_t, uint8_t);
extern  uint8_t  writeSingleRegister(uint16_t, uint16_t);
extern  uint8_t readHoldingRegisters(uint16_t, uint16_t);
extern  uint8_t calibrateRegister(uint16_t, uint8_t);
extern  uint8_t resetenergyRegister(uint16_t, uint8_t);
extern  uint8_t  writeSingleCoil(uint16_t, uint8_t);
extern  uint8_t readCoils(uint16_t, uint16_t);

uint8_t OpenDevice(uint8_t id)
{
    begin(id);
    return( writeSingleCoil(ADDR_SENSORDEVICE, 1));
}

uint8_t ReadDevice(uint8_t id)
{
    begin(id);
    return(readInputRegisters(ADDR_READSENSOR, 2));
}

uint8_t DeviceLedOn(uint8_t id)
{
    begin(id);
    return(writeSingleCoil(ADDR_LED_1,1));
}


uint8_t DeviceLedOff(uint8_t id)
{
    begin(id);
    return(writeSingleCoil(ADDR_LED_1,0));
}

//extern UARTDEV_T    RS485Uart;
//extern UARTDEV_T* pUartDevISR;
int g_ModbusIsEnabled;

void _BMP_Decode(char * szFileName, int x, int y)
{
    int hFile, wbytes, i32FileSize;
    char szFileName2[80];

    fsAsciiToUnicode(szFileName, szFileName2, TRUE);
    hFile = fsOpenFile(szFileName2, szFileName, O_RDONLY | O_FSEEK);
    i32FileSize = fsGetFileSize(hFile);
//    sysprintf("1 0x%x, %d\n", hFile, i32FileSize);
    fsReadFile(hFile, (UINT8 *)s_au8BMP, i32FileSize, &wbytes);
    fsCloseFile(hFile);
//    GUI_BMP_Draw(s_au8BMP, x, y);
    GUI_DrawStreamedBitmapAuto(s_au8BMP, x, y);
//    GUI_JPEG_Draw(s_au8BMP, i32FileSize, x, y);
}

U32 _BMP_Decode2(char * szFileName, UINT8 * pu8BMP)
{
    int hFile, wbytes, i32FileSize;
    char szFileName2[80];

    fsAsciiToUnicode(szFileName, szFileName2, TRUE);
    hFile = fsOpenFile(szFileName2, szFileName, O_RDONLY | O_FSEEK);
    i32FileSize = fsGetFileSize(hFile);
//    sysprintf("2 0x%x, %d\n", hFile, i32FileSize);
    fsReadFile(hFile, (UINT8 *)pu8BMP, i32FileSize, &wbytes);
    fsCloseFile(hFile);

    return i32FileSize;
}

#if 0
static char _acBuffer[100 * 1024] __attribute__((aligned(32)));
static UINT8 _auOutBuffer[480 * 480 * 2] __attribute__((aligned(32)));

#include "GUI_Private.h"

/*********************************************************************
*
*       _DrawBitmap
*/
static void _DrawBitmap(int x, int y, void const * p, int xSize, int ySize, int BytesPerLine, int BitsPerPixel) {
  #if (GUI_WINSUPPORT)
    GUI_RECT r;
  #endif
  #if (GUI_WINSUPPORT)
    WM_ADDORG(x,y);
    r.x1 = (r.x0 = x) + xSize-1;
    r.y1 = (r.y0 = y) + ySize-1;
    WM_ITERATE_START(&r) {
  #endif
//        sysprintf("*r.x0=%d, r.y0=%d, r.x1=%d, r.y1=%d\n", r.x0, r.y0, r.x1, r.y1);
  LCD_DrawBitmap(x, y, xSize, ySize, 1, 1, BitsPerPixel, BytesPerLine, p, NULL);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
}

/*********************************************************************
*
*       JPEG_X_Draw
*/
int JPEG_X_Draw(GUI_GET_DATA_FUNC * pfGetData, void * p, int x0, int y0) {
    GUI_JPEG_INFO Info;
    U8                * pInBuffer;
//    U32                 Off;

//    GUI_HMEM            hOutBuffer;
//    U32                 OutBufferSize;
//    U8                * pOutBuffer;

    /*Off              = */pfGetData(p, (const U8 **)&pInBuffer, (100 * 1024), 0);

//    OutBufferSize = 800 * 480 * 2;
//    hOutBuffer = GUI_ALLOC_AllocNoInit(OutBufferSize);
//    pOutBuffer = GUI_ALLOC_LockH(hOutBuffer);
    //
    // runtime to get jpeg width and height will decrease performance
    //
    GUI_JPEG_GetInfo(pInBuffer, (100 * 1024), &Info);
    //
    // unmark sysprintf here to increase performance
    // run sysprintf here is just for debugging and will decrease performance
    //
//    sysprintf("x=%d, y=%d\n", Info.XSize, Info.YSize);
//    sysprintf("x0=%d, y0=%d\n", x0, y0);
    memcpy(_acBuffer, pInBuffer, (100 * 1024));
    jpegOpen();
    jpegInit();
    jpegIoctl(JPEG_IOCTL_SET_BITSTREAM_ADDR, (((unsigned int)_acBuffer) | BIT31), 0);
    jpegIoctl(JPEG_IOCTL_SET_DECODE_MODE, JPEG_DEC_PRIMARY_PACKET_RGB565, 0);
    jpegIoctl(JPEG_IOCTL_SET_DECODE_DOWNSCALE, Info.YSize, Info.XSize);
    jpegIoctl(JPEG_IOCTL_SET_DECODE_STRIDE, Info.XSize, 0);
    jpegIoctl(JPEG_IOCTL_SET_YADDR, (((unsigned int)_auOutBuffer) | BIT31), 0);
    jpegIoctl(JPEG_IOCTL_DECODE_TRIGGER, 0, 0);
    jpegWait();
    jpegClose();

    //
    // Draw buffer
    //
    _DrawBitmap(x0, y0, (void const *)_auOutBuffer, Info.XSize, Info.YSize, (Info.XSize * 2), 16);

//    GUI_ALLOC_UnlockH((void **)&pOutBuffer);
//    GUI_ALLOC_Free(hOutBuffer);

    return 0;
}
#endif

WM_HWIN g_hWinMenu;

volatile int g_moveflag;
volatile int g_movexpos;

#if NVT_WIFI   /* WiFi */
#define WM_MENU2_ONOFF          (WM_USER + 0x0)
#define WM_MENU2_TEMP1          (WM_USER + 0x1)
#define WM_MENU2_TEMP1_BACK     (WM_USER + 0x2)
#define WM_MENU2_TEMP1_MODBUS   (WM_USER + 0x3)

#define WM_MENU1_MODE1          (WM_USER + 0x4)
#define WM_MENU1_MODE1_BACK     (WM_USER + 0x5)
#define WM_MENU1_MODE1_MODE1    (WM_USER + 0x6)
#define WM_MENU1_MODE1_MODE2    (WM_USER + 0x7)
#define WM_MENU1_MODE1_MODE3    (WM_USER + 0x8)

#define WM_MENU3_DATE1          (WM_USER + 0x9)
#define WM_MENU3_DATE1_BACK     (WM_USER + 10)
#define WM_MENU3_DATE1_DATE1    (WM_USER + 11)
#define WM_MENU3_DATE1_DATE2    (WM_USER + 12)
#define WM_MENU3_DATE1_DATE3    (WM_USER + 13)

#define WM_MENU123    (WM_USER + 14)

#define WM_MENU1_DEHUMIDIFY     (WM_USER + 15)
#define WM_MENU1_FAN            (WM_USER + 16)
#define WM_MENU1_COOL           (WM_USER + 17)
#define WM_MENU1_HEAT           (WM_USER + 18)
#define WM_MENU1_BACK           (WM_USER + 19)
#define WM_MENU1_ONOFF_LEVEL    (WM_USER + 20)
#define WM_MENU1_INCREASE_LEVEL (WM_USER + 21)
#define WM_MENU1_DECREASE_LEVEL (WM_USER + 22)

extern unsigned int menu2settemp_vflag;
extern unsigned int menu2settemp_v;
extern unsigned int menu2onoff_vflag;
extern unsigned int menu2onoff_v;

extern unsigned int menu2temp1_vflag;
extern unsigned int menu2temp1_v;
extern unsigned int menu2temp1modbus_vflag;
extern unsigned int menu2temp1modbus_v;
extern unsigned int menu2temp1backlight_v;

extern unsigned int menu1mode1_vflag;
extern unsigned int menu1mode1_v;
extern unsigned int menu1mode1mode1_vflag;
extern unsigned int menu1mode1mode1_v;
extern unsigned int menu1mode1mode2_vflag;
extern unsigned int menu1mode1mode2_v;
extern unsigned int menu1mode1mode3_vflag;
extern unsigned int menu1mode1mode3_v;

extern unsigned int menu3date1_vflag;
extern unsigned int menu3date1_v;
extern unsigned int menu3date1date1_vflag;
extern unsigned int menu3date1date1_v;
extern unsigned int menu3date1date2_vflag;
extern unsigned int menu3date1date2_v;
extern unsigned int menu3date1date3_vflag;
extern unsigned int menu3date1date3_v;

extern unsigned int menu123_vflag;
extern unsigned int menu123_v;
unsigned int menu_v = 2;

extern unsigned int menu1dehumidify_vflag;
extern unsigned int menu1dehumidify_v;
extern unsigned int menu1fan_vflag;
extern unsigned int menu1fan_v;
extern unsigned int menu1cool_vflag;
extern unsigned int menu1cool_v;
extern unsigned int menu1heat_vflag;
extern unsigned int menu1heat_v;
extern unsigned int menu1onofflevel_vflag;
extern unsigned int menu1onofflevel_v;
extern unsigned int menu1increaselevel_vflag;
extern unsigned int menu1increaselevel_v;
extern unsigned int menu1decreaselevel_vflag;
extern unsigned int menu1decreaselevel_v;

#endif

WM_HWIN s_hCal;
static CALENDAR_DATE s_calDate;

static int s_Menu3Date1Flag;
static int s_Menu3Date2Flag;
static int s_Menu3Date3Flag;

static void _cbHeading(WM_MESSAGE * pMsg)
{
    int xSize;
//    const GUI_BITMAP * pBm;
    WM_HWIN hWin;
    static GUI_RECT s_Rect;

    hWin = pMsg->hWin;
    switch (pMsg->MsgId)
    {
    case WM_CREATE:
        s_Rect.x0 = 70;
        s_Rect.y0 = 0;
        s_Rect.x1 = s_Rect.x0 + 480 - 70 - 1;
        s_Rect.y1 = s_Rect.y0 + 35 - 1;
        //
        // Create timer
        //
        WM_CreateTimer(hWin, 0, 1000, 0);
        break;
    case WM_TIMER:

#ifdef WIN32
#else
        /* Get the current time */
        RTC_Read(RTC_CURRENT_TIME, &g_sCurTime);
#endif

        if (s_i32RTCFlag == 1)
        {
            s_i32RTCFlag = 0;
            s_Menu3Date1Flag = 0;
            GUI_MessageBox("Time's up!", "Test Schedule", GUI_MESSAGEBOX_CF_MOVEABLE);
        }
        WM_InvalidateRect(hWin, &s_Rect);
        WM_RestartTimer(pMsg->Data.v, 0);

#ifdef WIN32
#else

        sprintf(buf,"%d/%02d/%02d %02d:%02d:%02d\n", g_sCurTime.u32Year, g_sCurTime.u32cMonth, g_sCurTime.u32cDay,g_sCurTime.u32cHour,g_sCurTime.u32cMinute,g_sCurTime.u32cSecond);

#endif

//        sysprintf("%s\n", buf);
        break;
    case WM_PAINT:
        //
        // Get window dimension
        //
        xSize = WM_GetWindowSizeX(hWin);
        //
        // Draw logo
        //
//        pBm = &bmlogo;
        GUI_SetColor(GUI_MAKE_COLOR(0xf8f8f8));
        GUI_FillRect(0, 0, xSize - 1, 57);
        GUI_SetColor(GUI_MAKE_COLOR(0xb2b2b2));
        GUI_FillRect(0, 58, xSize - 1, 59);
//        GUI_DrawBitmap(pBm, 0, 0);
        _BMP_Decode("C:\\Application\\Heading\\logo.dta", 0, 0);
        GUI_SetFont(GUI_FONT_32B_1);
        GUI_SetColor(GUI_BLACK);
        GUI_SetTextMode(GUI_TM_TRANS);
        GUI_DispStringHCenterAt(buf, xSize / 2, 10);

#if NVT_WIFI   /* WiFi */
        GUI_DispDecAt(wifi_connected, 450, 10, 1);
#endif

        if (g_ModbusIsEnabled == 1 && g_ModbusTempIsEnabled == 2)
        {
            if (ReadDevice(2) == 0)
                g_ModbusTempIsEnabled = 2;
            else
                g_ModbusTempIsEnabled = 0;

            sprintf(s_achT1, "%02d", c_div);
            sprintf(s_achT2, ".%01d", c_rem);
            s_achT2[2] = 0xB0;
            s_achT2[3] = 'c';
            s_achT2[4] = 0;
            TEXT_SetTextColor(s_hText1, GUI_BLACK);
            TEXT_SetTextColor(s_hText2, GUI_BLACK);
            TEXT_SetText(s_hText1, s_achT1);
            TEXT_SetText(s_hText2, s_achT2);
#if NVT_WIFI   /* WiFi */
            mcu_dp_value_update(DPID_TEMP_CURRENT,c_temp/10);
#endif
        }

        if (g_ModbusIsEnabled == 1 && g_ModbusTempIsEnabled == 1)
        {
            if (OpenDevice(2) == 0)
                g_ModbusTempIsEnabled = 2;
            else
                g_ModbusTempIsEnabled = 0;
        }
#if NVT_WIFI   /* WiFi */
        if (g_hWinMenu == 0)
//            sysprintf("### invalid ###\n");
            ;
        else
        {
            if (menu2onoff_vflag == 1)
            {
                menu2onoff_vflag = 2;
                WM_SendMessageNoPara(g_hWinMenu, WM_MENU2_ONOFF);
            }

            if (menu2temp1_vflag == 1)
            {
                menu2temp1_vflag = 2;
                WM_SendMessageNoPara(g_hWinMenu, WM_MENU2_TEMP1);
            }

            if (menu1mode1_vflag == 1)
            {
                menu1mode1_vflag = 2;
                WM_SendMessageNoPara(g_hWinMenu, WM_MENU1_MODE1);
            }

            if (menu3date1_vflag == 1)
            {
                menu3date1_vflag = 2;
                WM_SendMessageNoPara(g_hWinMenu, WM_MENU3_DATE1);
            }

            if (menu123_vflag == 1)
            {
                menu123_vflag = 2;
                WM_SendMessageNoPara(g_hWinMenu, WM_MENU123);
            }

            if (g_moveflag == 1)
            {
                g_moveflag = 0;
                if (g_movexpos == 0)
                    menu_v = 1;
                else if ((g_movexpos >= -500) && (g_movexpos <= -400)) /* FIXME */
                    menu_v = 2;
                else
                    menu_v = 3;
                menu123_v = menu_v;
                mcu_dp_value_update(DPID_MENU,menu_v);
            }

            if (menu1dehumidify_vflag == 1)
            {
                menu1dehumidify_vflag = 2;
                WM_SendMessageNoPara(g_hWinMenu, WM_MENU1_DEHUMIDIFY);
            }

            if (menu1fan_vflag == 1)
            {
                menu1fan_vflag = 2;
                WM_SendMessageNoPara(g_hWinMenu, WM_MENU1_FAN);
            }

            if (menu1cool_vflag == 1)
            {
                menu1cool_vflag = 2;
                WM_SendMessageNoPara(g_hWinMenu, WM_MENU1_COOL);
            }

            if (menu1heat_vflag == 1)
            {
                menu1heat_vflag = 2;
                WM_SendMessageNoPara(g_hWinMenu, WM_MENU1_HEAT);
            }

            if (menu2settemp_vflag == 1)
            {
                menu2settemp_vflag = 0;
                g_i32SetTemp = menu2settemp_v;
            }
        }
#endif
        break;
    default:
        WM_DefaultProc(pMsg);
    }
}

static int s_backFlag;
static int s_effectFlag0;
static int s_effectFlag1;
static int s_effectFlag2;
static int s_effectFlag3;
static int s_effectFlag0Toggle;
static int s_effectFlag1Toggle;
static int s_effectFlag2Toggle;
static int s_effectFlag3Toggle;
static int s_ledFlag;

static int _ButtonSkinMenu1(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo)
{
    int Index;
    WM_HWIN hWin;

    hWin = pDrawItemInfo->hWin;
    BUTTON_GetUserData(hWin, &Index, sizeof(Index));

    switch (pDrawItemInfo->Cmd)
    {
    case WIDGET_ITEM_DRAW_BACKGROUND:
        if (Index == 1 || Index == 2)   /* menu1 increase / decrease level */
        {
            GUI_SetBkColor(0x0038393B);
//        GUI_SetBkColor(GUI_YELLOW);
            GUI_Clear();
        }
        break;
    case WIDGET_ITEM_DRAW_BITMAP:
        if (Index == 1) /* menu1 increase level */
//            GUI_DrawBitmap(&bmmenu1up1, (WM_GetXSize(hWin)-bmmenu1up1.XSize)/2, (WM_GetYSize(hWin)-bmmenu1up1.YSize)/2);
            _BMP_Decode("C:\\Application\\Menu1\\menu1up1.dta", (WM_GetXSize(hWin)-52)/2, (WM_GetYSize(hWin)-52)/2);
        else if (Index == 2)    /* menu1 decrease level */
//            GUI_DrawBitmap(&bmmenu1down1, (WM_GetXSize(hWin)-bmmenu1down1.XSize)/2, (WM_GetYSize(hWin)-bmmenu1down1.YSize)/2);
            _BMP_Decode("C:\\Application\\Menu1\\menu1down1.dta", (WM_GetXSize(hWin)-52)/2, (WM_GetYSize(hWin)-52)/2);
        break;
    }
    return 0;
}

extern int InitEffect(void);
extern int Effect1(unsigned int i32Effect1);
extern int Effect2(unsigned int i32Effect2);
extern int Effect3(unsigned int i32Effect3a, unsigned int i32Effect3b, unsigned int i32Effect3c);

static void _cbSelectedMenu1(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hButton;
    int Index, i;
    int Id, NCode;
//    const GUI_BITMAP * pBm;
    GUI_RECT Rect;
    //
    // input device state
    //
    GUI_PID_STATE CurrentState;
#if NVT_WIFI   /* WiFi */
    static GUI_PID_STATE CurrentState2;
#endif
    hWin = pMsg->hWin;
    switch (pMsg->MsgId)
    {
#if NVT_WIFI   /* WiFi */
    case WM_MENU1_BACK:
        CurrentState2.x = 0 + 10;
        CurrentState2.y = 0 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
    case WM_MENU1_ONOFF_LEVEL:
        CurrentState2.x = 142 + 10;
        CurrentState2.y = 142 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
    case WM_MENU1_INCREASE_LEVEL:
        CurrentState2.x = 369 + 10;
        CurrentState2.y = 160 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
    case WM_MENU1_DECREASE_LEVEL:
        CurrentState2.x = 10 + 10;
        CurrentState2.y = 160 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
#endif
    case WM_TIMER:
        WM_DeleteTimer(pMsg->Data.v);
        //
        // get current input device state
        //
        GUI_PID_GetState(&CurrentState);
        //
        // if pressed for 500ms, then start repeated
        //
        if (CurrentState.Pressed == 1)
        {
            //
            // update input device state to simulate pressing key
            //
            CurrentState.Pressed = 0;
            GUI_PID_StoreState(&CurrentState);
            CurrentState.Pressed = 1;
            GUI_PID_StoreState(&CurrentState);
        }
        break;
    case WM_NOTIFY_PARENT:
        Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
        NCode = pMsg->Data.v;                 // Notification code
        switch (NCode)
        {
        case WM_NOTIFICATION_RELEASED:
            break;
        case WM_NOTIFICATION_CLICKED:
            Rect.x0 = 117;
            Rect.y0 = 112;
            Rect.x1 = Rect.x0 + 246 - 1;
            Rect.y1 = Rect.y0 + 245 - 1;
            WM_InvalidateRect(hWin, &Rect);

            if (Id == GUI_ID_BUTTON0+0) /* menu1 back to menu1*/
            {
                s_backFlag = 1;
            }

            WM_GetUserData(hWin, &Index, sizeof(Index));

            if (Id == GUI_ID_BUTTON0+1) /* menu1 increase level */
            {
                if (Index == 0 && s_effectFlag0Toggle == 1) /* menu1 dehumidify */
                {
                    s_effectFlag0++;
                    if (s_effectFlag0 > 3)
                        s_effectFlag0 = 3;
#if NVT_WIFI   /* WiFi */
                    mcu_dp_value_update(DPID_MENU1_INCREASE_LEVEL,s_effectFlag0);
#endif
                }
                else if (Index == 1 && s_effectFlag1Toggle == 1)    /* menu1 fan */
                {
                    s_effectFlag1++;
                    if (s_effectFlag1 > 3)
                        s_effectFlag1 = 3;
#if NVT_WIFI   /* WiFi */
                    mcu_dp_value_update(DPID_MENU1_INCREASE_LEVEL,s_effectFlag1);
#endif
                }
                else if (Index == 2 && s_effectFlag2Toggle == 1)    /* menu1 cool */
                {
                    s_effectFlag2++;
                    if (s_effectFlag2 > 3)
                        s_effectFlag2 = 3;
#if NVT_WIFI   /* WiFi */
                    mcu_dp_value_update(DPID_MENU1_INCREASE_LEVEL,s_effectFlag2);
#endif
                }
                else if (Index == 3 && s_effectFlag3Toggle == 1)    /* menu1 heat */
                {
                    s_effectFlag3++;
                    if (s_effectFlag3 > 3)
                        s_effectFlag3 = 3;
#if NVT_WIFI   /* WiFi */
                    mcu_dp_value_update(DPID_MENU1_INCREASE_LEVEL,s_effectFlag3);
#endif
                }
            }

            if (Id == GUI_ID_BUTTON0+2) /* menu1 decrease level */
            {
                if (Index == 0 && s_effectFlag0Toggle == 1) /* menu1 dehumidify */
                {
                    s_effectFlag0--;
                    if (s_effectFlag0 < 0)
                        s_effectFlag0 = 0;
#if NVT_WIFI   /* WiFi */
                    mcu_dp_value_update(DPID_MENU1_DECREASE_LEVEL,s_effectFlag0);
#endif
                }
                else if (Index == 1 && s_effectFlag1Toggle == 1)    /* menu1 fan */
                {
                    s_effectFlag1--;
                    if (s_effectFlag1 < 0)
                        s_effectFlag1 = 0;
#if NVT_WIFI   /* WiFi */
                    mcu_dp_value_update(DPID_MENU1_DECREASE_LEVEL,s_effectFlag1);
#endif
                }
                else if (Index == 2 && s_effectFlag2Toggle == 1)    /* menu1 cool */
                {
                    s_effectFlag2--;
                    if (s_effectFlag2 < 0)
                        s_effectFlag2 = 0;
#if NVT_WIFI   /* WiFi */
                    mcu_dp_value_update(DPID_MENU1_DECREASE_LEVEL,s_effectFlag2);
#endif
                }
                else if (Index == 3 && s_effectFlag3Toggle == 1)    /* menu1 heat */
                {
                    s_effectFlag3--;
                    if (s_effectFlag3 < 0)
                        s_effectFlag3 = 0;
#if NVT_WIFI   /* WiFi */
                    mcu_dp_value_update(DPID_MENU1_DECREASE_LEVEL,s_effectFlag3);
#endif
                }
            }

            if (Id == (GUI_ID_BUTTON0+3))   /* menu1 onoff level */
            {
                if (Index == 0) /* menu1 dehumidify */
                {
                    s_effectFlag0Toggle = s_effectFlag0Toggle ^ 1;
                    if (s_effectFlag0Toggle == 0)
                    {
                        s_effectFlag0 = -1;
#if NVT_WIFI   /* WiFi */
                        mcu_dp_bool_update(DPID_MENU1_ONOFF_LEVEL,0);
#endif
                    }
                    else
                    {
                        s_effectFlag0 = 0;
#if NVT_WIFI   /* WiFi */
                        mcu_dp_bool_update(DPID_MENU1_ONOFF_LEVEL,1);
#endif
                    }
                }
                else if (Index == 1)    /* menu1 fan */
                {
                    s_effectFlag1Toggle = s_effectFlag1Toggle ^ 1;
                    if (s_effectFlag1Toggle == 0)
                    {
                        s_effectFlag1 = -1;
#if NVT_WIFI   /* WiFi */
                        mcu_dp_bool_update(DPID_MENU1_ONOFF_LEVEL,0);
#endif
                    }
                    else
                    {
                        s_effectFlag1 = 0;
#if NVT_WIFI   /* WiFi */
                        mcu_dp_bool_update(DPID_MENU1_ONOFF_LEVEL,1);
#endif
                    }
                }
                else if (Index == 2)    /* menu1 cool */
                {
                    s_effectFlag2Toggle = s_effectFlag2Toggle ^ 1;
                    if (s_effectFlag2Toggle == 0)
                    {
                        s_effectFlag2 = -1;
#if NVT_WIFI   /* WiFi */
                        mcu_dp_bool_update(DPID_MENU1_ONOFF_LEVEL,0);
#endif
                    }
                    else
                    {
                        s_effectFlag2 = 0;
#if NVT_WIFI   /* WiFi */
                        mcu_dp_bool_update(DPID_MENU1_ONOFF_LEVEL,1);
#endif
                    }
                }
                else if (Index == 3)    /* menu1 heat */
                {
                    s_effectFlag3Toggle = s_effectFlag3Toggle ^ 1;
                    if (s_effectFlag3Toggle == 0)
                    {
                        s_effectFlag3 = -1;
#if NVT_WIFI   /* WiFi */
                        mcu_dp_bool_update(DPID_MENU1_ONOFF_LEVEL,0);
#endif
                    }
                    else
                    {
                        s_effectFlag3 = 0;
#if NVT_WIFI   /* WiFi */
                        mcu_dp_bool_update(DPID_MENU1_ONOFF_LEVEL,1);
#endif
                    }
                }
            }

            if ((Id == GUI_ID_BUTTON0+1) || (Id == GUI_ID_BUTTON0+2))   /* menu1 increase / decrease level */
            {
                //
                // Create timer
                //
                WM_CreateTimer(hWin, 1, 500, 0);
            }
            break;
        }
        break;
    case WM_CREATE:
        i = 0;  /* menu1 back to menu1*/
        hButton = BUTTON_CreateUser(0, 0, 128+26, 120+25, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinMenu1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));

        i = 1;  /* menu1 increase level */
        hButton = BUTTON_CreateUser(369, 160, 99, 123, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinMenu1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));

        i = 2;  /* menu1 decrease level */
        hButton = BUTTON_CreateUser(10, 160, 99, 123, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinMenu1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));

        i = 3;  /* menu1 onoff level */
        hButton = BUTTON_CreateUser(142, 142, 190, 190, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinMenu1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));

#ifdef WIN32
#else
        InitEffect();
#endif

        break;
    case WM_PAINT:
        WM_GetUserData(hWin, &Index, sizeof(Index));

        if (Index == 0) /* menu1 dehumidify */
        {
//            pBm = _aMenu1[Index].pBm;
//            GUI_DrawBitmap(pBm, 0, 0);
            _BMP_Decode("C:\\Application\\Menu1\\dehumidify1.dta", 0, 0);
        }
        else if (Index == 1)    /* menu1 fan */
        {
            _BMP_Decode("C:\\Application\\Menu1\\fan1.dta", 0, 0);
        }
        else if (Index == 2)    /* menu1 cool */
        {
            _BMP_Decode("C:\\Application\\Menu1\\cool1.dta", 0, 0);
        }
        else if (Index == 3)    /* menu1 heat */
        {
            _BMP_Decode("C:\\Application\\Menu1\\heat1.dta", 0, 0);
        }

        if (Index == 0) /* menu1 dehumidify */
        {
            if (s_effectFlag0Toggle == 0)
                s_effectFlag0 = -1;
            switch (s_effectFlag0)
            {
            case -1:
//                GUI_DrawBitmap(&bmdehumidify1disable1, 142, 142);
                _BMP_Decode("C:\\Application\\Menu1\\dehumidify1disable1.dta", 142, 142);
                break;

#ifdef WIN32
#else
            case 3:
                Effect1(0x00F6C561);
                Effect2(6);
                Effect3(240, 240-4, 118);
            case 2:
                Effect1(0x00F9DB9E);
                Effect2(4);
                Effect3(240, 240-4, 110);
            case 1:
                Effect1(0x00FBE7C1);
                Effect2(2);
                Effect3(240, 240-4, 100);
#endif

            }
        }

        if (Index == 1) /* menu1 fan */
        {
            if (s_effectFlag1Toggle == 0)
                s_effectFlag1 = -1;
            switch (s_effectFlag1)
            {
            case -1:
//                GUI_DrawBitmap(&bmfan1disable1, 142, 142);
                _BMP_Decode("C:\\Application\\Menu1\\fan1disable1.dta", 142, 142);
                break;

#ifdef WIN32
#else
            case 3:
                Effect1(0x008CC17C);
                Effect2(6);
                Effect3(240, 240-4, 118);
            case 2:
                Effect1(0x00AFD2A0);
                Effect2(4);
                Effect3(240, 240-4, 110);
            case 1:
                Effect1(0x00D3E5CA);
                Effect2(2);
                Effect3(240, 240-4, 100);
#endif

            }
        }

        if (Index == 2) /* menu1 cool */
        {
            if (s_effectFlag2Toggle == 0)
                s_effectFlag2 = -1;
            switch (s_effectFlag2)
            {
            case -1:
//                GUI_DrawBitmap(&bmcool1disable1, 142, 142);
                _BMP_Decode("C:\\Application\\Menu1\\cool1disable1.dta", 142, 142);
                break;

#ifdef WIN32
#else
            case 3:
                Effect1(0x0057C2E1);
                Effect2(6);
                Effect3(240, 240-4, 118);
            case 2:
                Effect1(0x0097D5EA);
                Effect2(4);
                Effect3(240, 240-4, 110);
            case 1:
                Effect1(0x00C2E6F3);
                Effect2(2);
                Effect3(240, 240-4, 100);
#endif

            }
        }

        if (Index == 3) /* menu1 heat */
        {
            if (s_effectFlag3Toggle == 0)
                s_effectFlag3 = -1;
            switch (s_effectFlag3)
            {
            case -1:
//                GUI_DrawBitmap(&bmheat1disable1, 142, 142);
                _BMP_Decode("C:\\Application\\Menu1\\heat1disable1.dta", 142, 142);
                break;

#ifdef WIN32
#else
            case 3:
                Effect1(0x00EA5E67);
                Effect2(6);
                Effect3(240, 240-4, 118);
            case 2:
                Effect1(0x00EF8989);
                Effect2(4);
                Effect3(240, 240-4, 110);
            case 1:
                Effect1(0x00F7C5C1);
                Effect2(2);
                Effect3(240, 240-4, 100);
#endif

            }
        }
        break;
    default:
        WM_DefaultProc(pMsg);
    }
}

static void _CreateSelectedMenu1(int Index, WM_HWIN hWin)
{
    WM_HWIN hWinBase;     // Window to be shifted out
    WM_HWIN hWinSelected; // New window to become visible behind hWinBase
    int xSize, ySize;

    hWinBase = WM_GetParent(WM_GetParent(hWin));
    xSize = WM_GetXSize(hWinBase);
    ySize = WM_GetYSize(hWinBase);
    hWinSelected = WM_CreateWindow(0, 0, xSize, ySize, WM_CF_SHOW, _cbSelectedMenu1, sizeof(Index));
#if NVT_WIFI   /* WiFi */
    switch (Index)
    {
    case 0:
        mcu_dp_bool_update(DPID_MENU1_DEHUMIDIFY,1);
        if (s_effectFlag0Toggle == 0)
            mcu_dp_bool_update(DPID_MENU1_ONOFF_LEVEL,0);
        else
            mcu_dp_bool_update(DPID_MENU1_ONOFF_LEVEL,1);
        mcu_dp_value_update(DPID_MENU1_INCREASE_LEVEL,s_effectFlag0);
        mcu_dp_value_update(DPID_MENU1_DECREASE_LEVEL,s_effectFlag0);
        break;
    case 1:
        mcu_dp_bool_update(DPID_MENU1_FAN,1);
        if (s_effectFlag1Toggle == 0)
            mcu_dp_bool_update(DPID_MENU1_ONOFF_LEVEL,0);
        else
            mcu_dp_bool_update(DPID_MENU1_ONOFF_LEVEL,1);
        mcu_dp_value_update(DPID_MENU1_INCREASE_LEVEL,s_effectFlag1);
        mcu_dp_value_update(DPID_MENU1_DECREASE_LEVEL,s_effectFlag1);
        break;
    case 2:
        mcu_dp_bool_update(DPID_MENU1_COOL,1);
        if (s_effectFlag2Toggle == 0)
            mcu_dp_bool_update(DPID_MENU1_ONOFF_LEVEL,0);
        else
            mcu_dp_bool_update(DPID_MENU1_ONOFF_LEVEL,1);
        mcu_dp_value_update(DPID_MENU1_INCREASE_LEVEL,s_effectFlag2);
        mcu_dp_value_update(DPID_MENU1_DECREASE_LEVEL,s_effectFlag2);
        break;
    case 3:
        mcu_dp_bool_update(DPID_MENU1_HEAT,1);
        if (s_effectFlag3Toggle == 0)
            mcu_dp_bool_update(DPID_MENU1_ONOFF_LEVEL,0);
        else
            mcu_dp_bool_update(DPID_MENU1_ONOFF_LEVEL,1);
        mcu_dp_value_update(DPID_MENU1_INCREASE_LEVEL,s_effectFlag3);
        mcu_dp_value_update(DPID_MENU1_DECREASE_LEVEL,s_effectFlag3);
        break;
    }
#endif
    //
    // Pass menu index to window
    //
    WM_SetUserData(hWinSelected, &Index, sizeof(Index));
    //
    // Window animation
    //
//  GUI_MEMDEV_ShiftOutWindow(hWinBase, 500, GUI_MEMDEV_EDGE_LEFT);
//    GUI_MEMDEV_FadeOutWindow(hWinBase, 500);
    WM_HideWindow(hWinBase);
    while(1)
    {
        if (s_backFlag == 1)
        {
            s_backFlag = 0;
            break;
        }
        GUI_Exec();
#if NVT_WIFI   /* WiFi */
        wifi_uart_service();
        
        if (hWinSelected == 0)
            ;
        else
        {
            if (menu1dehumidify_vflag == 1)
            {
                menu1dehumidify_vflag = 0;
                WM_SendMessageNoPara(hWinSelected, WM_MENU1_BACK);
            }

            if (menu1fan_vflag == 1)
            {
                menu1fan_vflag = 0;
                WM_SendMessageNoPara(hWinSelected, WM_MENU1_BACK);
            }

            if (menu1cool_vflag == 1)
            {
                menu1cool_vflag = 0;
                WM_SendMessageNoPara(hWinSelected, WM_MENU1_BACK);
            }

            if (menu1heat_vflag == 1)
            {
                menu1heat_vflag = 0;
                WM_SendMessageNoPara(hWinSelected, WM_MENU1_BACK);
            }

            if (menu1onofflevel_vflag == 1)
            {
                menu1onofflevel_vflag = 0;
                WM_SendMessageNoPara(hWinSelected, WM_MENU1_ONOFF_LEVEL);
            }

            if (menu1increaselevel_vflag == 1)
            {
                menu1increaselevel_vflag = 0;
                WM_SendMessageNoPara(hWinSelected, WM_MENU1_INCREASE_LEVEL);
            }

            if (menu1decreaselevel_vflag == 1)
            {
                menu1decreaselevel_vflag = 0;
                WM_SendMessageNoPara(hWinSelected, WM_MENU1_DECREASE_LEVEL);
            }
        }
#endif
    }
//  GUI_MEMDEV_ShiftInWindow(hWinBase, 500, GUI_MEMDEV_EDGE_LEFT);
//    GUI_MEMDEV_FadeInWindow(hWinBase, 500);
    WM_ShowWindow(hWinBase);
    //
    // Remove the new window
    //
    WM_DeleteWindow(hWinSelected);
#if NVT_WIFI   /* WiFi */
    switch (Index)
    {
    case 0:
        mcu_dp_bool_update(DPID_MENU1_DEHUMIDIFY,0);
        break;
    case 1:
        mcu_dp_bool_update(DPID_MENU1_FAN,0);
        break;
    case 2:
        mcu_dp_bool_update(DPID_MENU1_COOL,0);
        break;
    case 3:
        mcu_dp_bool_update(DPID_MENU1_HEAT,0);
        break;
    }
#endif
}

static int _ButtonSkinCommon1(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo)
{
    int Index;
    WM_HWIN hWin;

    hWin = pDrawItemInfo->hWin;
    BUTTON_GetUserData(hWin, &Index, sizeof(Index));

    switch (pDrawItemInfo->Cmd)
    {
    case WIDGET_ITEM_DRAW_BACKGROUND:
        if (Index == 5 || Index == 6)
        {
            GUI_SetBkColor(0x00ADAAA5);
//        GUI_SetBkColor(GUI_YELLOW);
            GUI_Clear();
        }
        break;
    case WIDGET_ITEM_DRAW_BITMAP:
        if (Index == 5) /* menu2 up */
//            GUI_DrawBitmap(&bmmenu2up1, (WM_GetXSize(hWin)-bmmenu2up1.XSize)/2, (WM_GetYSize(hWin)-bmmenu2up1.YSize)/2+40);
            _BMP_Decode("C:\\Application\\Menu2\\menu2up1.dta", (WM_GetXSize(hWin)-52)/2, (WM_GetYSize(hWin)-52)/2+40);
        else if (Index == 6)    /* menu2 down */
//            GUI_DrawBitmap(&bmmenu2down1, (WM_GetXSize(hWin)-bmmenu2down1.XSize)/2, (WM_GetYSize(hWin)-bmmenu2down1.YSize)/2+40);
            _BMP_Decode("C:\\Application\\Menu2\\menu2down1.dta", (WM_GetXSize(hWin)-52)/2, (WM_GetYSize(hWin)-52)/2+40);
        else if (Index == 7)    /* menu2 onoff */
        {
            if (s_ledFlag == 0)
//                GUI_DrawBitmap(&bmoff1, 0, 0);
                _BMP_Decode("C:\\Application\\Common\\off1.dta", 0, 0);
            else
//                GUI_DrawBitmap(&bmon1, 0, 0);
                _BMP_Decode("C:\\Application\\Common\\on1.dta", 0, 0);
        }
        break;
    }
    return 0;
}

static int s_Menu2Temp1Flag;

static U8 s_u8BLValue;
static U8 s_au8BLValue[16];

char g_Menu2Temp1Voltage[20];
char g_Menu2Temp1Current[20];
char g_Menu2Temp1Power[20];
char g_Menu2Temp1Energy[20];

#ifdef WIN32
#else
PWM_TIME_DATA_T sPt;
#endif

static int _ButtonSkinMenu2Temp1(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo)
{
    int Index;
    WM_HWIN hWin;

    hWin = pDrawItemInfo->hWin;
    BUTTON_GetUserData(hWin, &Index, sizeof(Index));

    switch (pDrawItemInfo->Cmd)
    {
    case WIDGET_ITEM_DRAW_BACKGROUND:
        //        GUI_SetBkColor(0x0038393B);
        GUI_SetBkColor(GUI_YELLOW);
        GUI_Clear();
        break;
    case WIDGET_ITEM_DRAW_BITMAP:
        if (Index == 0) /* menu2 temp1 back */
//            GUI_DrawBitmap(&bmmenu2back1, 0, 0);
            _BMP_Decode("C:\\Application\\Menu2\\menu2back1.dta", 0, 0);
        else if (Index == 1)    /* menu2 temp1 modbus */
        {
            if (s_Menu2Temp1Flag == 0)
            {
//                GUI_DrawBitmap(&bmoff1, 0, 0);
                _BMP_Decode("C:\\Application\\Common\\off1.dta", 0, 0);

#ifdef WIN32
#else
                g_ModbusIsEnabled = 0;
                g_ModbusTempIsEnabled = 1;
                g_ModbusLedIsEnabled = 1;
                g_ModbusMeterIsEnabled = 1;
#endif

            }
            else
            {
//                GUI_DrawBitmap(&bmon1, 0, 0);
                _BMP_Decode("C:\\Application\\Common\\on1.dta", 0, 0);

#ifdef WIN32
#else
                g_ModbusIsEnabled = 1;
#endif

            }
        }
        break;
    }
    return 0;
}

static void _cbSelectedMenu2Temp1(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hButton;
    TEXT_Handle hText;
    SLIDER_Handle hSlider;
    int i;
    int Id, NCode;
    char chVer[50];
#if NVT_WIFI   /* WiFi */
    static GUI_PID_STATE CurrentState2;
#endif
    hWin = pMsg->hWin;
    switch (pMsg->MsgId)
    {
#if NVT_WIFI   /* WiFi */
    case WM_MENU2_TEMP1_BACK:
        CurrentState2.x = 0 + 10;
        CurrentState2.y = 0 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
    case WM_MENU2_TEMP1_MODBUS:
        CurrentState2.x = 278 + 10;
        CurrentState2.y = 146 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
#endif
    case WM_NOTIFY_PARENT:
        Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
        NCode = pMsg->Data.v;                 // Notification code
#if NVT_WIFI   /* WiFi */
        if (menu2temp1_vflag == 2)
        {
            menu2temp1_vflag = 0;
        }

        if (menu2temp1modbus_vflag == 2)
        {
            menu2temp1modbus_vflag = 0;
        }
#endif
        switch (NCode)
        {
        case WM_NOTIFICATION_CLICKED:
            break;
        case WM_NOTIFICATION_RELEASED:
            if (Id == GUI_ID_BUTTON0+0) /* menu2 temp1 back */
            {
                s_backFlag = 1;
            }

            if (Id == GUI_ID_BUTTON0+1) /* menu2 temp1 modbus */
            {
                s_Menu2Temp1Flag = s_Menu2Temp1Flag ^ 1;
#if NVT_WIFI   /* WiFi */
                menu2temp1modbus_v = s_Menu2Temp1Flag;
                mcu_dp_bool_update(DPID_MENU2_TEMP1_MODBUS,s_Menu2Temp1Flag);
#endif
            }
            break;
        case WM_NOTIFICATION_VALUE_CHANGED:
            hSlider = WM_GetDialogItem(hWin, GUI_ID_SLIDER0);
            s_u8BLValue = SLIDER_GetValue(hSlider);
#if NVT_WIFI   /* WiFi */
            mcu_dp_value_update(DPID_BACKLIGHT,s_u8BLValue);
            menu2temp1backlight_v = s_u8BLValue;
#endif
#ifdef WIN32
#else
            sPt.u8HighPulseRatio = s_u8BLValue; /* High Pulse period : Total Pulse period = 1 : 100 */
            /* Set PWM Timer 0 Configuration */
            PWM_SetTimerClk(PWM_TIMER0,&sPt);
#endif
            hText = WM_GetDialogItem(hWin, GUI_ID_TEXT1);
            sprintf((char *)s_au8BLValue, "BackLight: %d", s_u8BLValue);
            TEXT_SetText(hText, (char *)s_au8BLValue);
            break;
        }
        break;
    case WM_CREATE:
        i = 0; /* menu2 temp1 back */
        hButton = BUTTON_CreateUser(0, 0, 207, 86, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinMenu2Temp1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));

        hText = TEXT_CreateEx(120, 159, 146, 57, hWin, WM_CF_SHOW, 0, GUI_ID_TEXT0, "Modbus");

        TEXT_SetFont(hText, GUI_FONT_32B_1);
        TEXT_SetTextColor(hText, GUI_BLUE);

        i = 1;  /* menu2 temp1 modbus */
        hButton = BUTTON_CreateUser(278, 146, 98, 59, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinMenu2Temp1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));

        hText = TEXT_CreateEx(40, 159+64, 200, 57, hWin, WM_CF_SHOW, 0, GUI_ID_TEXT1, "BackLight: ");
        TEXT_SetFont(hText, GUI_FONT_32B_1);
        TEXT_SetTextColor(hText, GUI_BLUE);

        hSlider = SLIDER_CreateEx(240, 152+64, 200, 32, hWin, WM_CF_SHOW, 0, GUI_ID_SLIDER0);
        SLIDER_SetRange(hSlider, 1, 100);
        SLIDER_SetValue(hSlider, s_u8BLValue);
        SLIDER_SetWidth(hSlider, 20);

        sprintf(g_Menu2Temp1Voltage, "Voltage:%u.%02uV", 0, 0);
        hText = TEXT_CreateEx(40, 159+64+64, 146+64, 57, hWin, WM_CF_SHOW, 0, GUI_ID_TEXT2, "Voltage:0.00V");
        TEXT_SetFont(hText, GUI_FONT_32B_1);
        TEXT_SetTextColor(hText, GUI_BLUE);

        sprintf(g_Menu2Temp1Current, "Current:%u.%02uA", 0, 0);
        hText = TEXT_CreateEx(40, 159+64+64+64, 146+64, 57, hWin, WM_CF_SHOW, 0, GUI_ID_TEXT3, "Current:0.00A");
        TEXT_SetFont(hText, GUI_FONT_32B_1);
        TEXT_SetTextColor(hText, GUI_BLUE);

        sprintf(g_Menu2Temp1Power, "Power:%u.%01uW", 0, 0);
        hText = TEXT_CreateEx(268, 159+64+64, 146+64, 57, hWin, WM_CF_SHOW, 0, GUI_ID_TEXT4, "Power:0.0W");
        TEXT_SetFont(hText, GUI_FONT_32B_1);
        TEXT_SetTextColor(hText, GUI_BLUE);

        sprintf(g_Menu2Temp1Energy, "Energy:%uWh", 0);
        hText = TEXT_CreateEx(268, 159+64+64+64, 146+64, 57, hWin, WM_CF_SHOW, 0, GUI_ID_TEXT5, "Energy:0Wh");
        TEXT_SetFont(hText, GUI_FONT_32B_1);
        TEXT_SetTextColor(hText, GUI_BLUE);

        sprintf(chVer, "Thermostat GUI version: %s", s_ach2ThermostatVersion);
        hText = TEXT_CreateEx(40, 159+64+64+64+64, 400, 40, hWin, WM_CF_SHOW, 0, GUI_ID_TEXT0, chVer);
        TEXT_SetFont(hText, GUI_FONT_32B_1);
        TEXT_SetTextColor(hText, GUI_BLUE);
        //
        // Create timer
        //
        WM_CreateTimer(hWin, 0, 200, 0);
        break;
    case WM_TIMER:
        WM_Invalidate(hWin);
        WM_RestartTimer(pMsg->Data.v, 0);
#if NVT_WIFI   /* WiFi */
#ifdef WIN32
#else
        sPt.u8HighPulseRatio = menu2temp1backlight_v; /* High Pulse period : Total Pulse period = 1 : 100 */
        /* Set PWM Timer 0 Configuration */
        PWM_SetTimerClk(PWM_TIMER0,&sPt);
#endif
        hText = WM_GetDialogItem(hWin, GUI_ID_TEXT1);
        sprintf((char *)s_au8BLValue, "BackLight: %d", menu2temp1backlight_v);
        TEXT_SetText(hText, (char *)s_au8BLValue);
        hSlider = WM_GetDialogItem(hWin, GUI_ID_SLIDER0);
        SLIDER_SetValue(hSlider, menu2temp1backlight_v);
#endif

        break;
    case WM_PAINT:
        GUI_SetBkColor(0x00ADAAA5);
        GUI_Clear();
//        GUI_DrawBitmap(&bmonoffselect1, 230, 152);
        _BMP_Decode("C:\\Application\\Common\\onoffselect1.dta", 230, 152);
        hText = WM_GetDialogItem(hWin, GUI_ID_TEXT2);
        TEXT_SetText(hText, (char *)g_Menu2Temp1Voltage);
        hText = WM_GetDialogItem(hWin, GUI_ID_TEXT3);
        TEXT_SetText(hText, (char *)g_Menu2Temp1Current);
        hText = WM_GetDialogItem(hWin, GUI_ID_TEXT4);
        TEXT_SetText(hText, (char *)g_Menu2Temp1Power);
        hText = WM_GetDialogItem(hWin, GUI_ID_TEXT5);
        TEXT_SetText(hText, (char *)g_Menu2Temp1Energy);
        GUI_DrawRoundedRect(40-10, 159+64+64-10, 450, 390, 10);
        break;
    default:
        WM_DefaultProc(pMsg);
    }
}

static void _CreateSelectedMenu2Temp1(int Index, WM_HWIN hWin)
{
    WM_HWIN hWinBase;     // Window to be shifted out
    WM_HWIN hWinSelected; // New window to become visible behind hWinBase
    int xSize, ySize;

    UINT32 u32Item;
    short hi_voltage,lo_voltage;

    hWinBase = WM_GetParent(WM_GetParent(hWin));
    xSize = WM_GetXSize(hWinBase);
    ySize = WM_GetYSize(hWinBase);
    hWinSelected = WM_CreateWindow(0, 0, xSize, ySize, WM_CF_SHOW, _cbSelectedMenu2Temp1, sizeof(Index));
#if NVT_WIFI   /* WiFi */
    mcu_dp_bool_update(DPID_MENU2_TEMP1,1);
#endif
    //
    // Pass menu index to window
    //
    WM_SetUserData(hWinSelected, &Index, sizeof(Index));
    //
    // Window animation
    //
//  GUI_MEMDEV_ShiftOutWindow(hWinBase, 500, GUI_MEMDEV_EDGE_LEFT);
//    GUI_MEMDEV_FadeOutWindow(hWinBase, 500);
    WM_HideWindow(hWinBase);
    while(1)
    {
        if (s_backFlag == 1)
        {
            s_backFlag = 0;
            break;
        }
        GUI_Exec();
#if NVT_WIFI   /* WiFi */
        wifi_uart_service();
        
        if (hWinSelected == 0)
            ;
        else
        {
            if (menu2temp1_vflag == 1)
            {
                menu2temp1_vflag = 2;
                WM_SendMessageNoPara(hWinSelected, WM_MENU2_TEMP1_BACK);
            }

            if (menu2temp1modbus_vflag == 1)
            {
                menu2temp1modbus_vflag = 2;
                WM_SendMessageNoPara(hWinSelected, WM_MENU2_TEMP1_MODBUS);
            }
        }
#endif
        if (g_ModbusIsEnabled == 1 && g_ModbusMeterIsEnabled == 1)
        {
            // Modbus slave ID 1
            begin(1);
//            sysprintf(" [1] Read the measurement\n");
//          sysprintf(" [2] Read the slave parameter\n");
//          sysprintf(" [3] Write the slave parameter - (High voltage alarm)\n");
//          sysprintf(" [4] Write the slave parameter - (Low voltage alarm)\n");
//          sysprintf(" [5] Reset energy\n");
//          sysprintf(" [6] Calibration\n");
            u32Item = '1';

            if (u32Item == '1')
            {
                if (readInputRegisters(0x0, 8) == 0)
                    g_ModbusMeterIsEnabled = 1;
                else
                    g_ModbusMeterIsEnabled = 0;
            }
            else if (u32Item == '2')
            {
                readHoldingRegisters(0, 4);
            }
            else if ( u32Item == '3')
            {
                hi_voltage = 30000; //300.00 V
                writeSingleRegister(0,hi_voltage);
            }
            else if ( u32Item == '4')
            {
                lo_voltage = 700; //7.00 V
                writeSingleRegister(1,lo_voltage);
            }
            else if ( u32Item == '5')
            {
                resetenergyRegister(0, 0);
            }
            else if ( u32Item == '6')
            {
                calibrateRegister(0x3721, 0);
            }
        }
    }
//  GUI_MEMDEV_ShiftInWindow(hWinBase, 500, GUI_MEMDEV_EDGE_LEFT);
//    GUI_MEMDEV_FadeInWindow(hWinBase, 500);
    WM_ShowWindow(hWinBase);
    //
    // Remove the new window
    //
    WM_DeleteWindow(hWinSelected);
#if NVT_WIFI   /* WiFi */
    mcu_dp_bool_update(DPID_MENU2_TEMP1,0);
#endif
}

static int s_Menu1Mode1Flag;
static int s_Menu1Mode2Flag;
static int s_Menu1Mode3Flag;

static int _ButtonSkinMenu1Mode1(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo)
{
    int Index;
    WM_HWIN hWin;

    hWin = pDrawItemInfo->hWin;
    BUTTON_GetUserData(hWin, &Index, sizeof(Index));

    switch (pDrawItemInfo->Cmd)
    {
    case WIDGET_ITEM_DRAW_BACKGROUND:
        GUI_SetBkColor(0x0038393B);
//        GUI_SetBkColor(GUI_YELLOW);
        GUI_Clear();
        break;
    case WIDGET_ITEM_DRAW_BITMAP:
        if (Index == 0)  /* menu1 mode1 back */
//            GUI_DrawBitmap(&bmmenu1back1, 26, 25);
            _BMP_Decode("C:\\Application\\Menu1\\menu1back1.dta", 26, 25);
        else if (Index == 1)  /* menu1 mode1 mode1 */
        {
            if (s_Menu1Mode1Flag == 0)
//                GUI_DrawBitmap(&bmoff1, 0, 0);
                _BMP_Decode("C:\\Application\\Common\\off1.dta", 0, 0);
            else
//                GUI_DrawBitmap(&bmon1, 0, 0);
                _BMP_Decode("C:\\Application\\Common\\on1.dta", 0, 0);
        }
        else if (Index == 2)  /* menu1 mode1 mode2 */
        {
            if (s_Menu1Mode2Flag == 0)
//                GUI_DrawBitmap(&bmoff1, 0, 0);
                _BMP_Decode("C:\\Application\\Common\\off1.dta", 0, 0);
            else
//                GUI_DrawBitmap(&bmon1, 0, 0);
                _BMP_Decode("C:\\Application\\Common\\on1.dta", 0, 0);
        }
        else if (Index == 3)  /* menu1 mode1 mode3 */
        {
            if (s_Menu1Mode3Flag == 0)
//                GUI_DrawBitmap(&bmoff1, 0, 0);
                _BMP_Decode("C:\\Application\\Common\\off1.dta", 0, 0);
            else
//                GUI_DrawBitmap(&bmon1, 0, 0);
                _BMP_Decode("C:\\Application\\Common\\on1.dta", 0, 0);
        }
        break;
    }
    return 0;
}

static void _cbSelectedMenu1Mode1(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hButton;
    TEXT_Handle hText;
    int i;
    int Id, NCode;
#if NVT_WIFI   /* WiFi */
    static GUI_PID_STATE CurrentState2;
#endif
    hWin = pMsg->hWin;
    switch (pMsg->MsgId)
    {
#if NVT_WIFI   /* WiFi */
    case WM_MENU1_MODE1_BACK:
        CurrentState2.x = 0 + 10;
        CurrentState2.y = 0 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
    case WM_MENU1_MODE1_MODE1:
        CurrentState2.x = 278 + 10;
        CurrentState2.y = 146 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
    case WM_MENU1_MODE1_MODE2:
        CurrentState2.x = 278 + 10;
        CurrentState2.y = 146 + 64 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
    case WM_MENU1_MODE1_MODE3:
        CurrentState2.x = 278 + 10;
        CurrentState2.y = 146 + 64 + 64 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
#endif
    case WM_NOTIFY_PARENT:
        Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
        NCode = pMsg->Data.v;                 // Notification code
#if NVT_WIFI   /* WiFi */
        if (menu1mode1_vflag == 2)
        {
            menu1mode1_vflag = 0;
        }

        if (menu1mode1mode1_vflag == 2)
        {
            menu1mode1mode1_vflag = 0;
        }

        if (menu1mode1mode2_vflag == 2)
        {
            menu1mode1mode2_vflag = 0;
        }

        if (menu1mode1mode3_vflag == 2)
        {
            menu1mode1mode3_vflag = 0;
        }
#endif
        switch (NCode)
        {
        case WM_NOTIFICATION_CLICKED:
            break;
        case WM_NOTIFICATION_RELEASED:
            if (Id == GUI_ID_BUTTON0+0)  /* menu1 mode1 back */
            {
                s_backFlag = 1;
            }

            if (Id == GUI_ID_BUTTON0+1)  /* menu1 mode1 mode1 */
            {
                s_Menu1Mode1Flag = s_Menu1Mode1Flag ^ 1;
#if NVT_WIFI   /* WiFi */
                menu1mode1mode1_v = s_Menu1Mode1Flag;
                mcu_dp_bool_update(DPID_MENU1_MODE1_MODE1,s_Menu1Mode1Flag);
#endif
            }

            if (Id == GUI_ID_BUTTON0+2)  /* menu1 mode1 mode2 */
            {
                s_Menu1Mode2Flag = s_Menu1Mode2Flag ^ 1;
#if NVT_WIFI   /* WiFi */
                menu1mode1mode2_v = s_Menu1Mode2Flag;
                mcu_dp_bool_update(DPID_MENU1_MODE1_MODE2,s_Menu1Mode2Flag);
#endif
            }

            if (Id == GUI_ID_BUTTON0+3)  /* menu1 mode1 mode3 */
            {
                s_Menu1Mode3Flag = s_Menu1Mode3Flag ^ 1;
#if NVT_WIFI   /* WiFi */
                menu1mode1mode3_v = s_Menu1Mode3Flag;
                mcu_dp_bool_update(DPID_MENU1_MODE1_MODE3,s_Menu1Mode3Flag);
#endif
            }
            break;
        }
        break;
    case WM_CREATE:
        i = 0;  /* menu1 mode1 back */
        hButton = BUTTON_CreateUser(0, 0, 128+26, 100, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinMenu1Mode1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));

        i = 1;  /* menu1 mode1 mode1 */
        hButton = BUTTON_CreateUser(278, 146, 98, 59, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinMenu1Mode1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));

        i = 2;  /* menu1 mode1 mode2 */
        hButton = BUTTON_CreateUser(278, 146+64, 98, 59, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinMenu1Mode1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));

        i = 3;  /* menu1 mode1 mode3 */
        hButton = BUTTON_CreateUser(278, 146+64+64, 98, 59, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinMenu1Mode1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));

        hText = TEXT_CreateEx(140, 159, 146, 57, hWin, WM_CF_SHOW, 0, GUI_ID_TEXT0, "Mode1");
        TEXT_SetFont(hText, GUI_FONT_32B_1);
        TEXT_SetTextColor(hText, GUI_BLUE);

        hText = TEXT_CreateEx(140, 159+64, 146, 57, hWin, WM_CF_SHOW, 0, GUI_ID_TEXT0, "Mode2");
        TEXT_SetFont(hText, GUI_FONT_32B_1);
        TEXT_SetTextColor(hText, GUI_BLUE);

        hText = TEXT_CreateEx(140, 159+64+64, 146, 57, hWin, WM_CF_SHOW, 0, GUI_ID_TEXT0, "Mode3");
        TEXT_SetFont(hText, GUI_FONT_32B_1);
        TEXT_SetTextColor(hText, GUI_BLUE);
        break;
    case WM_PAINT:
        GUI_SetBkColor(0x0038393B);
        GUI_Clear();
        GUI_SetBkColor(0x00ADAAA5);
//        GUI_FillRect(0, 100, 480-1, 480-1);
        GUI_ClearRect(0, 100, 480-1, 480-1);
//        GUI_DrawBitmap(&bmonoffselect1, 230, 152);
        _BMP_Decode("C:\\Application\\Common\\onoffselect1.dta", 230, 152);
//        GUI_DrawBitmap(&bmonoffselect1, 230, 152+64);
        _BMP_Decode("C:\\Application\\Common\\onoffselect1.dta", 230, 152+64);
//        GUI_DrawBitmap(&bmonoffselect1, 230, 152+64+64);
        _BMP_Decode("C:\\Application\\Common\\onoffselect1.dta", 230, 152+64+64);
        break;
    default:
        WM_DefaultProc(pMsg);
    }
}

static void _CreateSelectedMenu1Mode1(int Index, WM_HWIN hWin)
{
    WM_HWIN hWinBase;     // Window to be shifted out
    WM_HWIN hWinSelected; // New window to become visible behind hWinBase
    int xSize, ySize;

    hWinBase = WM_GetParent(WM_GetParent(hWin));
    xSize = WM_GetXSize(hWinBase);
    ySize = WM_GetYSize(hWinBase);
    hWinSelected = WM_CreateWindow(0, 0, xSize, ySize, WM_CF_SHOW, _cbSelectedMenu1Mode1, sizeof(Index));
#if NVT_WIFI   /* WiFi */
    mcu_dp_bool_update(DPID_MENU1_MODE1,1);
#endif
    //
    // Pass menu index to window
    //
    WM_SetUserData(hWinSelected, &Index, sizeof(Index));
    //
    // Window animation
    //
//  GUI_MEMDEV_ShiftOutWindow(hWinBase, 500, GUI_MEMDEV_EDGE_LEFT);
//    GUI_MEMDEV_FadeOutWindow(hWinBase, 500);
    WM_HideWindow(hWinBase);
    while(1)
    {
        if (s_backFlag == 1)
        {
            s_backFlag = 0;
            break;
        }
        GUI_Exec();
#if NVT_WIFI   /* WiFi */
        wifi_uart_service();
        
        if (hWinSelected == 0)
            ;
        else
        {
            if (menu1mode1_vflag == 1)
            {
                menu1mode1_vflag = 2;
                WM_SendMessageNoPara(hWinSelected, WM_MENU1_MODE1_BACK);
            }

            if (menu1mode1mode1_vflag == 1)
            {
                menu1mode1mode1_vflag = 2;
                WM_SendMessageNoPara(hWinSelected, WM_MENU1_MODE1_MODE1);
            }

            if (menu1mode1mode2_vflag == 1)
            {
                menu1mode1mode2_vflag = 2;
                WM_SendMessageNoPara(hWinSelected, WM_MENU1_MODE1_MODE2);
            }

            if (menu1mode1mode3_vflag == 1)
            {
                menu1mode1mode3_vflag = 2;
                WM_SendMessageNoPara(hWinSelected, WM_MENU1_MODE1_MODE3);
            }
        }
#endif
    }
//  GUI_MEMDEV_ShiftInWindow(hWinBase, 500, GUI_MEMDEV_EDGE_LEFT);
//    GUI_MEMDEV_FadeInWindow(hWinBase, 500);
    WM_ShowWindow(hWinBase);
    //
    // Remove the new window
    //
    WM_DeleteWindow(hWinSelected);
#if NVT_WIFI   /* WiFi */
    mcu_dp_bool_update(DPID_MENU1_MODE1,0);
#endif
}

static int _ButtonSkinMenu3Date1(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo)
{
    int Index;
    WM_HWIN hWin;

    hWin = pDrawItemInfo->hWin;
    BUTTON_GetUserData(hWin, &Index, sizeof(Index));

    switch (pDrawItemInfo->Cmd)
    {
    case WIDGET_ITEM_DRAW_BACKGROUND:
        GUI_SetBkColor(0x00ABBCC5);
//        GUI_SetBkColor(GUI_YELLOW);
        GUI_Clear();
        break;
    case WIDGET_ITEM_DRAW_BITMAP:
        if (Index == 0) /* menu3 date1 back */
//            GUI_DrawBitmap(&bmmenu3back1, 26, 25);
            _BMP_Decode("C:\\Application\\Menu3\\menu3back1.dta", 26, 25);
        else if (Index == 1)    /* menu3 date1 date1 */
        {
            if (s_Menu3Date1Flag == 0)
//                GUI_DrawBitmap(&bmoff1, 0, 0);
                _BMP_Decode("C:\\Application\\Common\\off1.dta", 0, 0);
            else
//                GUI_DrawBitmap(&bmon1, 0, 0);
                _BMP_Decode("C:\\Application\\Common\\on1.dta", 0, 0);

#if NVT_WIFI   /* WiFi */
                menu3date1date1_v = s_Menu3Date1Flag;
                mcu_dp_bool_update(DPID_MENU3_DATE1_DATE1,s_Menu3Date1Flag);
#endif

        }
        else if (Index == 2)    /* menu3 date1 date2 */
        {
            if (s_Menu3Date2Flag == 0)
//                GUI_DrawBitmap(&bmoff1, 0, 0);
                _BMP_Decode("C:\\Application\\Common\\off1.dta", 0, 0);
            else
//                GUI_DrawBitmap(&bmon1, 0, 0);
                _BMP_Decode("C:\\Application\\Common\\on1.dta", 0, 0);
        }
        else if (Index == 3)    /* menu3 date1 date3 */
        {
            if (s_Menu3Date3Flag == 0)
//                GUI_DrawBitmap(&bmoff1, 0, 0);
                _BMP_Decode("C:\\Application\\Common\\off1.dta", 0, 0);
            else
//                GUI_DrawBitmap(&bmon1, 0, 0);
                _BMP_Decode("C:\\Application\\Common\\on1.dta", 0, 0);
        }
        break;
    }
    return 0;
}

static char s_au8Date1[64];

static void _cbSelectedMenu3Date1(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hButton;
    TEXT_Handle hText;
    int i;
    int Id, NCode;
#if NVT_WIFI   /* WiFi */
    static GUI_PID_STATE CurrentState2;
#endif
    hWin = pMsg->hWin;
    switch (pMsg->MsgId)
    {
#if NVT_WIFI   /* WiFi */
    case WM_MENU3_DATE1_BACK:
        CurrentState2.x = 0 + 10;
        CurrentState2.y = 0 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
    case WM_MENU3_DATE1_DATE1:
        CurrentState2.x = 278 + 10;
        CurrentState2.y = 146 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
    case WM_MENU3_DATE1_DATE2:
        CurrentState2.x = 278 + 10;
        CurrentState2.y = 146 + 64 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
    case WM_MENU3_DATE1_DATE3:
        CurrentState2.x = 278 + 10;
        CurrentState2.y = 146 + 64 + 64 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
#endif
    case WM_NOTIFY_PARENT:
        Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
        NCode = pMsg->Data.v;                 // Notification code
#if NVT_WIFI   /* WiFi */
        if (menu3date1_vflag == 2)
        {
            menu3date1_vflag = 0;
        }

        if (menu3date1date1_vflag == 2)
        {
            menu3date1date1_vflag = 0;
        }

        if (menu3date1date2_vflag == 2)
        {
            menu3date1date2_vflag = 0;
        }

        if (menu3date1date3_vflag == 2)
        {
            menu3date1date3_vflag = 0;
        }
#endif
        switch (NCode)
        {
        case WM_NOTIFICATION_CLICKED:
            break;
        case WM_NOTIFICATION_RELEASED:
            if (Id == GUI_ID_BUTTON0+0) /* menu3 date1 back */
            {
                s_backFlag = 1;
            }

            if (Id == GUI_ID_BUTTON0+1) /* menu3 date1 date1 */
            {
                s_Menu3Date1Flag = s_Menu3Date1Flag ^ 1;
#if NVT_WIFI   /* WiFi */
                menu3date1date1_v = s_Menu3Date1Flag;
                mcu_dp_bool_update(DPID_MENU3_DATE1_DATE1,s_Menu3Date1Flag);
#endif

#ifdef WIN32
#else
                if (s_Menu3Date1Flag == 1)
                {
                    g_sCurTime2.pfnAlarmCallBack = RTC_AlarmISR;
                    RTC_Write(RTC_ALARM_TIME, &g_sCurTime2);
                }
                else
                {
                    g_sCurTime2.pfnAlarmCallBack = NULL;
                    RTC_Write(RTC_ALARM_TIME, &g_sCurTime2);
                }
#endif

            }

            if (Id == GUI_ID_BUTTON0+2) /* menu3 date1 date2 */
            {
                s_Menu3Date2Flag = s_Menu3Date2Flag ^ 1;
#if NVT_WIFI   /* WiFi */
                menu3date1date2_v = s_Menu3Date2Flag;
                mcu_dp_bool_update(DPID_MENU3_DATE1_DATE2,s_Menu3Date2Flag);
#endif
            }

            if (Id == GUI_ID_BUTTON0+3) /* menu3 date1 date3 */
            {
                s_Menu3Date3Flag = s_Menu3Date3Flag ^ 1;
#if NVT_WIFI   /* WiFi */
                menu3date1date3_v = s_Menu3Date3Flag;
                mcu_dp_bool_update(DPID_MENU3_DATE1_DATE3,s_Menu3Date3Flag);
#endif
            }
            break;
        }
        break;
    case WM_CREATE:
        CALENDAR_GetSel(s_hCal, &s_calDate);

        g_sCurTime2.u32Year          = s_calDate.Year;
        g_sCurTime2.u32cMonth        = s_calDate.Month;
        g_sCurTime2.u32cDay          = s_calDate.Day;
        g_sCurTime2.u32cHour         = g_sCurTime.u32cHour;
        g_sCurTime2.u32cMinute       = g_sCurTime.u32cMinute + 1;
        g_sCurTime2.u32cSecond       = 0;
        g_sCurTime2.u32cDayOfWeek    = g_sCurTime.u32cDayOfWeek;
        g_sCurTime2.u8cClockDisplay  = g_sCurTime.u8cClockDisplay;

        sprintf(s_au8Date1, "Date1 %d-%02d-%02d %02d:%02d\n",
            g_sCurTime2.u32Year,
            g_sCurTime2.u32cMonth,
            g_sCurTime2.u32cDay,
            g_sCurTime2.u32cHour,
            g_sCurTime2.u32cMinute);

        i = 0;  /* menu3 date1 back */
        hButton = BUTTON_CreateUser(0, 0, 128+32, 100, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinMenu3Date1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));

        i = 1;  /* menu3 date1 date1 */
        hButton = BUTTON_CreateUser(278, 146, 98, 59, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinMenu3Date1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));

        i = 2;  /* menu3 date1 date2 */
        hButton = BUTTON_CreateUser(278, 146+64, 98, 59, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinMenu3Date1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));

        i = 3;  /* menu3 date1 date3 */
        hButton = BUTTON_CreateUser(278, 146+64+64, 98, 59, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinMenu3Date1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));

        hText = TEXT_CreateEx(10, 159, 200, 57, hWin, WM_CF_SHOW, 0, GUI_ID_TEXT0, s_au8Date1);
        TEXT_SetFont(hText, GUI_FONT_20B_ASCII);
        TEXT_SetTextColor(hText, GUI_BLUE);

        hText = TEXT_CreateEx(140, 159+64, 146, 57, hWin, WM_CF_SHOW, 0, GUI_ID_TEXT0, "Date2");
        TEXT_SetFont(hText, GUI_FONT_32B_1);
        TEXT_SetTextColor(hText, GUI_BLUE);

        hText = TEXT_CreateEx(140, 159+64+64, 146, 57, hWin, WM_CF_SHOW, 0, GUI_ID_TEXT0, "Date3");
        TEXT_SetFont(hText, GUI_FONT_32B_1);
        TEXT_SetTextColor(hText, GUI_BLUE);

        break;
    case WM_PAINT:
        GUI_SetBkColor(0x00ABBCC5);
        GUI_Clear();
        GUI_SetBkColor(0x00ADAAA5);
//        GUI_FillRect(0, 100, 480-1, 480-1);
        GUI_ClearRect(0, 100, 480-1, 480-1);
//        GUI_DrawBitmap(&bmonoffselect1, 230, 152);
        _BMP_Decode("C:\\Application\\Common\\onoffselect1.dta", 230, 152);
//        GUI_DrawBitmap(&bmonoffselect1, 230, 152+64);
        _BMP_Decode("C:\\Application\\Common\\onoffselect1.dta", 230, 152+64);
//        GUI_DrawBitmap(&bmonoffselect1, 230, 152+64+64);
        _BMP_Decode("C:\\Application\\Common\\onoffselect1.dta", 230, 152+64+64);
        break;
    default:
        WM_DefaultProc(pMsg);
    }
}

static void _CreateSelectedMenu3Date1(int Index, WM_HWIN hWin)
{
    WM_HWIN hWinBase;     // Window to be shifted out
    WM_HWIN hWinSelected; // New window to become visible behind hWinBase
    int xSize, ySize;

    hWinBase = WM_GetParent(WM_GetParent(hWin));
    xSize = WM_GetXSize(hWinBase);
    ySize = WM_GetYSize(hWinBase);
    hWinSelected = WM_CreateWindow(0, 0, xSize, ySize, WM_CF_SHOW, _cbSelectedMenu3Date1, sizeof(Index));
#if NVT_WIFI   /* WiFi */
    mcu_dp_bool_update(DPID_MENU3_DATE1,1);
#endif
    //
    // Pass menu index to window
    //
    WM_SetUserData(hWinSelected, &Index, sizeof(Index));
    //
    // Window animation
    //
//  GUI_MEMDEV_ShiftOutWindow(hWinBase, 500, GUI_MEMDEV_EDGE_LEFT);
//    GUI_MEMDEV_FadeOutWindow(hWinBase, 500);
    WM_HideWindow(hWinBase);
    while(1)
    {
        if (s_backFlag == 1)
        {
            s_backFlag = 0;
            break;
        }
        GUI_Exec();
#if NVT_WIFI   /* WiFi */
        wifi_uart_service();
        
        if (hWinSelected == 0)
            ;
        else
        {
            if (menu3date1_vflag == 1)
            {
                menu3date1_vflag = 2;
                WM_SendMessageNoPara(hWinSelected, WM_MENU3_DATE1_BACK);
            }

            if (menu3date1date1_vflag == 1)
            {
                menu3date1date1_vflag = 2;
                WM_SendMessageNoPara(hWinSelected, WM_MENU3_DATE1_DATE1);
            }

            if (menu3date1date2_vflag == 1)
            {
                menu3date1date2_vflag = 2;
                WM_SendMessageNoPara(hWinSelected, WM_MENU3_DATE1_DATE2);
            }

            if (menu3date1date3_vflag == 1)
            {
                menu3date1date3_vflag = 2;
                WM_SendMessageNoPara(hWinSelected, WM_MENU3_DATE1_DATE3);
            }
        }
#endif
    }
//  GUI_MEMDEV_ShiftInWindow(hWinBase, 500, GUI_MEMDEV_EDGE_LEFT);
//    GUI_MEMDEV_FadeInWindow(hWinBase, 500);
    WM_ShowWindow(hWinBase);
    //
    // Remove the new window
    //
    WM_DeleteWindow(hWinSelected);
#if NVT_WIFI   /* WiFi */
    mcu_dp_bool_update(DPID_MENU3_DATE1,0);
#endif
}

/*********************************************************************
*
*       _cbMenu
*
* Purpose:
*   Callback function of menu window.
*/

static const char * _apMonths[] =
{
    "JAN",
    "FEB",
    "MAR",
    "APR",
    "MAY",
    "JUN",
    "JLY",
    "AUG",
    "SEP",
    "OCT",
    "NOV",
    "DEC",
};

static const char * _apDays[] =
{
    "SAT",
    "SUN",
    "MON",
    "TUE",
    "WED",
    "THU",
    "FRI",
};

static void _cbMenu(WM_MESSAGE * pMsg)
{
    int i, Id, NCode;
    WM_MOTION_INFO * pInfo;
    WM_HWIN hWin, hButton, hImage1, hImage2, hImage3;
    U32 u32FileSize;
    //
    // input device state
    //
    GUI_PID_STATE CurrentState;
    static int IsPressed;
#if NVT_WIFI   /* WiFi */
    static int xpox_v;
    static GUI_PID_STATE CurrentState2;
#endif
    hWin = pMsg->hWin;
    switch (pMsg->MsgId)
    {
    case WM_TIMER:
        WM_DeleteTimer(pMsg->Data.v);
        //
        // get current input device state
        //
        GUI_PID_GetState(&CurrentState);
        //
        // if pressed for 500ms, then start repeated
        //
        if (CurrentState.Pressed == 1)
        {
            //
            // update input device state to simulate pressing key
            //
            CurrentState.Pressed = 0;
            GUI_PID_StoreState(&CurrentState);
            CurrentState.Pressed = 1;
            GUI_PID_StoreState(&CurrentState);
        }
        break;
#if NVT_WIFI   /* WiFi */
    case WM_MENU2_ONOFF:
        CurrentState2.x = 184 + 10;
        CurrentState2.y = 293 + 60 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
    case WM_MENU2_TEMP1:
        CurrentState2.x = 0 + 10;
        CurrentState2.y = 0 + 60 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
    case WM_MENU1_MODE1:
        CurrentState2.x = 0 + 10;
        CurrentState2.y = 0 + 60 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
    case WM_MENU3_DATE1:
        CurrentState2.x = 0 + 10;
        CurrentState2.y = 0 + 60 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
    case WM_MENU123:
        if (menu_v == 2)
        {
            if (menu123_v == 1)
            {
                menu_v = 1;
                CurrentState2.x = 0 + 10;
                CurrentState2.y = 0 + 60 + 100;
                CurrentState2.Layer = 0;
                CurrentState2.Pressed = 1;
                GUI_PID_StoreState(&CurrentState2);
                for (xpox_v=10;xpox_v<340;xpox_v+=10)
                {
                CurrentState2.x = xpox_v;
                CurrentState2.y = 0 + 60 + 100;
                CurrentState2.Pressed = 1;
                GUI_PID_StoreState(&CurrentState2);
                }
                CurrentState2.x = xpox_v-10;
                CurrentState2.y = 0 + 60 + 100;
                CurrentState2.Pressed = 0;
                GUI_PID_StoreState(&CurrentState2);
            }
            else if (menu123_v == 3)
            {
                menu_v = 3;
                CurrentState2.x = 340;
                CurrentState2.y = 0 + 60 + 100;
                CurrentState2.Layer = 0;
                CurrentState2.Pressed = 1;
                GUI_PID_StoreState(&CurrentState2);
                for (xpox_v=340;xpox_v>10;xpox_v-=10)
                {
                CurrentState2.x = xpox_v;
                CurrentState2.y = 0 + 60 + 100;
                CurrentState2.Pressed = 1;
                GUI_PID_StoreState(&CurrentState2);
                }
                CurrentState2.x = xpox_v+10;
                CurrentState2.y = 0 + 60 + 100;
                CurrentState2.Pressed = 0;
                GUI_PID_StoreState(&CurrentState2);
            }
        }
        else if (menu_v == 1)
        {
            if (menu123_v == 2)
            {
                menu_v = 2;
                CurrentState2.x = 340;
                CurrentState2.y = 0 + 60 + 100;
                CurrentState2.Layer = 0;
                CurrentState2.Pressed = 1;
                GUI_PID_StoreState(&CurrentState2);
                for (xpox_v=340;xpox_v>10;xpox_v-=10)
                {
                CurrentState2.x = xpox_v;
                CurrentState2.y = 0 + 60 + 100;
                CurrentState2.Pressed = 1;
                GUI_PID_StoreState(&CurrentState2);
                }
                CurrentState2.x = xpox_v+10;
                CurrentState2.y = 0 + 60 + 100;
                CurrentState2.Pressed = 0;
                GUI_PID_StoreState(&CurrentState2);
            }
        }
        else if (menu_v == 3)
        {
            if (menu123_v == 2)
            {
                menu_v = 2;
                CurrentState2.x = 0 + 10;
                CurrentState2.y = 0 + 60 + 100;
                CurrentState2.Layer = 0;
                CurrentState2.Pressed = 1;
                GUI_PID_StoreState(&CurrentState2);
                for (xpox_v=10;xpox_v<340;xpox_v+=10)
                {
                CurrentState2.x = xpox_v;
                CurrentState2.y = 0 + 60 + 100;
                CurrentState2.Pressed = 1;
                GUI_PID_StoreState(&CurrentState2);
                }
                CurrentState2.x = xpox_v-10;
                CurrentState2.y = 0 + 60 + 100;
                CurrentState2.Pressed = 0;
                GUI_PID_StoreState(&CurrentState2);
            }
        }
        break;
    case WM_MENU1_DEHUMIDIFY:
        CurrentState2.x = 99 + 10;
        CurrentState2.y = 105 + 60 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
    case WM_MENU1_FAN:
        CurrentState2.x = 249 + 10;
        CurrentState2.y = 103 + 60 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
    case WM_MENU1_COOL:
        CurrentState2.x = 102 + 10;
        CurrentState2.y = 264 + 60 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
    case WM_MENU1_HEAT:
        CurrentState2.x = 246 + 10;
        CurrentState2.y = 263 + 60 + 10;
        CurrentState2.Layer = 0;
        CurrentState2.Pressed = 1;
        GUI_PID_StoreState(&CurrentState2);
        CurrentState2.Pressed = 0;
        GUI_PID_StoreState(&CurrentState2);
        break;
#endif
    case WM_NOTIFY_PARENT:
        Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
        NCode = pMsg->Data.v;                 // Notification code
#if NVT_WIFI   /* WiFi */
        if (menu2onoff_vflag == 2)
        {
            menu2onoff_vflag = 0;
        }

        if (menu2temp1_vflag == 2)
        {
            menu2temp1_vflag = 0;
        }

        if (menu1mode1_vflag == 2)
        {
            menu1mode1_vflag = 0;
        }

        if (menu3date1_vflag == 2)
        {
            menu3date1_vflag = 0;
        }

        if (menu123_vflag == 2)
        {
            menu123_vflag = 0;
        }

        if (menu1dehumidify_vflag == 2)
        {
            menu1dehumidify_vflag = 0;
        }

        if (menu1fan_vflag == 2)
        {
            menu1fan_vflag = 0;
        }

        if (menu1cool_vflag == 2)
        {
            menu1cool_vflag = 0;
        }

        if (menu1heat_vflag == 2)
        {
            menu1heat_vflag = 0;
        }
#endif
        switch (NCode)
        {
        case WM_NOTIFICATION_CLICKED:
            IsPressed = 1;
            if (Id == GUI_ID_BUTTON0+5) /* menu2 up */
            {
                TEXT_SetFont(s_hText1, GUI_FONT_D80);
                TEXT_SetTextColor(s_hText1, GUI_RED);
                TEXT_SetFont(s_hText2, GUI_FONT_32B_1);
                TEXT_SetTextColor(s_hText2, GUI_RED);
                g_i32SetTemp += 5;
                if (g_i32SetTemp > 370)
                    g_i32SetTemp = 370;
                sprintf(g_au8SetTemp, "%d", g_i32SetTemp);
                s_achT1[0] = g_au8SetTemp[0];
                s_achT1[1] = g_au8SetTemp[1];
                s_achT2[1] = g_au8SetTemp[2];
                TEXT_SetText(s_hText1, s_achT1);
                TEXT_SetText(s_hText2, s_achT2);
#if NVT_WIFI   /* WiFi */
                menu2settemp_v = g_i32SetTemp;
                mcu_dp_value_update(DPID_TEMP_SET,menu2settemp_v);
#endif
            }

            if (Id == GUI_ID_BUTTON0+6) /* menu2 down */
            {
                TEXT_SetFont(s_hText1, GUI_FONT_D80);
                TEXT_SetTextColor(s_hText1, GUI_BLUE);
                TEXT_SetFont(s_hText2, GUI_FONT_32B_1);
                TEXT_SetTextColor(s_hText2, GUI_BLUE);
                g_i32SetTemp -= 5;
                if (g_i32SetTemp < 160)
                    g_i32SetTemp = 160;
                sprintf(g_au8SetTemp, "%d", g_i32SetTemp);
                s_achT1[0] = g_au8SetTemp[0];
                s_achT1[1] = g_au8SetTemp[1];
                s_achT2[1] = g_au8SetTemp[2];
                TEXT_SetText(s_hText1, s_achT1);
                TEXT_SetText(s_hText2, s_achT2);
#if NVT_WIFI   /* WiFi */
                menu2settemp_v = g_i32SetTemp;
                mcu_dp_value_update(DPID_TEMP_SET,menu2settemp_v);
#endif
            }

            if ((Id == GUI_ID_BUTTON0+5) || (Id == GUI_ID_BUTTON0+6)) /* menu2 up down */
            {
                //
                // Create timer
                //
                WM_CreateTimer(hWin, 2, 500, 0);
            }
            break;
        case WM_NOTIFICATION_RELEASED:
            if (IsPressed)
            {
                //
                // React only if window is not moving
                //
                // Menu1
                if ((Id == GUI_ID_BUTTON0+0) || (Id == GUI_ID_BUTTON0+1) || (Id == GUI_ID_BUTTON0+2) || (Id == GUI_ID_BUTTON0+3))
                    _CreateSelectedMenu1(Id - GUI_ID_BUTTON0, pMsg->hWin);

                if (Id == (GUI_ID_BUTTON0+4))   /* menu1 mode1 */
                    _CreateSelectedMenu1Mode1(Id - GUI_ID_BUTTON0, pMsg->hWin);

                if (Id == GUI_ID_BUTTON0+7) /* menu2 onoff */
                {
                    s_ledFlag = s_ledFlag ^ 1;

#if NVT_WIFI   /* WiFi */
                    mcu_dp_bool_update(DPID_SWITCH,s_ledFlag);
#endif

                    if (g_ModbusIsEnabled == 1 && g_ModbusLedIsEnabled == 1)
                    {
                        if (s_ledFlag == 0)
                        {
                            if (DeviceLedOff(2) == 0)
                                g_ModbusLedIsEnabled = 1;
                            else
                                g_ModbusLedIsEnabled = 0;
                        }
                        else
                        {
                            if (DeviceLedOn(2) == 0)
                                g_ModbusLedIsEnabled = 1;
                            else
                                g_ModbusLedIsEnabled = 0;
                        }
                    }
                }

                if (Id == (GUI_ID_BUTTON0+8))   /* menu2 temp1 */
                    _CreateSelectedMenu2Temp1(Id - GUI_ID_BUTTON0, pMsg->hWin);

                if (Id == (GUI_ID_BUTTON0+9))   /* menu3 date1 */
                    _CreateSelectedMenu3Date1(Id - GUI_ID_BUTTON0, pMsg->hWin);

                IsPressed = 0;
            }
            break;
        }
        break;
    case WM_MOTION:
        pInfo = (WM_MOTION_INFO *)pMsg->Data.p;
        if (pInfo->FinalMove == 0)
            g_moveflag = 0;
        else
        {
            g_moveflag = 1;
            g_movexpos = pInfo->xPos;
        }
        switch (pInfo->Cmd)
        {
        case WM_MOTION_MOVE:
            //
            // Button should not react if window is moving
            //
            IsPressed = 0;
            break;
        case WM_MOTION_INIT:
            //
            // Activate snapping
            //
            pInfo->SnapX = 480;
            pInfo->Period = 200;
            break;
        }
        break;
    case WM_CREATE:
        hImage1 = IMAGE_CreateEx(0, 0, 480, 480 - 60, hWin, WM_CF_SHOW, IMAGE_CF_AUTOSIZE, GUI_ID_IMAGE0);
//        IMAGE_SetBitmap(hImage, &bmmenu1);
        u32FileSize = _BMP_Decode2("C:\\Application\\Menu1\\menu1.dta", s_au8BMP1);
        IMAGE_SetDTA(hImage1, (const void *)s_au8BMP1, u32FileSize);
//        IMAGE_SetJPEG(hImage1, (const void *)s_au8BMP1, u32FileSize);

        hImage2 = IMAGE_CreateEx(480, 0, 480, 480 - 60, hWin, WM_CF_SHOW, IMAGE_CF_AUTOSIZE, GUI_ID_IMAGE1);
//        IMAGE_SetBitmap(hImage, &bmmenu2);
        u32FileSize = _BMP_Decode2("C:\\Application\\Menu2\\menu2.dta", s_au8BMP2);
        IMAGE_SetDTA(hImage2, (const void *)s_au8BMP2, u32FileSize);
//        IMAGE_SetJPEG(hImage2, (const void *)s_au8BMP2, u32FileSize);

        hImage3 = IMAGE_CreateEx(480 + 480, 0, 480, 480 - 60, hWin, WM_CF_SHOW, IMAGE_CF_AUTOSIZE, GUI_ID_IMAGE2);
//        IMAGE_SetBitmap(hImage, &bmmenu3);
        u32FileSize = _BMP_Decode2("C:\\Application\\Menu3\\menu3.dta", s_au8BMP3);
        IMAGE_SetDTA(hImage3, (const void *)s_au8BMP3, u32FileSize);
//        IMAGE_SetJPEG(hImage3, (const void *)s_au8BMP3, u32FileSize);

        CALENDAR_SetDefaultDays(_apDays);
        CALENDAR_SetDefaultMonths(_apMonths);
        CALENDAR_SetDefaultBkColor(CALENDAR_CI_WEEKEND, GUI_LIGHTRED);
        CALENDAR_SetDefaultBkColor(CALENDAR_CI_WEEKDAY, GUI_LIGHTGRAY);
        CALENDAR_SetDefaultColor(CALENDAR_CI_WEEKDAY, GUI_BLUE);
        CALENDAR_SetDefaultColor(CALENDAR_CI_WEEKEND, GUI_YELLOW);
        CALENDAR_SetDefaultColor(CALENDAR_CI_LABEL, GUI_BLACK);
        CALENDAR_SetDefaultSize(CALENDAR_SI_CELL_X, 48);
        CALENDAR_SetDefaultSize(CALENDAR_SI_CELL_Y, 32);
        CALENDAR_SetDefaultSize(CALENDAR_SI_HEADER, 48);
        CALENDAR_SetDefaultFont(CALENDAR_FI_CONTENT, GUI_FONT_24B_ASCII);
        CALENDAR_SetDefaultFont(CALENDAR_FI_HEADER, GUI_FONT_COMIC18B_1);
        s_hCal = CALENDAR_Create(hWin, 480 + 480 + 100, 64, g_sCurTime.u32Year, g_sCurTime.u32cMonth, g_sCurTime.u32cDay, 1, GUI_ID_CALENDAR0, WM_CF_SHOW);

//        GUI_UC_SetEncodeUTF8();

//        s_iT1 = 25;
        s_hText1 = TEXT_CreateEx(480 + 160, 200, 171, 106, hWin, WM_CF_SHOW, 0, GUI_ID_TEXT0, s_achT1);
        TEXT_SetFont(s_hText1, GUI_FONT_D80);
        TEXT_SetTextColor(s_hText1, GUI_BLACK);

//        s_iT2 = 5;
        s_hText2 = TEXT_CreateEx(480 + 280, 250, 100, 56, hWin, WM_CF_SHOW, 0, GUI_ID_TEXT1, s_achT2);
        TEXT_SetFont(s_hText2, GUI_FONT_32B_1);
        TEXT_SetTextColor(s_hText2, GUI_BLACK);

        i = 0;  /* menu1 dehumidify */
        hButton = BUTTON_CreateUser(99, 105, 133, 148, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinCommon1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));
        i = 1;  /* menu1 fan */
        hButton = BUTTON_CreateUser(249, 103, 129, 148, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinCommon1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));
        i = 2;  /* menu1 cool */
        hButton = BUTTON_CreateUser(102, 264, 128, 147, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinCommon1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));
        i = 3;  /* menu1 heat */
        hButton = BUTTON_CreateUser(246, 263, 131, 147, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinCommon1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));
        i = 4;  /* menu1 mode1 */
        hButton = BUTTON_CreateUser(0, 0, 140, 80, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinCommon1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));

        i = 5;  /* menu2 up */
        hButton = BUTTON_CreateUser(480 + 384, 280, 96, 141, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinCommon1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));
        i = 6;  /* menu2 down */
        hButton = BUTTON_CreateUser(480 + 0, 280, 96, 141, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinCommon1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));
        i = 7;  /* menu2 onoff */
        hButton = BUTTON_CreateUser(480 + 184, 293, 98, 59, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinCommon1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));
        i = 8;  /* menu2 temp1 */
        hButton = BUTTON_CreateUser(480 + 0, 0, 207, 86, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinCommon1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));

        i = 9;  /* menu3 date1 */
        hButton = BUTTON_CreateUser(480 + 480 + 0, 0, 188, 100, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
        BUTTON_SetSkin(hButton, _ButtonSkinCommon1);
        BUTTON_SetUserData(hButton, &i, sizeof(i));
        break;
//    case WM_PRE_PAINT:
//        GUI_MULTIBUF_Begin();
//    break;
//    case WM_POST_PAINT:
//        GUI_MULTIBUF_End();
//    break;
//    case WM_PAINT:
//        break;
    default:
        WM_DefaultProc(pMsg);
    }
}

/*********************************************************************
*
*       _cbDummy
*
* Purpose:
*   Required for base window and viewport window to make sure WM_DefaultProc() is called.
*/
static void _cbDummy(WM_MESSAGE * pMsg)
{
    switch (pMsg->MsgId)
    {
    default:
        WM_DefaultProc(pMsg);
    }
}

/*********************************************************************
*
*       MainTask
*
* Purpose:
*
*/
void MainTask(void)
{
    int xSize, ySize;
    WM_HWIN hWinBase;     // Parent window for heading and viewport
    WM_HWIN hWinViewport; // Viewport window

#ifdef WIN32
#else
    RTC_Init();
    RTC_TimeDisplay();

    s_u8BLValue = 100;

    /* Enable PWM clock */
    PWM_Open();
    sPt.u8Mode = PWM_TOGGLE_MODE;
    sPt.fFrequency = 1000;
    sPt.u8HighPulseRatio = s_u8BLValue; /* High Pulse period : Total Pulse period = 1 : 100 */
    sPt.bInverter = FALSE;
    /* Set PWM Timer 0 Configuration */
    PWM_SetTimerClk(PWM_TIMER0,&sPt);
    /* Enable Output for PWM Timer 0 */
    PWM_SetTimerIO(PWM_TIMER0,TRUE);
    /* Enable the PWM Timer 0 */
    PWM_Enable(PWM_TIMER0,TRUE);
#endif

    g_ModbusTempIsEnabled = 1;
    g_ModbusLedIsEnabled = 1;
    g_ModbusMeterIsEnabled = 1;

    WM_SetCreateFlags(WM_CF_MEMDEV);

    GUI_Init();

//    GUI_JPEG_SetpfDrawEx(JPEG_X_Draw);

    WM_MULTIBUF_Enable(1);
    GUI_MEMDEV_MULTIBUF_Enable(1);

    WM_MOTION_Enable(1);
    BUTTON_SetReactOnLevel();
    //
    // Get display dimension
    //
    xSize = LCD_GetXSize();
    ySize = LCD_GetYSize();
    //
    // Limit desktop window to display size
    //
    WM_SetSize(WM_HBKWIN, xSize, ySize);
    xSize = 480;
    ySize = 480;

#if 0   /* WiFi */
    GUI_SetBkColor(GUI_BLACK);
    GUI_Clear();
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_D80);

    wifi_protocol_init();

    while (1)
    {
        GUI_DispDecAt(wifi_connected, 0, 0, 1);
        GUI_Delay(100);
        if ((wifi_connected == 0) || (wifi_connected == 1) || (wifi_connected == 2) || (wifi_connected == 3) || (wifi_connected == 4))
        {
            wifi_uart_service();
            mcu_get_wifi_connect_status();
        }
        else
        {
            wifi_connected = 0;
            mcu_reset_wifi();
        }
    }
#endif

    //
    // Create windows
    //
    hWinBase     = WM_CreateWindow       (     0,  0,     xSize,      ySize,               WM_CF_SHOW,                                               _cbDummy,   0);
    hWinViewport = WM_CreateWindowAsChild(     0, 60,     xSize, ySize - 60,     hWinBase, WM_CF_SHOW,                                               _cbDummy,   0);
                   WM_CreateWindowAsChild(     0,  0,     xSize,         60,     hWinBase, WM_CF_SHOW,                                               _cbHeading, 0);
    g_hWinMenu   = WM_CreateWindowAsChild(-xSize,  0, xSize * 3, ySize - 60, hWinViewport, WM_CF_SHOW | WM_CF_MOTION_X | WM_MOTION_MANAGE_BY_WINDOW, _cbMenu,    0);

//    sysprintf("Run Modbus\n");
//    pUartDevISR = &RS485Uart;
    //
    // Keep demo alive
    //
//  while (hWinMenu && hWinHeading) {

#if NVT_WIFI   /* WiFi */
    wifi_protocol_init();
#endif

    while(1)
    {
        GUI_Exec();

#if NVT_WIFI   /* WiFi */
        wifi_uart_service();
        mcu_get_wifi_connect_status();
        if(wifi_connected == 4 && g_RTCDateflag == 0)
            mcu_get_system_time();
#endif

    }
}

/*************************** End of file ****************************/
