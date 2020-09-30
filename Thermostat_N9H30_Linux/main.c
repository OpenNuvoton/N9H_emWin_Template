/****************************************************************************
 *                                                                          *
 * Copyright (c) 2018 Nuvoton Technology Corp. All rights reserved.         *
 *                                                                          *
 ***************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *     main.c
 *
 * VERSION
 *     1.1
 *
 * DESCRIPTION
 *     To utilize emWin library to demonstrate interactive feature.
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     None
 *
 * HISTORY
 *     2019/01/08        Ver 1.1 Updated
 *
 * REMARK
 *     None
 **************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/time.h>

#include <pthread.h>  // Thread

#include "GUI.h"
//#include "LCDConf.h"
#include "WM.h"

/*IOCTLs*/
//#define IOCTLSETCURSOR            _IOW('v', 21, unsigned int) //set cursor position
#define VIDEO_ACTIVE_WINDOW_COORDINATES     _IOW('v', 22, unsigned int) //set display-start line in display buffer
//#define IOCTLREADSTATUS           _IOW('v', 23, unsigned int) //read lcd module status
#define VIDEO_DISPLAY_ON            _IOW('v', 24, unsigned int) //display on
#define VIDEO_DISPLAY_OFF           _IOW('v', 25, unsigned int) //display off
//#define IOCTLCLEARSCREEN      _IOW('v', 26, unsigned int) //clear screen
//#define VIDEO_UPSCALING           _IOW('v', 27, unsigned int) //video up scaling
#define IOCTL_LCD_BRIGHTNESS        _IOW('v', 27, unsigned int)  //brightness control   



#define VIDEO_DISPLAY_LCD           _IOW('v', 38, unsigned int) //display on
#define VIDEO_DISPLAY_TV            _IOW('v', 39, unsigned int) //display off
#define VIDEO_FORMAT_CHANGE         _IOW('v', 50, unsigned int) //frame buffer format change
#define VIDEO_TV_SYSTEM             _IOW('v', 51, unsigned int) //set TV NTSC/PAL system 

//#define LCD_RGB565_2_RGB555       _IOW('v', 30, unsigned int) //RGB565_2_RGB555
//#define LCD_RGB555_2_RGB565       _IOW('v', 31, unsigned int) //RGB555_2_RGB565

#define LCD_RGB565_2_RGB555     _IO('v', 30)
#define LCD_RGB555_2_RGB565     _IO('v', 31)

#define LCD_ENABLE_INT      _IO('v', 28)
#define LCD_DISABLE_INT     _IO('v', 29)

//#define IOCTL_LCD_GET_DMA_BASE          _IOR('v', 32, unsigned int *)

#define DISPLAY_MODE_RGB555 0
#define DISPLAY_MODE_RGB565 1
#define DISPLAY_MODE_CBYCRY 4
#define DISPLAY_MODE_YCBYCR 5
#define DISPLAY_MODE_CRYCBY 6
#define DISPLAY_MODE_YCRYCB 7
/* Macros about LCM */
#define CMD_DISPLAY_ON                      0x3F
#define CMD_DISPLAY_OFF                     0x3E
#define CMD_SET_COL_ADDR                    0x40
#define CMD_SET_ROW_ADDR                    0xB8
#define CMD_SET_DISP_START_LINE     0xC0


#define CHAR_WIDTH      5

static struct fb_var_screeninfo var;
unsigned char *pVideoBuffer;
//unsigned char *g_VAFrameBuf;
int g_xres;
int g_yres;
int g_bits_per_pixel;

typedef struct Cursor
{
    unsigned char x;
    unsigned char y;
} Cursor;

typedef struct
{
    unsigned int start;
    unsigned int end;
} ActiveWindow;

typedef struct
{
    int hor;
    int ver;
} video_scaling;

typedef  struct _font
{
    unsigned char c[CHAR_WIDTH];
} font;

