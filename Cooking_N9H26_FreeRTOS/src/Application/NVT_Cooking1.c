#include "N9H26.h"
#include "GUI.h"
#include "WM.h"
#include "IMAGE.h"
#include "BUTTON.h"

#include "NVT_Config.h"

void NVT_Cooking1Video1(WM_HWIN hItem);
void NVT_Cooking1Video2(WM_HWIN hItem);
void NVT_Cooking1Video3(WM_HWIN hItem);
void NVT_Cooking1Video4(WM_HWIN hItem);
void NVT_Cooking1Video5(WM_HWIN hItem);
void NVT_Cooking1Video6(WM_HWIN hItem);
void NVT_Cooking1Temp1(WM_HWIN hItem);
void NVT_Cooking1Setting1(WM_HWIN hItem);
void NVT_Cooking1SetButton(WM_HWIN hItem, int i);

#define NVT_COOKING_VIDEO1 "D:\\01_dessert.dta"
#define NVT_COOKING_VIDEO2 "D:\\02_meat.dta"
#define NVT_COOKING_VIDEO3 "D:\\03_rice.dta"
#define NVT_COOKING_VIDEO4 "D:\\04_seafood.dta"
#define NVT_COOKING_VIDEO5 "D:\\05_soup.dta"
#define NVT_COOKING_VIDEO6 "D:\\06_vegetable.dta"
#define NVT_COOKING_TEMP1 "D:\\07_temp_time.dta"
#define NVT_COOKING_SETTING1 "D:\\08_setting.dta"

static UINT8 s_u8Video1ImageBuf[95 * 1024] __attribute__((aligned(32)));
static UINT8 s_u8Video2ImageBuf[95 * 1024] __attribute__((aligned(32)));
static UINT8 s_u8Video3ImageBuf[95 * 1024] __attribute__((aligned(32)));
static UINT8 s_u8Video4ImageBuf[95 * 1024] __attribute__((aligned(32)));
static UINT8 s_u8Video5ImageBuf[95 * 1024] __attribute__((aligned(32)));
static UINT8 s_u8Video6ImageBuf[95 * 1024] __attribute__((aligned(32)));
static UINT8 s_u8Temp1ImageBuf[95 * 1024] __attribute__((aligned(32)));
static UINT8 s_u8Setting1ImageBuf[95 * 1024] __attribute__((aligned(32)));

void NVT_Cooking1Video1(WM_HWIN hItem)
{
    static int i32FileSize;

    if (i32FileSize == 0)
        i32FileSize = NVT_GetImage(NVT_COOKING_VIDEO1, s_u8Video1ImageBuf);

    IMAGE_SetDTA(hItem, s_u8Video1ImageBuf, i32FileSize);
}

void NVT_Cooking1Video2(WM_HWIN hItem)
{
    static int i32FileSize;

    if (i32FileSize == 0)
        i32FileSize = NVT_GetImage(NVT_COOKING_VIDEO2, s_u8Video2ImageBuf);

    IMAGE_SetDTA(hItem, s_u8Video2ImageBuf, i32FileSize);
}

void NVT_Cooking1Video3(WM_HWIN hItem)
{
    static int i32FileSize;

    if (i32FileSize == 0)
        i32FileSize = NVT_GetImage(NVT_COOKING_VIDEO3, s_u8Video3ImageBuf);

    IMAGE_SetDTA(hItem, s_u8Video3ImageBuf, i32FileSize);
}

void NVT_Cooking1Video4(WM_HWIN hItem)
{
    static int i32FileSize;

    if (i32FileSize == 0)
        i32FileSize = NVT_GetImage(NVT_COOKING_VIDEO4, s_u8Video4ImageBuf);

    IMAGE_SetDTA(hItem, s_u8Video4ImageBuf, i32FileSize);
}

void NVT_Cooking1Video5(WM_HWIN hItem)
{
    static int i32FileSize;

    if (i32FileSize == 0)
        i32FileSize = NVT_GetImage(NVT_COOKING_VIDEO5, s_u8Video5ImageBuf);

    IMAGE_SetDTA(hItem, s_u8Video5ImageBuf, i32FileSize);
}

void NVT_Cooking1Video6(WM_HWIN hItem)
{
    static int i32FileSize;

    if (i32FileSize == 0)
        i32FileSize = NVT_GetImage(NVT_COOKING_VIDEO6, s_u8Video6ImageBuf);

    IMAGE_SetDTA(hItem, s_u8Video6ImageBuf, i32FileSize);
}

void NVT_Cooking1Temp1(WM_HWIN hItem)
{
    static int i32FileSize;

    if (i32FileSize == 0)
        i32FileSize = NVT_GetImage(NVT_COOKING_TEMP1, s_u8Temp1ImageBuf);

    IMAGE_SetDTA(hItem, s_u8Temp1ImageBuf, i32FileSize);
}

void NVT_Cooking1Setting1(WM_HWIN hItem)
{
    static int i32FileSize;

    if (i32FileSize == 0)
        i32FileSize = NVT_GetImage(NVT_COOKING_SETTING1, s_u8Setting1ImageBuf);

    IMAGE_SetDTA(hItem, s_u8Setting1ImageBuf, i32FileSize);
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
        /* TODO */
#if 0
        GUI_SetBkColor(GUI_YELLOW);
        GUI_Clear();
#endif
        break;
    case WIDGET_ITEM_DRAW_BITMAP:
        /* TODO */
#if 0
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
                GUI_SetBkColor(GUI_BLUE);
                GUI_Clear();
            }
            break;
        }
#endif
        break;
    }
    return 0;
}

void NVT_Cooking1SetButton(WM_HWIN hItem, int i)
{
    BUTTON_SetSkin(hItem, _ButtonSkin);
    BUTTON_SetUserData(hItem, &i, sizeof(i));
}
