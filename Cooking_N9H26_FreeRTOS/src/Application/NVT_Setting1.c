#include "N9H26.h"
#include "GUI.h"
#include "WM.h"
#include "IMAGE.h"
#include "BUTTON.h"

#include "NVT_Config.h"

void NVT_Setting1Back(WM_HWIN hItem);

#define NVT_SETTING_BACK "D:\\Home_01.dta"

static UINT8 s_u8BackImageBuf[10 * 1024] __attribute__((aligned(32)));

void NVT_Setting1Back(WM_HWIN hItem)
{
    static int i32FileSize;

    if (i32FileSize == 0)
        i32FileSize = NVT_GetImage(NVT_SETTING_BACK, s_u8BackImageBuf);

    IMAGE_SetDTA(hItem, s_u8BackImageBuf, i32FileSize);
}
