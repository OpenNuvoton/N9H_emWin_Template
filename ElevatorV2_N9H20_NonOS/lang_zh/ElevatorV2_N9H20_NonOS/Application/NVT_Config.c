#include "N9H20.h"
#include "GUI.h"
#include "GUI_Private.h"

#include "N9H20TouchPanel.h"
//
// JPEG bitstream buffer size in byte
// FIXME some JPEG files need to increase this value
// lower value can speed up the performance of parsing JPEG header and JPEG decoding
//
#define NVT_JPEG_BITSTREAM_SIZE (200 * 1024)

char _acBuffer[NVT_JPEG_BITSTREAM_SIZE] __attribute__((aligned(32)));
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

    /*Off              = */pfGetData(p, (const U8 **)&pInBuffer, NVT_JPEG_BITSTREAM_SIZE, 0);

//    OutBufferSize = 800 * 480 * 2;
//    hOutBuffer = GUI_ALLOC_AllocNoInit(OutBufferSize);
//    pOutBuffer = GUI_ALLOC_LockH(hOutBuffer);
    //
    // runtime to get jpeg width and height will decrease performance
    //
    GUI_JPEG_GetInfo(pInBuffer, NVT_JPEG_BITSTREAM_SIZE, &Info);
    //
    // unmark sysprintf here to increase performance
    // run sysprintf here is just for debugging and will decrease performance
    //
//    sysprintf("x=%d, y=%d\n", Info.XSize, Info.YSize);
//    sysprintf("x0=%d, y0=%d\n", x0, y0);
    memcpy(_acBuffer, pInBuffer, NVT_JPEG_BITSTREAM_SIZE);
    jpegOpen();
    jpegInit();
    jpegIoctl(JPEG_IOCTL_SET_BITSTREAM_ADDR, (((unsigned int)_acBuffer) | BIT31), 0);
    jpegIoctl(JPEG_IOCTL_SET_DECODE_MODE, JPEG_DEC_PRIMARY_PACKET_RGB888, 0);
//    jpegIoctl(JPEG_IOCTL_SET_DECODE_DOWNSCALE, Info.YSize, Info.XSize);
    jpegIoctl(JPEG_IOCTL_SET_DECODE_STRIDE, Info.XSize, 0);
    jpegIoctl(JPEG_IOCTL_SET_YADDR, (((unsigned int)_auOutBuffer) | BIT31), 0);
    jpegIoctl(JPEG_IOCTL_DECODE_TRIGGER, 0, 0);
    jpegWait();
    jpegClose();

    //
    // Draw buffer
    //
    _DrawBitmap(x0, y0, (void const *)_auOutBuffer, Info.XSize, Info.YSize, (Info.XSize * 4), 32);

//    GUI_ALLOC_UnlockH((void **)&pOutBuffer);
//    GUI_ALLOC_Free(hOutBuffer);

    return 0;
}

void Init_NVTJPEG(void)
{
    //
    // Speed up H/W jpeg decode in emWin
    //
    GUI_JPEG_SetpfDrawEx(JPEG_X_Draw);
}

#include "bg.h"

extern UINT8 *u8FrameBufPtr;

int NVT_DecodeBGJPEG(void)
{
    jpegOpen();
    jpegInit();
    jpegIoctl(JPEG_IOCTL_SET_BITSTREAM_ADDR, (((unsigned int)_acbg) | BIT31), 0);
    jpegIoctl(JPEG_IOCTL_SET_DECODE_MODE, JPEG_DEC_PRIMARY_PACKET_YUV422, 0);   // JPEG_DEC_PRIMARY_PACKET_YUV422 JPEG_DEC_PRIMARY_PACKET_RGB565 JPEG_DEC_PRIMARY_PACKET_RGB888
//    jpegIoctl(JPEG_IOCTL_SET_DECODE_DOWNSCALE, LCD_YSIZE, LCD_XSIZE);
    jpegIoctl(JPEG_IOCTL_SET_DECODE_STRIDE, LCD_XSIZE, 0);
    jpegIoctl(JPEG_IOCTL_SET_YADDR, (((unsigned int)u8FrameBufPtr) | BIT31), 0);
    jpegIoctl(JPEG_IOCTL_DECODE_TRIGGER, 0, 0);
    jpegWait();
    jpegClose();

    return 0;
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

void _NVTDiskInfo(void)
{
    UINT32 uBlockSize, uFreeSize, uDiskSize;

    fsDiskFreeSpace('D', &uBlockSize, &uFreeSize, &uDiskSize);

    sysprintf("############## D uBlockSize[%d], uFreeSize[%d], uDiskSize[%d]\r\n", uBlockSize, uFreeSize, uDiskSize);

    fsDiskFreeSpace('C', &uBlockSize, &uFreeSize, &uDiskSize);

    sysprintf("############## C uBlockSize[%d], uFreeSize[%d], uDiskSize[%d]\r\n", uBlockSize, uFreeSize, uDiskSize);
}
