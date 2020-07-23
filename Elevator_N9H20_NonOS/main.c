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
UINT8 u8FrameBuf[XSIZE_PHYS*YSIZE_PHYS*2*3];
#else
UINT8 u8FrameBuf[XSIZE_PHYS*YSIZE_PHYS*2*3] __attribute__((aligned(32)));
#endif

UINT8 *u8FrameBufPtr;

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
//
// Define text box x-y & width-height
//
#define NVT_TEXT_BOX_X 240
#define NVT_TEXT_BOX_Y 0
#define NVT_TEXT_BOX_WIDTH (NVT_TEXT_BOX_X + 128)
#define NVT_TEXT_BOX_HEIGHT (NVT_TEXT_BOX_Y + 128)
//
// Define total counts
// Need correct number of images, respectively
//
#define NVT_BG_CNT 3
#define NVT_FL_CNT 15
#define NVT_AR_CNT 8
//
// Define update interval
// Please note NVT_AR_UPDATE_TIME must less than elevator loop time
//
#define NVT_BG_UPDATE_TIME 2000
#define NVT_FL_UPDATE_TIME 1000
#define NVT_AR_UPDATE_TIME 300
//
// Update background index
//
volatile int g_BGCnt;
volatile int g_BGHandle;

void TMR0_IRQHandler_BGTask(void)
{
    g_BGCnt++;
    if (g_BGCnt > NVT_BG_CNT)
        g_BGCnt = 1;
}
//
// Update floor number index
//
volatile int g_FLCnt;
volatile int g_FLHandle;

void TMR0_IRQHandler_FLTask(void)
{
    g_FLCnt++;
    if (g_FLCnt > NVT_FL_CNT)
        g_FLCnt = 1;
}
//
// Update arrow index
//
volatile int g_ARCnt;
volatile int g_ARHandle;

void TMR0_IRQHandler_ARTask(void)
{
    g_ARCnt++;
    if (g_ARCnt > NVT_AR_CNT)
        g_ARCnt = 1;
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
    sysSetTimerEvent(TIMER0,  1, (PVOID)TMR0_IRQHandler);           /* 1  tick  per call back */
    sysSetTimerEvent(TIMER0, 20, (PVOID)TMR0_IRQHandler_TouchTask); /* 20 ticks per call back */
    g_BGHandle = sysSetTimerEvent(TIMER0, NVT_BG_UPDATE_TIME, (PVOID)TMR0_IRQHandler_BGTask); /* 2000 ticks per call back */
    g_FLHandle = sysSetTimerEvent(TIMER0, NVT_FL_UPDATE_TIME, (PVOID)TMR0_IRQHandler_FLTask); /* 1000 ticks per call back */
    g_ARHandle = sysSetTimerEvent(TIMER0, NVT_AR_UPDATE_TIME, (PVOID)TMR0_IRQHandler_ARTask); /*  300 ticks per call back */

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
    vpostLCMInit(&lcdFormat, (UINT32*)((UINT32)u8FrameBuf | BIT31));
    u8FrameBufPtr  = (UINT8 *)((UINT32)u8FrameBuf | BIT31);
    /* If backlight control signal is different from nuvoton's demo board,
        please don't call this function and must implement another similar one to enable LCD backlight. */
//    vpostEnaBacklight();
}

/*********************************************************************
*
*       main
*/

//extern void MainTask(void);

#if 0
#include "001.c"
#include "002.c"
#include "003.c"

void MainTask(void)
{
    GUI_Init();

    while(1)
    {
        GUI_BMP_Draw(_ac001, 0, 0);
        GUI_Delay(5000);
        GUI_BMP_Draw(_ac002, 0, 0);
        GUI_Delay(5000);
        GUI_BMP_Draw(_ac003, 0, 0);
        GUI_Delay(5000);
    }
}
#endif
//
// Read file and decode image
//
UINT8 u8BMPBuf[1 * 1024 * 1024] __attribute__((aligned(32)));

