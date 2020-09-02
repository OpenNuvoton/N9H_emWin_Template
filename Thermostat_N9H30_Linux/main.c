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

    usleep(10000000);

    printf("Network Task start\n");

    memset(acTemp, 0x00, 40);
    pstream_eth0 = popen("ifconfig eth0 up", "r");
    //fread(acTemp, 1, 40, pstream_eth0);
    usleep(1000000);
    pclose(pstream_eth0);

    #if 0 // FIXME
    memset(acTemp, 0x00, 40);
    pstream_dhcp = popen("udhcpc -i eth0 -q", "r");
    //fread(acTemp, 1, 40, pstream_wlan0);
    usleep(1000000);
    pclose(pstream_dhcp);
    #if 0
    memset(acTemp, 0x00, 1024);
    pstream = popen("ifconfig eth0|grep addr:|cut -f2 -d:", "r");
    fread(acTemp, 1, 1024, pstream);
    printf("### addr:%s ###\n", acTemp);
    pclose(pstream);
    #endif
    #endif

    while (1)
    {
    memset(acTemp, 0x00, 40);
    pstream_wlan0 = popen("ifconfig wlan0 up", "r");
    //fread(acTemp, 1, 40, pstream_wlan0);
    usleep(1000000);
    pclose(pstream_wlan0);

    memset(acTemp, 0x00, 1024);
    pstream_wifi = popen("wpa_supplicant -D nl80211 -i wlan0 -c /mnt/conf/wpa_supplicant.conf -B", "r");
    //fread(acTemp, 1, 1024, pstream_wifi);
    //pclose(pstream_wifi);
    usleep(1000000);

    i = 0;
    l = 0;
    k = 0;
    while (1)
    {
        if (SettingEthernetFlag == 1)
        {
            SettingEthernetFlag = 2;

            memset(acTemp, 0x00, 40);
            pstream_dhcp = popen("udhcpc -i eth0 -q", "r");
            //fread(acTemp, 1, 40, pstream_wlan0);
            usleep(1000000);
            pclose(pstream_dhcp);
        }
        memset(acTemp, 0x00, 1024);
        pstream_wifi_status = popen("wpa_cli -i wlan0 status|grep wpa_state=|cut -f2 -d=", "r");
        fread(acTemp, 1, 1024, pstream_wifi_status);
        usleep(1000000);
        //printf("#### %s", acTemp);
        j = strlen(acTemp);
        //printf("### acTemp=%d ###\n", j);
        acTemp[j-1] = 0x00;
        //j = strcmp("SCANNING", acTemp);
        //printf("### j=%d ###\n", j);
        if (strcmp("SCANNING", acTemp) == 0)
        {
            l = k = 0;
            printf("### %d SCANNING ###\n", i);
            i++;
__RECONNECT_WIFI__:
            if (i >= 10)
            {
                pclose(pstream_wifi_status);

                memset(acTemp, 0x00, 40);
                pstream_wifi_status = popen("wpa_cli -i wlan0 disconnect", "r");
                //fread(acTemp, 1, 40, pstream_wlan0);
                usleep(1000000);
                pclose(pstream_wifi_status);

                memset(acTemp, 0x00, 40);
                pstream_wifi_status = popen("wpa_cli -i wlan0 terminate", "r");
                //fread(acTemp, 1, 40, pstream_wlan0);
                usleep(1000000);
                pclose(pstream_wifi_status);

                memset(acTemp, 0x00, 40);
                pstream_wlan0 = popen("ifconfig wlan0 down", "r");
                //fread(acTemp, 1, 40, pstream_wlan0);
                usleep(1000000);
                pclose(pstream_wlan0);
                pclose(pstream_wifi);
                break;
            }
        }
        else if (strcmp("COMPLETED", acTemp) == 0)
        {
            k++;
            if (k == 3)
            {
                memset(acTemp, 0x00, 40);
                pstream_dhcp = popen("udhcpc -i wlan0 -q", "r");
                //fread(acTemp, 1, 40, pstream_wlan0);
                usleep(1000000);
                pclose(pstream_dhcp);

                memset(acTemp, 0x00, 128);
                pstream_dhcp = popen("ping -c 4 8.8.8.8", "r");
                fread(acTemp, 1, 128, pstream_dhcp);
                pclose(pstream_dhcp);
                if ((acTemp[0] == 0x00) || (acTemp[0] == 0x0A))
                {
                    i = 10;
                    k = l = 0;
                    goto __RECONNECT_WIFI__;
                }
                else
                    printf("### ping=%s ###\n", acTemp);

                memset(acTemp, 0x00, 40);
                pstream_dhcp = popen("ntpdate tw.pool.ntp.org", "r");
                //fread(acTemp, 1, 40, pstream_wlan0);
                usleep(1000000);
                pclose(pstream_dhcp);

                SettingWiFiFlag = 1;
            }
            if ((k % 10) == 0)
            {
                memset(acTemp, 0x00, 128);
                pstream_dhcp = popen("ping -c 4 8.8.8.8|grep seq", "r");
                fread(acTemp, 1, 128, pstream_dhcp);
                pclose(pstream_dhcp);
                if ((acTemp[0] == 0x00) || (acTemp[0] == 0x0A))
                {
                    i = 10;
                    k = l = 0;
                    goto __RECONNECT_WIFI__;
                }
                else
                    printf("### ping=%s ###\n", acTemp);
            }
            printf("### COMPLETED ###\n");
            #if 0    // FIXME
            if (SettingWiFiFlag2 == 1)
            {
                SettingWiFiFlag2 = 0;
                i = 10;

                goto __RECONNECT_WIFI__;
            }

            if (SettingEthernetFlag2 == 1)
            {
                SettingEthernetFlag2 = 0;

                pstream = popen("ifconfig eth0 down", "r");
                usleep(1000000);
                pclose(pstream);
            }
            #endif
            if (g_SettingWiFiFlag == 1)
            {
                g_SettingWiFiFlag = 0;

                memset(acTemp, 0x00, 40);
                pstream_wifi_status = popen("wpa_cli -i wlan0 disconnect", "r");
                //fread(acTemp, 1, 40, pstream_wlan0);
                usleep(1000000);
                pclose(pstream_wifi_status);

                memset(acTemp, 0x00, 40);
                pstream_wifi_status = popen("wpa_cli -i wlan0 terminate", "r");
                //fread(acTemp, 1, 40, pstream_wlan0);
                usleep(1000000);
                pclose(pstream_wifi_status);

                sprintf(s_acTemp, "ctrl_interface=/var/run/wpa_supplicant\nap_scan=1\n\nnetwork={\n  ssid=\"%s\"\n  key_mgmt=WPA-PSK\n  proto=WPA2\n  psk=\"%s\"\n}\n\0", g_chSSID, g_chPSK);

                pstream = fopen("/mnt/conf/wpa_supplicant.conf", "wb");
                fwrite(s_acTemp, 1, strlen(s_acTemp), pstream);
                fclose(pstream);
                i = l = k = 0;
            }
        }
        else
        {
            printf("#### l=%s %d ###\n", acTemp, l);
            l++;
            if (l >= 10)
            {
                l=0;
                #if 1 // FIXME
                i = 10;
                k = 0;
                goto __RECONNECT_WIFI__;
                #endif
                break;
            }
        }
        pclose(pstream_wifi_status);
    }
    }
