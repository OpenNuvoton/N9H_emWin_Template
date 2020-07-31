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
 *     1.0
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
 *     2018/09/27        Ver 1.0 Created
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

#include <sys/shm.h>
#include <errno.h>

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

#define IOCTL_LCD_GET_DMA_BASE          _IOR('v', 32, unsigned int *)

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

// OSD
#define IOCTL_GET_OSD_OFFSET    _IOR('v', 60, unsigned int *)
#define OSD_SEND_CMD            _IOW('v', 160, unsigned int *)

#define LCD_RGB565_2_RGB555		_IO('v', 30)
#define LCD_RGB555_2_RGB565		_IO('v', 31)

#define LCD_ENABLE_INT		_IO('v', 28)
#define LCD_DISABLE_INT		_IO('v', 29)

#define IOCTL_FB_LOCK			_IOW('v', 64, unsigned int)	
#define IOCTL_FB_UNLOCK			_IOW('v', 65, unsigned int)	
#define IOCTL_WAIT_VSYNC		_IOW('v', 67, unsigned int)

static struct fb_var_screeninfo var;
static struct fb_fix_screeninfo fix;

unsigned char *pVideoBuffer;
unsigned int g_VAFrameBuf;
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

// OSD 
typedef enum {
  // All functions return -2 on "not open"
  OSD_Close=1,    // ()
  // Disables OSD and releases the buffers (??)
  // returns 0 on success

  OSD_Open,       // (cmd + color_format)
  // Opens OSD with color format
  // returns 0 on success

  OSD_Show,       // (cmd)
  // enables OSD mode
  // returns 0 on success

  OSD_Hide,       // (cmd)
  // disables OSD mode
  // returns 0 on success

  OSD_Clear,      // (cmd )
  // clear OSD buffer with color-key color
  // returns 0 on success
  
  OSD_Fill,      // (cmd +)
  // clear OSD buffer with assigned color
  // returns 0 on success

  OSD_FillBlock,      // (cmd+X-axis)  
  // set OSD buffer with user color data (color data will be sent by "write()" function later
  // returns 0 on success
  
  OSD_SetTrans,   // (transparency{color})
  // Sets transparency color-key
  // returns 0 on success

  OSD_ClrTrans,   // (transparency{color})
  // Disable transparency color-key
  // returns 0 on success

  OSD_SetBlend,   // (blending{weight})
  // Enable alpha-blending and give weighting value, 0 - 0xFF (opaque)
  // returns 0 on success

  OSD_ClrBlend,   // (blending{weight})
  // Disable alpha-blending
  // returns 0 on success

} OSD_Command;

typedef enum {
	OSD_RGB555=0xAB08, 
  	OSD_RGB565,       
  	OSD_RGBx888,       
	OSD_RGB888x,  
	OSD_ARGB888,  	
  	OSD_Cb0Y0Cr0Y1=0xAB00,       
  	OSD_Y0Cb0Y1Cr0,       
  	OSD_Cr0Y0Cb0Y1,       
  	OSD_Y0Cr0Y1Cb0,       
  	OSD_Y1Cr0Y0Cb0,       
  	OSD_Cr0Y1Cb0Y0,       
  	OSD_Y1Cb0Y0Cr0,       
  	OSD_Cb0Y1Cr0Y0,       
} OSD_Format;


typedef struct osd_cmd_s {
	int cmd;
	int x0;	
	int y0;
	int x0_size;
	int y0_size;
	int color;		// color_format, color_key
	int alpha;		// alpha blending weight	
	int format;	
} osd_cmd_t;

static unsigned long make_color(int color)
{
    U32 r, g, b;

    r = (color & 0x00f80000) >> 8; // 5 bits
    g = (color & 0x0000fc00) >> 5; // 6 bits
    b = (color & 0x000000f8) >> 3; // 5 bits

    return (r | g | b);
}

#define NVT_BEGIN_MOVIE "_movie.avi"
#define NVT_BEGIN_HIDE 5000 // millisecond

extern WM_HWIN CreateWindow_Temp(void);
extern WM_HWIN CreateWindow_Setting(void);
extern WM_HWIN CreateWindow_Cooking(void);

extern void TouchTask(void);
extern void NVT_print_pos(void);

U32 g_u32Playing;
U32 g_u32InCookingMenu;

static WM_HWIN g_hWinPlay;