void _BMP_Decode(char * szFileName)
{
    int hFile, wbytes, i32FileSize;
    char szFileName2[80];

    fsAsciiToUnicode(szFileName, szFileName2, TRUE);
    hFile = fsOpenFile(szFileName2, szFileName, O_RDONLY | O_FSEEK);
    i32FileSize = fsGetFileSize(hFile);
    fsReadFile(hFile, (UINT8 *)u8BMPBuf, i32FileSize, &wbytes);
    fsCloseFile(hFile);
    GUI_BMP_Draw(u8BMPBuf, 0, 0);
}

void _GIF_Decode(char * szFileName)
{
    int hFile, wbytes, i32FileSize;
    char szFileName2[80];

    fsAsciiToUnicode(szFileName, szFileName2, TRUE);
    hFile = fsOpenFile(szFileName2, szFileName, O_RDONLY | O_FSEEK);
    i32FileSize = fsGetFileSize(hFile);
    fsReadFile(hFile, (UINT8 *)u8BMPBuf, i32FileSize, &wbytes);
    fsCloseFile(hFile);
    GUI_GIF_Draw(u8BMPBuf, i32FileSize, 300, 0);
}

static char _acBuffer[100 * 1024] __attribute__((aligned(32)));
static UINT8 _auOutBuffer[XSIZE_PHYS * YSIZE_PHYS * 2] __attribute__((aligned(32)));

extern GUI_CONTEXT * GUI_pContext;

extern void LCD_DrawBitmap(int x0,    int y0,
                    int xsize, int ysize,
                    int xMul,  int yMul,
                    int BitsPerPixel,
                    int BytesPerLine,
                    const U8 * pPixel,
                    const LCD_PIXELINDEX * pTrans);

/*********************************************************************
*
*       _DrawBitmap
*/
static void _DrawBitmap(int x, int y, void const * p, int xSize, int ySize, int BytesPerLine, int BitsPerPixel)
{
#if (GUI_WINSUPPORT)
    GUI_RECT r;
#endif
#if (GUI_WINSUPPORT)
    WM_ADDORG(x,y);
    r.x1 = (r.x0 = x) + xSize-1;
    r.y1 = (r.y0 = y) + ySize-1;
    WM_ITERATE_START(&r)
    {
#endif
//        sysprintf("*r.x0=%d, r.y0=%d, r.x1=%d, r.y1=%d\n", r.x0, r.y0, r.x1, r.y1);
        LCD_DrawBitmap(x, y, xSize, ySize, 1, 1, BitsPerPixel, BytesPerLine, p, NULL);
#if (GUI_WINSUPPORT)
    }
    WM_ITERATE_END();
#endif
}