/* Dot-matrix of 0,1,2,3,4,5,6,7,8,9 */
font myFont[11] = {{0x3e, 0x41, 0x41, 0x3e, 0x00}, //zero
    {0x00, 0x41, 0x7f, 0x40, 0x00}, //un
    {0x71, 0x49, 0x49, 0x46, 0x00}, //deux
    {0x49, 0x49, 0x49, 0x36, 0x00}, //trois
    {0x0f, 0x08, 0x08, 0x7f, 0x00}, //quatre
    {0x4f, 0x49, 0x49, 0x31, 0x00}, //cinq
    {0x3e, 0x49, 0x49, 0x32, 0x00}, //six
    {0x01, 0x01, 0x01, 0x7f, 0x00}, //sept
    {0x36, 0x49, 0x49, 0x36, 0x00}, //huit
    {0x06, 0x49, 0x49, 0x3e, 0x00}, //neuf
    {0x00, 0x00, 0x60, 0x00, 0x00}
}; // point

WM_HWIN CreateFramewin(void);
extern void MainTask(void);
extern void TouchTask(void);
extern void MainTask2(void);
void *MainTask_ISR(void *arg)
{
#if 0
    printf("Main Task thread\n");

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
#else
    MainTask2();
#endif
}

void *TouchTask_ISR(void *arg)
{
    printf("Touch Task thread\n");
    TouchTask();
}

extern int SettingWiFiFlag;

U8 g_chSSID[1024];
U8 g_chPSK[1024];
extern int g_SettingWiFiFlag;
extern int SettingEthernetFlag;
char s_acTemp[4096];

U8 g_au8Eth0IP[32];
U8 g_au8WiFiIP[32];
U8 g_au8WiFiSignal[8];

static int s_i32WiFiFlag;

U8 g_au8WiFiList[1024];

extern void NVT_SetWiFiList(void);

int g_SettingEthernetFlag;

static int s_i32Eth0DhcpFlag;
static int s_i32WifiDhcpFlag;

