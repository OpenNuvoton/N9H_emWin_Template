#include "N9H20.h"
#include "GUI.h"
#include "WM.h"
#include "IMAGE.h"
#include "BUTTON.h"

#include "NVT_Config.h"

#define NVT_SETTING_BACKGROUND "D:\\setting_background.jpg"
#define NVT_SETTING_BUTTON1 "D:\\setting_back.dta"

static UINT8 s_u8BackgroundImageBuf[200 * 1024] __attribute__((aligned(32)));
static UINT8 s_u8Button1ImageBuf[18 * 1024] __attribute__((aligned(32)));

void NVT_Setting1SetBackground(WM_HWIN hItem)
{
    int i32FileSize;

    i32FileSize = NVT_GetImage(NVT_SETTING_BACKGROUND, s_u8BackgroundImageBuf);

    IMAGE_SetJPEG(hItem, s_u8BackgroundImageBuf, i32FileSize);
}

static int _ButtonSkin(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo)
{
    int Index;
    WM_HWIN hWin;

    hWin = pDrawItemInfo->hWin;
    BUTTON_GetUserData(hWin, &Index, sizeof(Index));

    switch (pDrawItemInfo->Cmd)
    {
    case WIDGET_ITEM_DRAW_BACKGROUND:
        //GUI_SetBkColor(0x00ABBCC5);
        //GUI_SetBkColor(GUI_YELLOW);
        //GUI_Clear();
        break;
    case WIDGET_ITEM_DRAW_BITMAP:
        switch (Index)
        {
        case 1:
            if (BUTTON_IsPressed(hWin) == 1)
            {
                GUI_SetBkColor(GUI_YELLOW);
                GUI_Clear();
            }
            else
                NVT_DrawBitmap(NVT_SETTING_BUTTON1, s_u8Button1ImageBuf, 0, 0);
            break;
        }
    }
    return 0;
}

void NVT_Setting1SetButton(WM_HWIN hItem, int i)
{
    BUTTON_SetSkin(hItem, _ButtonSkin);
    BUTTON_SetUserData(hItem, &i, sizeof(i));
}