static FILE *s_stream;
static int t;
enum vendor_state
{
    VENDOR_STATE_UNKOWN = 0,
    VENDOR_STATE_PLAYING = 1,
    VENDOR_STATE_PAUSING = 2,
    VENDOR_STATE_IDLING = 3
};
static int m_pb_stat;  // playback state
static U32 m_pb_pos; // playback position in secs
static U32 m_pb_tot; // playback total time in secs
static char sBuf[1024];

void NVT_PlayFile(const char * FileName)
{
    snprintf(sBuf, sizeof(sBuf), "echo loadfile %s > /tmp/fifo_mplayer_cmd &", FileName);
    s_stream = popen( sBuf, "rw" );
    pclose( s_stream );
}

void NVT_GetPos(U32 Pos)
{
    snprintf(sBuf, sizeof(sBuf), "echo pausing_keep seek %d 1 > /tmp/fifo_mplayer_cmd", Pos);
    s_stream = popen( sBuf, "rw" );
    pclose( s_stream );
}

void NVT_Home(void)
{
    s_stream = popen( "echo stop > /tmp/fifo_mplayer_cmd", "rw" );
    pclose( s_stream );
}

void NVT_PlayPause(void)
{
    s_stream = popen( "echo p > /tmp/fifo_mplayer_cmd", "rw" );
    pclose( s_stream );
}

static void cbBackgroundWin(WM_MESSAGE *pMsg)
{
    if ((g_hWinPlay > 0) && (g_u32Playing == 1) && ((m_pb_stat == 1) || (m_pb_stat == 2)))
    {
        switch (pMsg->MsgId)
        {
        case WM_TOUCH:
            t = GUI_GetTime();
            WM_ShowWin(g_hWinPlay);
            break;
        case WM_PAINT:
            GUI_SetBkColor(GUI_MAGENTA);
            GUI_Clear();
            break;
        }
    }
    WM_DefaultProc(pMsg);
}

void *MainTask_ISR(void *arg)
{
    int t1;

    WM_HWIN hWinTEMP;
    WM_HWIN hWinSetting;
    WM_HWIN hWinCooking;

    printf("Main Task thread\n");
    //
    // FIXME use multiple buffers
    //
    WM_SetCreateFlags(WM_CF_MEMDEV);

    GUI_Init();

    WM_SetCallback(WM_HBKWIN, cbBackgroundWin);

    while (1)
    {
        //
        // FIXME need elegant 
        //
        g_u32Playing = 1;
        g_hWinPlay = CreateWindow();
        t = GUI_GetTime();
        while (g_u32Playing == 1)
        {
            GUI_Delay(500);
            NVT_print_pos();
            t1 = GUI_GetTime() - t;
            if ( (t1 >= NVT_BEGIN_HIDE) && (g_hWinPlay > 0) )
            {
                WM_HideWindow(g_hWinPlay);
                t = GUI_GetTime();
            }
        }
        GUI_Delay(100);
        WM_DeleteWindow(g_hWinPlay);
        g_hWinPlay = -1;

        while (1)
        {
            g_u32InCookingMenu = 1;
            hWinCooking = CreateWindow_Cooking();
            while (g_u32InCookingMenu == 1)
            {
                GUI_Delay(500);
            }

            if (g_u32InCookingMenu == 7)
            {
                WM_DeleteWindow(hWinCooking);
                hWinCooking = -1;
                hWinTEMP = CreateWindow_Temp();
                while (g_u32InCookingMenu == 7)
                    GUI_Delay(500);
                WM_DeleteWindow(hWinTEMP);
                hWinTEMP = -1;
            }
            else if (g_u32InCookingMenu == 8)
            {
                WM_DeleteWindow(hWinCooking);
                hWinCooking = -1;
                hWinSetting = CreateWindow_Setting();
                while (g_u32InCookingMenu == 8)
                    GUI_Delay(500);
                WM_DeleteWindow(hWinSetting);
                hWinSetting = -1;
            }
            else
            {
                //
                // FIXME need elegant 
                //
                NVT_print_pos();
                while (m_pb_stat==3)
                {
                    NVT_print_pos();
                    GUI_Delay(10);
                }
                GUI_Delay(100);
                WM_DeleteWindow(hWinCooking);
                hWinCooking = -1;
                break;
            }
        }
    }
}

void *TouchTask_ISR(void *arg)
{
    printf("Touch Task thread\n");
    TouchTask();
}

#define	MPLAYER_FIFO_PATH		"/tmp/fifo_mplayer_cmd"
#define	VENDOR_OSDCOUNT			0
#define	VENDOR_OSDSHMID			12345
#define	VENDOR_OSDSHMSIZE		1024