#if 0
    memset(acTemp, 0x00, 40);
    pstream2 = popen("udhcpc -i wlan0", "r");
    //fread(acTemp, 1, 40, pstream);
    //pclose(pstream2);
//#if 0
    memset(acTemp, 0x00, 40);
    pstream3 = popen("ntpdate tw.pool.ntp.org", "r");
    //fread(acTemp, 1, 40, pstream);
    pclose(pstream3);
#endif
    printf("### all done ###\n");

    for (;;)
        usleep(1000000);  //delay 1000 ms
}


void *NetworkTask_ISR(void *arg)
{
    printf("Network Task thread\n");
    NetworkTask();
}

int main()
{
    int fd, ret;
    int i, t = 0;

    FILE *fpVideoImg;

    unsigned long uVideoSize;

    pthread_t tid1, tid2, tid3;


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

    pthread_create(&tid1, NULL, MainTask_ISR, (void *)"MainTask");
    pthread_create(&tid2, NULL, TouchTask_ISR, (void *)"TouchTask");
    pthread_create(&tid3, NULL, NetworkTask_ISR, (void *)"NetworkTask");

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);

//    MainTask();

    /* Close LCD */
    //ioctl(fd, VIDEO_DISPLAY_OFF);
    munmap(pVideoBuffer, uVideoSize);  //return memory
    close(fd);
    return 0;
}
