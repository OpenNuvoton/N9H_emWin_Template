#include "N9H26.h"
#include "GUI.h"
#include "WM.h"
#include "IMAGE.h"
#include "BUTTON.h"

#include "NVT_Config.h"

void NVT_Playback1Play(WM_HWIN hItem);
void NVT_Playback1Stop(WM_HWIN hItem);
void NVT_Playback1Back(WM_HWIN hItem);

#define NVT_PLAYBACK_PLAY "D:\\Play_01.dta"
#define NVT_PLAYBACK_STOP "D:\\Pause_01.dta"
#define NVT_PLAYBACK_BACK "D:\\Home_01.dta"

static UINT8 s_u8PlayImageBuf[10 * 1024] __attribute__((aligned(32)));
static UINT8 s_u8StopImageBuf[10 * 1024] __attribute__((aligned(32)));
static UINT8 s_u8BackImageBuf[10 * 1024] __attribute__((aligned(32)));

void NVT_Playback1Play(WM_HWIN hItem)
{
    static int i32FileSize;

    if (i32FileSize == 0)
        i32FileSize = NVT_GetImage(NVT_PLAYBACK_PLAY, s_u8PlayImageBuf);

    IMAGE_SetDTA(hItem, s_u8PlayImageBuf, i32FileSize);
}

void NVT_Playback1Stop(WM_HWIN hItem)
{
    static int i32FileSize;

    if (i32FileSize == 0)
        i32FileSize = NVT_GetImage(NVT_PLAYBACK_STOP, s_u8StopImageBuf);

    IMAGE_SetDTA(hItem, s_u8StopImageBuf, i32FileSize);
}

void NVT_Playback1Back(WM_HWIN hItem)
{
    static int i32FileSize;

    if (i32FileSize == 0)
        i32FileSize = NVT_GetImage(NVT_PLAYBACK_BACK, s_u8BackImageBuf);

    IMAGE_SetDTA(hItem, s_u8BackImageBuf, i32FileSize);
}