static U8 s_au8BMP[700 * 1024] __attribute__((aligned(32)));

typedef struct
{
    int i32Status;
    int i32Offset;
    int i32Size;
    int i32TargetX;
    int i32TargetY;
    int i32TargetOffset;
    int i32Width;
    int i32Height;
} S_VENDOR_OSD;

typedef struct
{
    int i32ColorKeyStatus;
    int i32ColorKeyValue;
} S_VENDOR_COLORKEY;

typedef struct
{
    int i32MplayerStatus;
    int i32MplayerCheck1;
    int i32MplayerCheck2;
    volatile int i32MplayerLockStatus;
    float pos;    // in secs
    float time_length;    // in secs
} S_VENDOR_SYNC;

static int m_shmid;
static void *m_shm;
static S_VENDOR_SYNC *m_vendor_sync;

U32 u64CurrentInPercent_Current_now;

static void NVT_init_shm(void)
{
    m_shmid = shmget((key_t)VENDOR_OSDSHMID, VENDOR_OSDSHMSIZE, (0666 | IPC_CREAT));
    if (m_shmid == -1)
    {
        printf("shmget (%d, 0x%08x) failed: %s\n", VENDOR_OSDSHMID, VENDOR_OSDSHMSIZE, strerror(errno));
        while (1);
    }

    if (m_shm != (void *)-1)
    {
        shmdt(m_shm);
        m_shm = (void *)-1;
    }

    m_shm = shmat(m_shmid, NULL, 0);

    if (m_shm == (void *)-1)
    {
        printf("shmat (%d) failed: %s\n", VENDOR_OSDSHMID, strerror(errno));
        while (1);
    }

    m_vendor_sync = (S_VENDOR_SYNC *) ((unsigned long)m_shm + sizeof(S_VENDOR_OSD) * VENDOR_OSDCOUNT + sizeof(S_VENDOR_COLORKEY));

    m_vendor_sync->i32MplayerStatus = -1;
    m_vendor_sync->i32MplayerCheck1 = -1;
    m_vendor_sync->i32MplayerCheck2 = -1;
    m_vendor_sync->pos = 0.0f;    // in secs
    m_vendor_sync->time_length = 0.0f;    // in secs
    m_vendor_sync->i32MplayerLockStatus = 0;    // unlock
}

void NVT_IMG_Decode(char * szFileName, int x, int y)
{
    FILE *pFile;
    int i32FileSize;
    char szFileName2[80];

    pFile = fopen(szFileName, "rb");
    fseek(pFile, 0L, SEEK_END);
    i32FileSize = ftell(pFile);
    fseek(pFile, 0L, SEEK_SET);
    fread((U8 *)s_au8BMP, 1, i32FileSize, pFile);
    fclose(pFile);
    GUI_DrawStreamedBitmapAuto(s_au8BMP, x, y);
}

void NVT_print_pos(void)
{
    m_vendor_sync->i32MplayerLockStatus = 1;	// Lock. Expect to unlock by mplayer.
    s_stream = popen( "echo pausing_keep_force get_vendor_state > /tmp/fifo_mplayer_cmd", "rw" );
    while (m_vendor_sync->i32MplayerLockStatus) GUI_Delay(10);
    m_pb_stat = m_vendor_sync->i32MplayerStatus;
    pclose( s_stream );

    m_vendor_sync->i32MplayerLockStatus = 1;	// Lock. Expect to unlock by mplayer.
    s_stream = popen( "echo pausing_keep_force get_time_pos > /tmp/fifo_mplayer_cmd", "rw" );
    while (m_vendor_sync->i32MplayerLockStatus) GUI_Delay(10);
    m_pb_pos = (U32)m_vendor_sync->pos;
    pclose( s_stream );

    m_vendor_sync->i32MplayerLockStatus = 1;	// Lock. Expect to unlock by mplayer.
    s_stream = popen( "echo pausing_keep_force get_time_length > /tmp/fifo_mplayer_cmd", "rw" );
    while (m_vendor_sync->i32MplayerLockStatus) GUI_Delay(10);
    m_pb_tot = (U32)m_vendor_sync->time_length;
    pclose( s_stream );

    //printf( "%02f / %02f / %d\n", m_vendor_sync->pos, m_vendor_sync->time_length, m_vendor_sync->i32MplayerStatus );
    //printf( "%d / %d / %d\n", m_pb_pos, m_pb_tot, m_pb_stat);
    //sprintf(sBuf, "### %d %d %d ###", m_pb_pos, m_pb_tot, m_pb_stat);
    //printf("%s\n", sBuf);

    if (m_pb_tot == 0)
        return;
    u64CurrentInPercent_Current_now = m_pb_pos * 100 / m_pb_tot;
}