void NetworkTask(void)
{
    int i, j, k, l;
    FILE *pstream_eth0;
    FILE *pstream_wlan0;
    FILE *pstream_wifi;
    FILE *pstream_wifi_status;
    FILE *pstream_dhcp;
    FILE *pstream;
    char acTemp[4096];
    int i32aTempSize;

    i32aTempSize = sizeof(acTemp);
    printf("Network Task start %d\n", i32aTempSize);

__RECONNECT_WIFI__:
    memset(acTemp, 0x00, i32aTempSize);
    pstream_wifi_status = popen("wpa_cli -i wlan0 disconnect", "r");
    fread(acTemp, 1, i32aTempSize, pstream_wifi_status);
    printf("### 001 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
    pclose(pstream_wifi_status);

    memset(acTemp, 0x00, i32aTempSize);
    pstream_wifi_status = popen("wpa_cli -i wlan0 terminate", "r");
    fread(acTemp, 1, i32aTempSize, pstream_wifi_status);
    printf("### 002 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
    pclose(pstream_wifi_status);

    memset(acTemp, 0x00, i32aTempSize);
    pstream_wlan0 = popen("ifconfig wlan0 down", "r");
    fread(acTemp, 1, i32aTempSize, pstream_wlan0);
    printf("### 003 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
    pclose(pstream_wlan0);

    memset(acTemp, 0x00, i32aTempSize);
    pstream_wlan0 = popen("ifconfig wlan0 up", "r");
    fread(acTemp, 1, i32aTempSize, pstream_wlan0);
    printf("### 004 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
    pclose(pstream_wlan0);

    memset(acTemp, 0x00, i32aTempSize);
    pstream_wifi = popen("wpa_supplicant -D nl80211 -i wlan0 -c /tmp/wpa_supplicant.conf -B", "r");
    fread(acTemp, 1, i32aTempSize, pstream_wifi);
    printf("### 005 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
    pclose(pstream_wifi);

    i = 0;
    s_i32WiFiFlag = 0;
    g_SettingEthernetFlag = 1;
    g_SettingWiFiFlag = 1;
    SettingWiFiFlag = 0;
    s_i32Eth0DhcpFlag = 0;
    s_i32WifiDhcpFlag = 0;

    while (1)
    {
        if ((SettingEthernetFlag == 1) && (g_SettingEthernetFlag == 0))
        {
            g_SettingEthernetFlag = 1;

            memset(acTemp, 0x00, i32aTempSize);
            pstream_eth0 = popen("ifconfig eth0 up", "r");
            fread(acTemp, 1, i32aTempSize, pstream_eth0);
            printf("### 007 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
            pclose(pstream_eth0);

            s_i32Eth0DhcpFlag = 1;
        }
        else if ((SettingEthernetFlag == 0) && (g_SettingEthernetFlag == 0))
        {
            g_SettingEthernetFlag = 1;

            memset(acTemp, 0x00, i32aTempSize);
            pstream_eth0 = popen("ifconfig eth0 down", "r");
            fread(acTemp, 1, i32aTempSize, pstream_eth0);
            printf("### 013 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
            pclose(pstream_eth0);
        }

        if ((SettingWiFiFlag == 1) && (g_SettingWiFiFlag == 0))
        {
            goto __RECONNECT_WIFI__;
        }
        else if ((SettingWiFiFlag == 0) && (g_SettingWiFiFlag == 0))
        {
            g_SettingWiFiFlag = 1;

            memset(acTemp, 0x00, i32aTempSize);
            pstream_wifi_status = popen("wpa_cli -i wlan0 disconnect", "r");
            fread(acTemp, 1, i32aTempSize, pstream_wifi_status);
            printf("### 001a 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
            pclose(pstream_wifi_status);
        
            memset(acTemp, 0x00, i32aTempSize);
            pstream_wifi_status = popen("wpa_cli -i wlan0 terminate", "r");
            fread(acTemp, 1, i32aTempSize, pstream_wifi_status);
            printf("### 002a 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
            pclose(pstream_wifi_status);
        
            memset(acTemp, 0x00, i32aTempSize);
            pstream_wlan0 = popen("ifconfig wlan0 down", "r");
            fread(acTemp, 1, i32aTempSize, pstream_wlan0);
            printf("### 003a 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
            pclose(pstream_wlan0);
        }

        if (g_SettingWiFiFlag == 2)
        {
            printf("### 014 %d ###\n", g_SettingWiFiFlag);

            sprintf(s_acTemp, "ctrl_interface=/var/run/wpa_supplicant\nap_scan=1\n\nnetwork={\n  ssid=\"%s\"\n  key_mgmt=WPA-PSK\n  proto=WPA2\n  psk=\"%s\"\n}\n\0", g_chSSID, g_chPSK);

            pstream = fopen("/tmp/wpa_supplicant.conf", "wb");
            fwrite(s_acTemp, 1, strlen(s_acTemp), pstream);
            fclose(pstream);

            goto __RECONNECT_WIFI__;
        }

        memset(acTemp, 0x00, i32aTempSize);
        pstream_wifi_status = popen("wpa_cli -i wlan0 status|grep wpa_state=|cut -f2 -d=", "r");
        fread(acTemp, 1, i32aTempSize, pstream_wifi_status);
        printf("### 015 0x%x 0x%x %d %d =%s###\n", acTemp[0], acTemp[1], i, strlen(acTemp), acTemp);
        pclose(pstream_wifi_status);

        if (strlen(acTemp) > 0)
            acTemp[strlen(acTemp) - 1] = 0x00;

        if (strcmp("COMPLETED", acTemp) == 0)
        {
            i = 0;

            if (s_i32WiFiFlag == 0)
            {
                s_i32WiFiFlag = 1;
                s_i32WifiDhcpFlag = 1;
            }
        }
        else
        {
            i++;
            if (i == 50)
                goto __RECONNECT_WIFI__;
        }

        memset(acTemp, 0x00, i32aTempSize);
        pstream_wifi_status = popen("wpa_cli -i wlan0 signal_poll|grep RSSI=|cut -f2 -d=", "r");
        fread(acTemp, 1, i32aTempSize, pstream_wifi_status);
        //printf("### 020 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
        pclose(pstream_wifi_status);
        // FIXME
        g_au8WiFiSignal[0] = acTemp[1];

        memset(acTemp, 0x00, i32aTempSize);
        pstream_wifi_status = popen("wpa_cli -i wlan0 scan", "r");
        fread(acTemp, 1, i32aTempSize, pstream_wifi_status);
        //printf("### 021 0x%x 0x%x %d =%s###\n", acTemp[0], acTemp[1], strlen(acTemp), acTemp);
        pclose(pstream_wifi_status);

        if (strlen(acTemp) > 0)
            acTemp[strlen(acTemp) - 1] = 0x00;

        if (strcmp("OK", acTemp) == 0)
        {
            memset(acTemp, 0x00, i32aTempSize);
            pstream_wifi_status = popen("wpa_cli -i wlan0 scan_results|grep ':'|awk -F '\t' '{print $5}'", "r");
            fread(acTemp, 1, i32aTempSize, pstream_wifi_status);
            //printf("### 022 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
            pclose(pstream_wifi_status);

            if (strlen(acTemp) > 0)
            {
                acTemp[strlen(acTemp) - 1] = 0x00;
                strcpy(g_au8WiFiList, acTemp);
            }
        }

        usleep(1000000);
    }
}


void *NetworkTask_ISR(void *arg)
{
    printf("Network Task thread\n");
    NetworkTask();
}

void *NetworkTask2_ISR(void *arg)
{
    FILE *pstream_dhcp;
    char acTemp[4096];
    int i32aTempSize;

    i32aTempSize = sizeof(acTemp);

    printf("Network Task2 thread\n");
    for(;;)
    {
        if (s_i32Eth0DhcpFlag == 1)
        {
            s_i32Eth0DhcpFlag = 0;

            memset(acTemp, 0x00, i32aTempSize);
            pstream_dhcp = popen("udhcpc -i eth0 -q|grep 'adding'|awk '/adding/ {print $2}'", "r");
            // FIXME, block in read until plug-in
            fread(acTemp, 1, i32aTempSize, pstream_dhcp);
            printf("### 008 0x%x 0x%x %d =%s###\n", acTemp[0], acTemp[1], strlen(acTemp), acTemp);
            pclose(pstream_dhcp);

            if (strlen(acTemp) > 4)
                acTemp[strlen(acTemp) - 5] = 0x00;
            else if (strlen(acTemp) > 0)
                acTemp[strlen(acTemp) - 1] = 0x00;

            if (strcmp("dns", acTemp) == 0)
            {
                printf("### 009 %s found ###\n", acTemp);

                memset(acTemp, 0x00, i32aTempSize);
                pstream_dhcp = popen("ifconfig eth0|grep 'inet addr:'|awk '/192/ {print $2}'|sed s/addr://", "r");
                fread(acTemp, 1, i32aTempSize, pstream_dhcp);
                printf("### 010 %d eip=%s###\n", strlen(acTemp), acTemp);
                pclose(pstream_dhcp);

                if (strlen(acTemp) > 0)
                {
                    acTemp[strlen(acTemp) - 1] = 0x00;
                    strcpy(g_au8Eth0IP, acTemp);
                }

                SettingEthernetFlag = 1;
                g_SettingEthernetFlag = 1;
                printf("### 011 eip=%s ###\n", acTemp);
            }
        }

        usleep(1000000);  //delay 1000 ms
    }
}

void *NetworkTask3_ISR(void *arg)
{
    FILE *pstream_dhcp;
    char acTemp[4096];
    int i32aTempSize;

    i32aTempSize = sizeof(acTemp);

    printf("Network Task3 thread\n");
    for(;;)
    {
        if (s_i32WifiDhcpFlag == 1)
        {
            s_i32WifiDhcpFlag = 0;

            memset(acTemp, 0x00, i32aTempSize);
            // FIXME may block here
            pstream_dhcp = popen("udhcpc -i wlan0 -q|grep 'adding'|awk '/adding/ {print $2}'", "r");
            fread(acTemp, 1, i32aTempSize, pstream_dhcp);
            printf("### 016 0x%x 0x%x %d =%s###\n", acTemp[0], acTemp[1], strlen(acTemp), acTemp);
            pclose(pstream_dhcp);

            if (strlen(acTemp) > 4)
                acTemp[strlen(acTemp) - 5] = 0x00;
            else if (strlen(acTemp) > 0)
                acTemp[strlen(acTemp) - 1] = 0x00;

            if (strcmp("dns", acTemp) == 0)
            {
                printf("### 017 %s found ###\n", acTemp);

                memset(acTemp, 0x00, i32aTempSize);
                pstream_dhcp = popen("ifconfig wlan0|grep 'inet addr:'|awk '/192/ {print $2}'|sed s/addr://", "r");
                fread(acTemp, 1, i32aTempSize, pstream_dhcp);
                printf("### 018 %d wip=%s###\n", strlen(acTemp), acTemp);
                pclose(pstream_dhcp);

                if (strlen(acTemp) > 0)
                {
                    acTemp[strlen(acTemp) - 1] = 0x00;
                    strcpy(g_au8WiFiIP, acTemp);
                }

                SettingWiFiFlag = 1;
                g_SettingEthernetFlag = 1;
                printf("### 019 wip=%s ###\n", acTemp);
            }
        }

        usleep(1000000);  //delay 1000 ms
    }
}

void *NetworkTask4_ISR(void *arg)
{
    FILE *pstream_ntpdate;
    char acTemp[4096];
    int i32aTempSize;

    i32aTempSize = sizeof(acTemp);

    printf("Network Task4 thread\n");
    for(;;)
    {
        usleep(10000000);  //delay 10000 ms
#if 1   // FIXME
            memset(acTemp, 0x00, i32aTempSize);
            pstream_ntpdate = popen("ntpdate pool.ntp.org", "r");
            fread(acTemp, 1, i32aTempSize, pstream_ntpdate);
            printf("### 023 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
            pclose(pstream_ntpdate);
#endif
    }
}

extern U8 g_au8CITY[];
extern U8 g_au8Temp1[];
extern U8 g_au8TempF1[];
char g_au8CITYWeather[1024];
I32 g_i32WeatherSmallFlag;
I32 g_i32WeatherBigFlag;

void *NetworkTask5_ISR(void *arg)
{
    FILE *pstream_curl;
    char acTemp[4096];
    int i32aTempSize;

    i32aTempSize = sizeof(acTemp);

    printf("Network Task5 thread\n");
    for(;;)
    {
        memset(acTemp, 0x00, i32aTempSize);
        pstream_curl = popen("cat /tmp/test1.txt|grep 'Weather report: '|awk -F ': ' '{print $2}'", "r");
        fread(acTemp, 1, i32aTempSize, pstream_curl);
        printf("### city 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
        pclose(pstream_curl);

        memset(acTemp, 0x00, i32aTempSize);
        pstream_curl = popen("cat /tmp/test1.txt|grep -E -o '[A-Z].+[a-z]'|sed -E '/Weather report/d'", "r");
        fread(acTemp, 1, i32aTempSize, pstream_curl);
        printf("### weather 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
        pclose(pstream_curl);

        memset(acTemp, 0x00, i32aTempSize);
        pstream_curl = popen("cat /tmp/test1.txt|grep -E -o '[-]?[0-9]+\\.\\.'|sed -E 's/\\.\\.//g'", "r");
        fread(acTemp, 1, i32aTempSize, pstream_curl);
        printf("### degree real 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
        pclose(pstream_curl);

        if (strlen(acTemp) > 0)
        {
            acTemp[strlen(acTemp) - 1] = 0x00;
            strcpy(g_au8Temp1, acTemp);
        }

        memset(acTemp, 0x00, i32aTempSize);
        pstream_curl = popen("cat /tmp/test1.txt|grep -E -o '\\.\\.[-]?[0-9]+'|sed -E 's/\\.\\.//g'", "r");
        fread(acTemp, 1, i32aTempSize, pstream_curl);
        printf("### degree feels 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
        pclose(pstream_curl);

        if (strlen(acTemp) > 0)
        {
            acTemp[strlen(acTemp) - 1] = 0x00;
            strcpy(g_au8TempF1, acTemp);
        }

        memset(acTemp, 0x00, i32aTempSize);
        pstream_curl = popen("cat /tmp/test1.txt|grep -E -o $'\xE2\x86\x90'.+h", "r");
        fread(acTemp, 1, i32aTempSize, pstream_curl);
        printf("### wind left 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
        pclose(pstream_curl);

        memset(acTemp, 0x00, i32aTempSize);
        pstream_curl = popen("cat /tmp/test1.txt|grep -E -o $'\xE2\x86\x91'.+h", "r");
        fread(acTemp, 1, i32aTempSize, pstream_curl);
        printf("### wind up 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
        pclose(pstream_curl);

        memset(acTemp, 0x00, i32aTempSize);
        pstream_curl = popen("cat /tmp/test1.txt|grep -E -o $'\xE2\x86\x92'.+h", "r");
        fread(acTemp, 1, i32aTempSize, pstream_curl);
        printf("### wind right 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
        pclose(pstream_curl);

        memset(acTemp, 0x00, i32aTempSize);
        pstream_curl = popen("cat /tmp/test1.txt|grep -E -o $'\xE2\x86\x93'.+h", "r");
        fread(acTemp, 1, i32aTempSize, pstream_curl);
        printf("### wind down 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
        pclose(pstream_curl);

        memset(acTemp, 0x00, i32aTempSize);
        pstream_curl = popen("cat /tmp/test1.txt|grep -E -o $'\xE2\x86\x96'.+h", "r");
        fread(acTemp, 1, i32aTempSize, pstream_curl);
        printf("### wind left+up 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
        pclose(pstream_curl);

        memset(acTemp, 0x00, i32aTempSize);
        pstream_curl = popen("cat /tmp/test1.txt|grep -E -o $'\xE2\x86\x97'.+h", "r");
        fread(acTemp, 1, i32aTempSize, pstream_curl);
        printf("### wind right+up 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
        pclose(pstream_curl);

        memset(acTemp, 0x00, i32aTempSize);
        pstream_curl = popen("cat /tmp/test1.txt|grep -E -o $'\xE2\x86\x98'.+h", "r");
        fread(acTemp, 1, i32aTempSize, pstream_curl);
        printf("### wind right+down 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
        pclose(pstream_curl);

        memset(acTemp, 0x00, i32aTempSize);
        pstream_curl = popen("cat /tmp/test1.txt|grep -E -o $'\xE2\x86\x99'.+h", "r");
        fread(acTemp, 1, i32aTempSize, pstream_curl);
        printf("### wind left+down 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
        pclose(pstream_curl);

        memset(acTemp, 0x00, i32aTempSize);
        pstream_curl = popen("cat /tmp/test1.txt|grep -E '[0-9]+ km'|sed '/h/d'|grep -E -o '[0-9]+ km'", "r");
        fread(acTemp, 1, i32aTempSize, pstream_curl);
        printf("### visi 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
        pclose(pstream_curl);

        memset(acTemp, 0x00, i32aTempSize);
        pstream_curl = popen("cat /tmp/test1.txt|grep -E -o '[0-9]+\\.[0-9]+ mm'", "r");
        fread(acTemp, 1, i32aTempSize, pstream_curl);
        printf("### rain 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
        pclose(pstream_curl);

        usleep(60000000);  //delay 60000 ms

        memset(acTemp, 0x00, i32aTempSize);
        sprintf(g_au8CITYWeather, "curl http://wttr.in/%s?0T > /tmp/test1.txt", g_au8CITY);
        pstream_curl = popen(g_au8CITYWeather, "r");
        fread(acTemp, 1, i32aTempSize, pstream_curl);
        //printf("### 024 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
        pclose(pstream_curl);

        g_i32WeatherSmallFlag = 0;

        memset(acTemp, 0x00, i32aTempSize);
        sprintf(g_au8CITYWeather, "curl http://wttr.in/%s_0q.png --output /tmp/w1.png", g_au8CITY);
        pstream_curl = popen(g_au8CITYWeather, "r");
        fread(acTemp, 1, i32aTempSize, pstream_curl);
        printf("### 025 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
        pclose(pstream_curl);

        g_i32WeatherSmallFlag = 1;

        g_i32WeatherBigFlag = 0;

        memset(acTemp, 0x00, i32aTempSize);
        sprintf(g_au8CITYWeather, "curl http://wttr.in/%s_3Fq.png --output /tmp/w2.png", g_au8CITY);
        pstream_curl = popen(g_au8CITYWeather, "r");
        fread(acTemp, 1, i32aTempSize, pstream_curl);
        printf("### 026 0x%x 0x%x =%s###\n", acTemp[0], acTemp[1], acTemp);
        pclose(pstream_curl);

        g_i32WeatherBigFlag = 1;
    }
}

int main()
{
    int fd, ret;
    int i, t = 0;
    FILE *pstream;
    char acTemp[4096];
    int i32aTempSize;

    FILE *fpVideoImg;

    unsigned long uVideoSize;

    pthread_t tid1, tid2, tid3, tid4, tid5, tid6, tid7;


    fd = open("/dev/fb0", O_RDWR);
    if (fd == -1)
    {
        printf("Cannot open fb0!\n");
        return -1;
    }

    if (ioctl(fd, FBIOGET_VSCREENINFO, &var) < 0)
    {
        perror("ioctl FBIOGET_VSCREENINFO");
        close(fd);
        return -1;
    }

//  ioctl(fd,LCD_ENABLE_INT);
    uVideoSize = var.xres * var.yres * var.bits_per_pixel / 8;

    printf("uVideoSize = 0x%x\n", uVideoSize);
    printf("var.xres = 0x%x\n", var.xres);
    printf("var.yres = 0x%x\n", var.yres);
    //  printf("uVideoSize = 0x%x \n", uVideoSize);
    pVideoBuffer = mmap(NULL, uVideoSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    printf("pVideoBuffer = 0x%x\n", pVideoBuffer);
    if(pVideoBuffer == MAP_FAILED)
    {
        printf("LCD Video Map Failed!\n");
        exit(0);
    }
    //ioctl(fd, IOCTL_LCD_GET_DMA_BASE, &g_VAFrameBuf);
    // the processing of video buffer
    //g_VAFrameBuf = pVideoBuffer;
    g_xres = var.xres;
    g_yres = var.yres;
    g_bits_per_pixel = var.bits_per_pixel;

    i32aTempSize = sizeof(acTemp);

    memset(acTemp, 0x00, i32aTempSize);
    pstream = popen("cp /mnt/conf/wpa_supplicant.conf /tmp", "r");
    fread(acTemp, 1, i32aTempSize, pstream);
    printf("### cp /mnt/conf/wpa_supplicant.conf /tmp ###\n");
    pclose(pstream);

    memset(acTemp, 0x00, i32aTempSize);
    pstream = popen("cp /mnt/png/w1.png /tmp", "r");
    fread(acTemp, 1, i32aTempSize, pstream);
    printf("### cp /mnt/png/w1.png /tmp ###\n");
    pclose(pstream);

    memset(acTemp, 0x00, i32aTempSize);
    pstream = popen("cp /mnt/png/w2.png /tmp", "r");
    fread(acTemp, 1, i32aTempSize, pstream);
    printf("### cp /mnt/png/w2.png /tmp ###\n");
    pclose(pstream);

    memset(acTemp, 0x00, i32aTempSize);
    pstream = popen("cp /mnt/conf/test1.txt /tmp", "r");
    fread(acTemp, 1, i32aTempSize, pstream);
    printf("### cp /mnt/conf/test1.txt /tmp ###\n");
    pclose(pstream);

    pthread_create(&tid1, NULL, MainTask_ISR, (void *)"MainTask");
    pthread_create(&tid2, NULL, TouchTask_ISR, (void *)"TouchTask");
    pthread_create(&tid3, NULL, NetworkTask_ISR, (void *)"NetworkTask");
    pthread_create(&tid4, NULL, NetworkTask2_ISR, (void *)"NetworkTask2");
    pthread_create(&tid5, NULL, NetworkTask3_ISR, (void *)"NetworkTask3");
    pthread_create(&tid6, NULL, NetworkTask4_ISR, (void *)"NetworkTask4");
    pthread_create(&tid7, NULL, NetworkTask5_ISR, (void *)"NetworkTask5");

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);
    pthread_join(tid4, NULL);
    pthread_join(tid5, NULL);
    pthread_join(tid6, NULL);
    pthread_join(tid7, NULL);

//    MainTask();

    /* Close LCD */
    //ioctl(fd, VIDEO_DISPLAY_OFF);
    munmap(pVideoBuffer, uVideoSize);  //return memory
    close(fd);
    return 0;
}
