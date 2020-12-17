#include "N9H20.h"
#include "GUI.h"
#include "WM.h"
#include "IMAGE.h"
#include "BUTTON.h"

#include "NVT_Config.h"

#define NVT_MAIN_BACKGROUND "D:\\main_background.dta"
#define NVT_MAIN_LOGO "D:\\logo.dta"
#define NVT_MAIN_BUTTON1 "D:\\main_freezer.dta"
#define NVT_MAIN_BUTTON2 "D:\\main_setting.dta"

static UINT8 s_u8BackgroundImageBuf[257 * 1024] __attribute__((aligned(32)));
static UINT8 s_u8LogoImageBuf[26 * 1024] __attribute__((aligned(32)));
static UINT8 s_u8Button1ImageBuf[26 * 1024] __attribute__((aligned(32)));
static UINT8 s_u8Button2ImageBuf[26 * 1024] __attribute__((aligned(32)));

void NVT_Main1SetBackground(WM_HWIN hItem)
{
    static int i32FileSize;

    if (i32FileSize == 0)
        i32FileSize = NVT_GetImage(NVT_MAIN_BACKGROUND, s_u8BackgroundImageBuf);

    IMAGE_SetDTA(hItem, s_u8BackgroundImageBuf, i32FileSize);
}

void NVT_Main1SetLogo(WM_HWIN hItem)
{
    static int i32FileSize;

    if (i32FileSize == 0)
        i32FileSize = NVT_GetImage(NVT_MAIN_LOGO, s_u8LogoImageBuf);

    IMAGE_SetDTA(hItem, s_u8LogoImageBuf, i32FileSize);
}

static int _ButtonSkin(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo)
{
    int Index;
    WM_HWIN hWin;
    static int i32FreezerImageFileSize;
    static int i32SettingImageFileSize;

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
                GUI_SetBkColor(GUI_RED);
                GUI_Clear();
            }
            else
            {
                if (i32FreezerImageFileSize == 0)
                    i32FreezerImageFileSize = NVT_GetImage(NVT_MAIN_BUTTON1, s_u8Button1ImageBuf);
                GUI_DrawStreamedBitmapAuto(s_u8Button1ImageBuf, 0, 0);
                GUI_GotoXY(0, 0);
                GUI_SetFont(GUI_FONT_20B_ASCII);
                GUI_SetColor(GUI_WHITE);
                GUI_SetTextMode(GUI_TM_TRANS);
                GUI_DispStringAt("Freezer", 50, 20);
            }
            break;
        case 2:
            if (BUTTON_IsPressed(hWin) == 1)
            {
                GUI_SetBkColor(GUI_GREEN);
                GUI_Clear();
            }
            else
            {
                if (i32SettingImageFileSize == 0)
                    i32SettingImageFileSize = NVT_GetImage(NVT_MAIN_BUTTON2, s_u8Button2ImageBuf);
                GUI_DrawStreamedBitmapAuto(s_u8Button2ImageBuf, 0, 0);
                GUI_GotoXY(0, 0);
                GUI_SetFont(GUI_FONT_20B_ASCII);
                GUI_SetColor(GUI_WHITE);
                GUI_SetTextMode(GUI_TM_TRANS);
                GUI_DispStringAt("Setting", 50, 20);
            }
            break;
        }
    }
    return 0;
}

void NVT_Main1SetButton(WM_HWIN hItem, int i)
{
    BUTTON_SetSkin(hItem, _ButtonSkin);
    BUTTON_SetUserData(hItem, &i, sizeof(i));
}