int main()
{
    int fd, ret;
    int i, t = 0;
    int osd_offset;
    osd_cmd_t osd_block;

    FILE *fpVideoImg;

    unsigned long uVideoSize;

    pthread_t tid1, tid2;


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

    if (ioctl(fd, FBIOGET_FSCREENINFO, &fix) < 0)
    {
        perror("ioctl FBIOGET_FSCREENINFO");
        close(fd);
        return -1;
    }

    if (ioctl(fd, IOCTL_GET_OSD_OFFSET, &osd_offset) < 0) {
		perror("ioctl IOCTL_GET_OSD_OFFSET");
		close(fd);
		return -1;
	}

//  ioctl(fd,LCD_ENABLE_INT);
    uVideoSize = var.xres * var.yres * var.bits_per_pixel / 8;

    printf("uVideoSize = 0x%x\n", uVideoSize);
    printf("var.xres = 0x%x\n", var.xres);
    printf("var.yres = 0x%x\n", var.yres);
    uVideoSize = fix.smem_len;
    printf("uVideoSize = 0x%x \n", uVideoSize);
    pVideoBuffer = mmap(NULL, uVideoSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    printf("pVideoBuffer = 0x%x\n", pVideoBuffer);
    if(pVideoBuffer == MAP_FAILED)
    {
        printf("LCD Video Map Failed!\n");
        exit(0);
    }
    printf("### fix.smem_start=0x%x\n", fix.smem_start);
    ioctl(fd, IOCTL_LCD_GET_DMA_BASE, &g_VAFrameBuf);
    printf("### g_VAFrameBuf=0x%x\n", g_VAFrameBuf);
    printf("osd_offset = 0x%x\n", osd_offset);
    printf("fix.smem_len = 0x%x\n", fix.smem_len);

    // STEP: set RGB565 color key
	osd_block.cmd = OSD_SetTrans;
	osd_block.color = make_color(GUI_MAGENTA);		// STEP: set RGB565 color key
	ioctl(fd,OSD_SEND_CMD, (unsigned long) &osd_block);
	
	// STEP: clear all of OSD buffer
	osd_block.format = OSD_RGB565;
	osd_block.cmd = OSD_Open;			
	ioctl(fd,OSD_SEND_CMD, (unsigned long) &osd_block);
	osd_block.cmd = OSD_Clear;    			
	ioctl(fd,OSD_SEND_CMD, (unsigned long) &osd_block);
	
	// STEP: enable OSD
	osd_block.cmd = OSD_Show;
	ioctl(fd,OSD_SEND_CMD, (unsigned long) &osd_block);

    pVideoBuffer = pVideoBuffer+osd_offset;
    printf("### pVideoBuffer = 0x%x\n", pVideoBuffer);

    // the processing of video buffer
    //g_VAFrameBuf = pVideoBuffer;
    g_xres = var.xres;
    g_yres = var.yres;
    g_bits_per_pixel = var.bits_per_pixel;
    //
    // Create FIFO
    //
    mkfifo( MPLAYER_FIFO_PATH, 0777 );
    NVT_init_shm();
    snprintf(sBuf, sizeof(sBuf), "./mplayer -noaspect -loop 0 -idle -quiet -osdlevel 0 -fs -vendor-osdcount 0 -vendor-osdshmid 12345 -vendor-osdshmsize 1024 -slave -vendor-framedrop 1 -input file=/tmp/fifo_mplayer_cmd %s &", NVT_BEGIN_MOVIE);
    //system( "./mplayer -noaspect -loop 0 -idle -really-quiet -osdlevel 0 -fs -vendor-osdcount 0 -vendor-osdshmid 12345 -vendor-osdshmsize 1024 -slave -vendor-framedrop 1 -input file=/tmp/fifo_mplayer_cmd _movie.avi &" );
    system(sBuf);
    //
    // Start emWin
    //
    pthread_create(&tid1, NULL, MainTask_ISR, (void *)"MainTask");
    pthread_create(&tid2, NULL, TouchTask_ISR, (void *)"TouchTask");

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

//    MainTask();

    /* Close LCD */
    //ioctl(fd, VIDEO_DISPLAY_OFF);
    munmap(pVideoBuffer, uVideoSize);  //return memory
    close(fd);
    return 0;
}