/*********************************************************************
*
*       JPEG_X_Draw
*/
int JPEG_X_Draw(GUI_GET_DATA_FUNC * pfGetData, void * p, int x0, int y0)
{
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

void _JPG_Decode(char * szFileName)
{
    int hFile, wbytes, i32FileSize;
    char szFileName2[80];

    fsAsciiToUnicode(szFileName, szFileName2, TRUE);
    hFile = fsOpenFile(szFileName2, szFileName, O_RDONLY | O_FSEEK);
    i32FileSize = fsGetFileSize(hFile);
    fsReadFile(hFile, (UINT8 *)u8BMPBuf, i32FileSize, &wbytes);
    fsCloseFile(hFile);
    GUI_JPEG_Draw(u8BMPBuf, i32FileSize, 0, 0);
}

void _PNG_Decode(char * szFileName)
{
    int hFile, wbytes, i32FileSize;
    char szFileName2[80];

    fsAsciiToUnicode(szFileName, szFileName2, TRUE);
    hFile = fsOpenFile(szFileName2, szFileName, O_RDONLY | O_FSEEK);
    i32FileSize = fsGetFileSize(hFile);
    fsReadFile(hFile, (UINT8 *)u8BMPBuf, i32FileSize, &wbytes);
    fsCloseFile(hFile);
    GUI_PNG_Draw(u8BMPBuf, i32FileSize, 0, 272 - 168);
}

//#include "SONG.h"

//#define LENGTH        194540  // The length of song in unit of 16-bit

UINT8 u8PCMPBuf[1 * 1024 * 1024] __attribute__((aligned(32)));
INT32 i32PCMSize;

static UINT16 u16IntCount;
static UINT32 u32FragSize;
static volatile UINT8 bPlaying;

int playCallBack(UINT8 * pu8Buffer)
{
    UINT32 u32Offset = 0;
    UINT32 len = i32PCMSize;

    u32Offset = ( u32FragSize / 2) * u16IntCount;
    if (u32Offset >= len)       // Reach the end of the song, restart from beginning
    {
        u16IntCount = 2;
        u32Offset = 0;
        bPlaying = FALSE;
        return TRUE;
    }

    memcpy(pu8Buffer, &u8PCMPBuf[u32Offset], u32FragSize/2);

    u16IntCount++;

    return FALSE;
}

void _PCM_Decode(char * szFileName)
{
    int hFile, wbytes, i32FileSize;
    char szFileName2[80];

    u16IntCount = 2;
    u32FragSize = 0;
    bPlaying = TRUE;

    fsAsciiToUnicode(szFileName, szFileName2, TRUE);
    hFile = fsOpenFile(szFileName2, szFileName, O_RDONLY | O_FSEEK);
    i32FileSize = fsGetFileSize(hFile);
    fsReadFile(hFile, (UINT8 *)u8PCMPBuf, i32FileSize, &wbytes);
    fsCloseFile(hFile);
    i32PCMSize = i32FileSize;

//  sysprintf("Start Playing...\n");

    spuOpen(eDRVSPU_FREQ_44100);

//    SPU_MONO();

    spuIoctl(SPU_IOCTL_SET_VOLUME, 100, 100);
    spuIoctl(SPU_IOCTL_GET_FRAG_SIZE, (UINT32)&u32FragSize, 0);

    spuEqOpen(eDRVSPU_EQBAND_2, eDRVSPU_EQGAIN_P7DB);
    spuDacOn(1);
    spuStartPlay((PFN_DRVSPU_CB_FUNC *) playCallBack, (UINT8 *)u8PCMPBuf);

    while(bPlaying == TRUE);

    spuEqClose();
    spuClose();
}

int hFile2;

int playCallBack2(UINT8 * pu8Buffer)
{
    int wbytes;
    UINT32 u32Offset = 0;
    UINT32 len = i32PCMSize;

    u32Offset = ( u32FragSize / 2) * u16IntCount;
    if (u32Offset >= len)       // Reach the end of the song, restart from beginning
    {
        u16IntCount = 2;
        u32Offset = 0;
        bPlaying = FALSE;
        return TRUE;
    }

//  memcpy(pu8Buffer, &u8PCMPBuf[u32Offset], u32FragSize/2);
    fsReadFile(hFile2, (UINT8 *)pu8Buffer, u32FragSize/2, &wbytes);

    u16IntCount++;

    return FALSE;
}

extern void SPU_MONO(void);
//
// Read wav file then decode, supported formats are 8~48kHz mono or stereo at 16-bit
//
void _WAV_Decode(char * szFileName)
{
    UINT32 u32WavSamplingRate, u32WavChannel, u32WavBit;
    int /*hFile, */wbytes;
    char szFileName2[80];

    fsAsciiToUnicode(szFileName, szFileName2, TRUE);
    hFile2 = fsOpenFile(szFileName2, szFileName, O_RDONLY | O_FSEEK);

    /* Scan wave header and finally position file pointer to start of wave data region. */
    do
    {
        UINT32 ckID = (UINT32) -1, ckSize = (UINT32) -1, wavSig = (UINT32) -1;
        UINT32 wavHdrPos = (UINT32) -1;
        UINT32 wavDatPos = (UINT32) -1;

        u32WavSamplingRate = 0;
        u32WavChannel = 0;
        u32WavBit = 0;

        fsReadFile(hFile2, (UINT8 *)&ckID, 4, &wbytes);  // "RIFF" Chunk ID.
        fsReadFile(hFile2, (UINT8 *)&ckSize, 4, &wbytes);    // Chunk size.
        fsReadFile(hFile2, (UINT8 *)&wavSig, 4, &wbytes);    // "WAVE" signature.
        if (ckID != 0x46464952 ||   // "RIFF"
                wavSig != 0x45564157)    // "WAVE"
        {
            sysprintf("Incorrect wave format!\n");
            fsCloseFile(hFile2);
            return;
        }
        while (1)
        {
            if(fsIsEOF(hFile2))
            {
                sysprintf("End of file reached!\n");
                fsCloseFile(hFile2);
                return;
            }

            ckID = (UINT32) -1;
            ckSize = (UINT32) -1;
            fsReadFile(hFile2, (UINT8 *)&ckID, 4, &wbytes);  // Chunk ID.
            fsReadFile(hFile2, (UINT8 *)&ckSize, 4, &wbytes);    // Chunk size.
            if (ckID == 0x20746d66)     // "fmt "
            {
                UINT16 fmtTag, numChan;
                UINT32 sampleRate, byteRate;
                UINT16 blockAlign, sampleSize;

                /* FIX ME */
//                wavHdrPos = f_tell(&wavFileObject);
                wavHdrPos = ckSize;

                fsReadFile(hFile2, (UINT8 *)&fmtTag, 2, &wbytes);    // Format tag.
                fsReadFile(hFile2, (UINT8 *)&numChan, 2, &wbytes);   // Number of channels.
                fsReadFile(hFile2, (UINT8 *)&sampleRate, 4, &wbytes);    // Sample rate.
                fsReadFile(hFile2, (UINT8 *)&byteRate, 4, &wbytes);  // Byte rate.
                fsReadFile(hFile2, (UINT8 *)&blockAlign, 2, &wbytes);    // Block align.
                fsReadFile(hFile2, (UINT8 *)&sampleSize, 2, &wbytes);    // Bits per sample.

                sysprintf("Compression code: %d\n", fmtTag);

                u32WavSamplingRate = sampleRate;
                u32WavChannel = numChan;
                u32WavBit = sampleSize;

//                fsFileSeek(hFile, ckSize, SEEK_CUR);    // Seek to next chunk.
            }
            else if (ckID == 0x61746164)    // "data"
            {
                /* FIX ME */
//                wavDatPos = f_tell(&wavFileObject);
                i32PCMSize = wavDatPos = ckSize;

//                fsFileSeek(hFile, ckSize, SEEK_CUR);    // Seek to next chunk.
            }
            else
            {
                fsFileSeek(hFile2, ckSize, SEEK_CUR);    // Seek to next chunk.
            }

            if (wavHdrPos != (UINT32) -1 && wavDatPos != (UINT32) -1)
            {
//                fsFileSeek(hFile, wavDatPos, SEEK_CUR); // Seek to wave data.
                break;
            }
        }
    }
    while (0);

    // change sampling rate
    if ( u32WavSamplingRate == 44100 )
    {
        spuOpen(eDRVSPU_FREQ_44100);
    }
    else if ( u32WavSamplingRate == 22050 )
    {
        spuOpen(eDRVSPU_FREQ_22050);
    }
    else if ( u32WavSamplingRate == 11025 )
    {
        spuOpen(eDRVSPU_FREQ_11025);
    }
    else if ( u32WavSamplingRate == 48000 )
    {
        spuOpen(eDRVSPU_FREQ_48000);
    }
    else if ( u32WavSamplingRate == 24000 )
    {
        spuOpen(eDRVSPU_FREQ_24000);
    }
    else if ( u32WavSamplingRate == 12000 )
    {
        spuOpen(eDRVSPU_FREQ_12000);
    }
    else if ( u32WavSamplingRate == 32000 )
    {
        spuOpen(eDRVSPU_FREQ_32000);
    }
    else if ( u32WavSamplingRate == 16000 )
    {
        spuOpen(eDRVSPU_FREQ_16000);
    }
    else if ( u32WavSamplingRate == 8000 )
    {
        spuOpen(eDRVSPU_FREQ_8000);
    }

    /* Check if sampling rate is supported. */
    do
    {
        UINT32 SRSupArr[] =   // Sampling rate support list
        {
            8000, 16000, 32000,
            12000, 24000, 48000,
            11025, 22050, 44100,
        };
        UINT32 *SRSupInd;
        UINT32 *SRSupEnd = SRSupArr + sizeof (SRSupArr) / sizeof (SRSupArr[0]);

        for (SRSupInd = SRSupArr; SRSupInd != SRSupEnd; SRSupInd ++)
        {
            if (u32WavSamplingRate == *SRSupInd)
            {
                break;
            }
        }
        if (SRSupInd == SRSupEnd)
        {
            sysprintf("%d sampling rate not support!\n", u32WavSamplingRate);
            fsCloseFile(hFile2);
            return;
        }
    }
    while (0);

    sysprintf("wav: sampling rate=%d, channel(s)=%d, bits=%d\n", u32WavSamplingRate, u32WavChannel, u32WavBit);

    if (u32WavBit == 16)
        u32WavBit = 16;
    else if (u32WavBit == 24)
        u32WavBit = 24;
    else if (u32WavBit == 32)
        u32WavBit = 32;
    else
    {
        sysprintf("bits not support!\n");
        fsCloseFile(hFile2);
        return;
    }

    if (u32WavChannel == 2)
    {
        u32WavChannel = 2;
    }
    else if (u32WavChannel == 1)
    {
        u32WavChannel = 1;
        SPU_MONO();
    }
    else
    {
        sysprintf("channel(s) not support!\n");
        fsCloseFile(hFile2);
        return;
    }

    u16IntCount = 2;
    u32FragSize = 0;
    bPlaying = TRUE;

//    fsReadFile(hFile, (UINT8 *)u8PCMPBuf, i32PCMSize, &wbytes);
//    fsCloseFile(hFile);

    spuIoctl(SPU_IOCTL_SET_VOLUME, 100, 100);
    spuIoctl(SPU_IOCTL_GET_FRAG_SIZE, (UINT32)&u32FragSize, 0);

    fsReadFile(hFile2, (UINT8 *)u8PCMPBuf, u32FragSize, &wbytes);

    spuEqOpen(eDRVSPU_EQBAND_2, eDRVSPU_EQGAIN_P7DB);
    spuDacOn(1);
    spuStartPlay((PFN_DRVSPU_CB_FUNC *) playCallBack2, (UINT8 *)u8PCMPBuf);

    while(bPlaying == TRUE);

    fsCloseFile(hFile2);

    spuEqClose();
    spuClose();
}

void MainTask(void)
{
//    int t, t1;    // Measure time
    unsigned int key;   // Key for action
    char szFileName[40];    // Working filename
    GUI_RECT Rect;  // Draw text and rotate
    //
    // Draw background, floor number & arrow start from 1
    // Those flags updated in timer isr
    //
    g_BGCnt = 1;
    g_FLCnt = 1;
    g_ARCnt = 1;
    //
    // Init key for action
    //
    kpi_init();
    kpi_open(3); // use nIRQ0 as external interrupt source
    //
    // Init emWin
    //
    GUI_Init();
    //
    // Use multiple buffers to avoid flicker
    //
    WM_MULTIBUF_Enable(1);
    GUI_MEMDEV_MULTIBUF_Enable(1);
    //
    // Speed up H/W jpeg decode in emWin
    //
    GUI_JPEG_SetpfDrawEx(JPEG_X_Draw);
    //
    // Text box
    //
    Rect.x0 = NVT_TEXT_BOX_X;
    Rect.y0 = NVT_TEXT_BOX_Y;
    Rect.x1 = NVT_TEXT_BOX_WIDTH;
    Rect.y1 = NVT_TEXT_BOX_HEIGHT;
    //
    // Text background is transparent
    //
    GUI_SetTextMode(GUI_TM_TRANS);
    //
    // Font type
    //
    GUI_SetFont(GUI_FONT_D80);
    //
    // Elevator loop
    //
    while(1)
    {
        //
        // Read key value and action later
        //
        key = kpi_read(KPI_NONBLOCK);
        //
        // Start to draw
        //
        GUI_MULTIBUF_Begin();

//        t = GUI_GetTime();
#if 0
        sprintf(szFileName, "C:\\IMG\\BG\\background%02d.bmp", g_BGCnt);
        _BMP_Decode(szFileName);
#endif
        //
        // Read background image (jpeg file) and H/W decode
        //
        sprintf(szFileName, "C:\\IMG\\BG\\background_%02d.jpg", g_BGCnt);
        _JPG_Decode(szFileName);

//        t = GUI_GetTime() - t;
//        sysprintf("1 %d\n", t);

//        t = GUI_GetTime();
#if 0
        sprintf(szFileName, "C:\\IMG\\AR\\arrow_%02d.gif", g_ARCnt);
        _GIF_Decode(szFileName);
//        _BMP_Decode(szFileName);
#endif
        //
        // Read arrow image (png file) and S/W decode
        //
        sprintf(szFileName, "C:\\IMG\\AR\\arrow_%02d.png", g_ARCnt);
        _PNG_Decode(szFileName);

//        t = GUI_GetTime() - t;
//        sysprintf("2 %d\n", t);

//        t = GUI_GetTime();
        //
        // Draw rotate text for floor number
        //
        sprintf(szFileName, "%d", g_FLCnt);
        GUI_DispStringInRectEx(szFileName, &Rect, GUI_TA_HCENTER | GUI_TA_VCENTER, strlen(szFileName), GUI_ROTATE_CCW);

//        t = GUI_GetTime() - t;
//        sysprintf("3 %d\n", t);
        //
        // End of drawing
        //
        GUI_MULTIBUF_End();
        //
        // Key action
        //
        if (key != 0)
        {
            //
            // Stop timer 0
            // Clear background, floor & arrow event
            //
            sysStopTimer(TIMER0);
            sysClearTimerEvent(TIMER0, g_BGHandle);
            sysClearTimerEvent(TIMER0, g_FLHandle);
            sysClearTimerEvent(TIMER0, g_ARHandle);
            //
            // Play wav
            //
            if (key == 1)
            {
                //
                // Play status
                //
                sprintf(szFileName, "C:\\WAV\\ST\\em001.wav");
                _WAV_Decode(szFileName);
            }
            else if (key == 2)
            {
                //
                // Play status in English
                //
                sprintf(szFileName, "D:\\WAV\\ST\\em001.wav");
                _WAV_Decode(szFileName);
            }
            else if ((key == 4) || (key == 8))
            {
                //
                // Play floor
                //
                sprintf(szFileName, "C:\\WAV\\FL\\%03d.wav", g_FLCnt);
                _WAV_Decode(szFileName);
                sprintf(szFileName, "C:\\WAV\\DO\\open001.wav");
                _WAV_Decode(szFileName);
                sprintf(szFileName, "C:\\WAV\\DO\\close001.wav");
                _WAV_Decode(szFileName);
            }
            else
            {
                //
                // Play floor in English
                //
                sprintf(szFileName, "D:\\WAV\\FL\\%03d.wav", g_FLCnt);
                _WAV_Decode(szFileName);
                sprintf(szFileName, "D:\\WAV\\DO\\open001.wav");
                _WAV_Decode(szFileName);
                sprintf(szFileName, "D:\\WAV\\DO\\close001.wav");
                _WAV_Decode(szFileName);
            }
            //
            // Restart timer 0 all events
            //
            sysStartTimer(TIMER0, 1000, PERIODIC_MODE);     /* 1000 ticks/per sec ==> 1tick/1ms */
            sysSetTimerEvent(TIMER0,  1, (PVOID)TMR0_IRQHandler);           /* 1  tick  per call back */
            sysSetTimerEvent(TIMER0, 20, (PVOID)TMR0_IRQHandler_TouchTask); /* 20 ticks per call back */
            g_BGHandle = sysSetTimerEvent(TIMER0, NVT_BG_UPDATE_TIME, (PVOID)TMR0_IRQHandler_BGTask); /* 2000 ticks per call back */
            g_FLHandle = sysSetTimerEvent(TIMER0, NVT_FL_UPDATE_TIME, (PVOID)TMR0_IRQHandler_FLTask); /* 1000 ticks per call back */
            g_ARHandle = sysSetTimerEvent(TIMER0, NVT_AR_UPDATE_TIME, (PVOID)TMR0_IRQHandler_ARTask); /*  300 ticks per call back */
        }
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
    //
    // Entry point
    //
    MainTask();
//    while(1);

//      Uninit_TouchPanel();
//      sysStopTimer(TIMER0);
//    sysDisableCache();
    return 0;
}

/*************************** End of file ****************************/
