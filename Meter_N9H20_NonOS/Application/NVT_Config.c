#include "N9H20.h"
#include "GUI.h"
#include "GUI_Private.h"

#include "NVT_Config.h"

#include "N9H20TouchPanel.h"
//
// JPEG bitstream buffer size in byte
// FIXME some JPEG files need to increase this value
// lower value can speed up the performance of parsing JPEG header and JPEG decoding
//
#define NVT_JPEG_BITSTREAM_SIZE (200 * 1024)

static char _acBuffer[NVT_JPEG_BITSTREAM_SIZE] __attribute__((aligned(32)));
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

U32 NVT_GetImage(char * szFileName, UINT8 * pu8Image)
{
    int hFile, wbytes, i32FileSize;
    char szFileName2[80];

    fsAsciiToUnicode(szFileName, szFileName2, TRUE);
    hFile = fsOpenFile(szFileName2, szFileName, O_RDONLY | O_FSEEK);
    i32FileSize = fsGetFileSize(hFile);
    fsReadFile(hFile, (UINT8 *)pu8Image, i32FileSize, &wbytes);
    fsCloseFile(hFile);

    return i32FileSize;
}

void NVT_DrawBitmap(char * szFileName, UINT8 * pu8Image, int x, int y)
{
    int hFile, wbytes, i32FileSize;
    char szFileName2[80];

    fsAsciiToUnicode(szFileName, szFileName2, TRUE);
    hFile = fsOpenFile(szFileName2, szFileName, O_RDONLY | O_FSEEK);
    i32FileSize = fsGetFileSize(hFile);
    fsReadFile(hFile, (UINT8 *)pu8Image, i32FileSize, &wbytes);
    fsCloseFile(hFile);
//    GUI_BMP_Draw(pu8Image, x, y);
    GUI_DrawStreamedBitmapAuto(pu8Image, x, y);
}
