#include "DIALOG.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#if 1
#define COLOR_BACK0            GUI_MAKE_COLOR(0xFF3333)
#define COLOR_BACK1            GUI_MAKE_COLOR(0x550000)
#define COLOR_BORDER           GUI_MAKE_COLOR(0x444444)
#define COLOR_KEYPAD0          GUI_MAKE_COLOR(0xAAAAAA)
#define COLOR_KEYPAD1          GUI_MAKE_COLOR(0x555555)
#define BUTTON_COLOR0          GUI_MAKE_COLOR(0xEEEEEE)
#define BUTTON_COLOR1          GUI_MAKE_COLOR(0xCCCCCC)
#define BUTTON_COLOR2          GUI_MAKE_COLOR(0xCCCCCC)
#define BUTTON_COLOR3          GUI_MAKE_COLOR(0xAAAAAA)
#define COLOR_BLUE             GUI_MAKE_COLOR(0x00865B00)
#define BUTTON_SKINFLEX_RADIUS 4
#define ID_BUTTON              (GUI_ID_USER + 0x200)
#define APP_INIT_LOWERCASE     (WM_USER + 0)
#define MSG_ANIMATE            (WM_USER + 0x02)
#define ANIMATION_TIME         300
#define BM_BACKBUTTON          bmBackButton_53x16
#define SIZE_Y_KEYPAD          (376)//(194)
#endif

#define ID_WINDOW_0    (GUI_ID_USER + 0x00)
#define ID_EDIT_0      (GUI_ID_USER + 0x01)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
#if 1
typedef struct {
  int          xPos;
  int          yPos;
  int          xSize;
  int          ySize;
  const char * acLabel;
  void (* pfDraw)(WM_HWIN hWin);
  int          HasLowerCase;
} BUTTON_DATA;

typedef struct {
  BUTTON_SKINFLEX_PROPS * pProp;
  int                     Index;
  BUTTON_SKINFLEX_PROPS   PropOld;
} BUTTON_PROP;

typedef struct {
  WM_HWIN hWin;
  WM_HWIN hBackButton;
  int     Dir;
} ANIM_DATA;

WM_HWIN GUI_CreateKeyPad(WM_HWIN hParent);

#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

/*********************************************************************
*
*       _aDialogCreate
*/
#if 0
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect, "Window", ID_WINDOW_0, 0, 0, 480, 272, 0, 0x0, 0 },
  { EDIT_CreateIndirect, "Edit", ID_EDIT_0, 10, 10, 460, 40, 0, 0x64, 0 },
};
#endif

/*********************************************************************
*
*       bmReturn, bmBackSpace, bmUp: Bitmaps used for keyboard
*/
static GUI_CONST_STORAGE unsigned long acReturn[] = {
#if (GUI_USE_ARGB == 1)
  0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x5FA0A0A0, 0xDF202020, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0xAF505050, 0xAF505050,
  0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x5FA0A0A0, 0xDF202020, 0xFF000000, 0xFF000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0xFF000000, 0xFF000000,
  0x00FFFFFF, 0x00FFFFFF, 0x5FA0A0A0, 0xDF202020, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0xFF000000, 0xFF000000,
  0x5FA0A0A0, 0xDF202020, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
  0x5FA0A0A0, 0xDF202020, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xAF505050,
  0x00FFFFFF, 0x00FFFFFF, 0x5FA0A0A0, 0xDF202020, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
  0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x5FA0A0A0, 0xDF202020, 0xFF000000, 0xFF000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
  0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x5FA0A0A0, 0xDF202020, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF
#else
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x50505050, 0x50505050,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000,
  0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000,
  0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x50505050,
  0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
#endif
};

static GUI_CONST_STORAGE GUI_BITMAP bmReturn = {
  16, // XSize
  8, // YSize
  64, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)acReturn,  // Pointer to picture data
  NULL,  // Pointer to palette
#if (GUI_USE_ARGB == 1)
  GUI_DRAW_BMPM8888I
#else
  GUI_DRAW_BMP8888
#endif
};

static GUI_CONST_STORAGE unsigned long acBackSpace[] = {
#if (GUI_USE_ARGB == 1)
  0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x5FA0A0A0, 0xDF202020, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
  0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x5FA0A0A0, 0xDF202020, 0xFF000000, 0xFF000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
  0x00FFFFFF, 0x00FFFFFF, 0x5FA0A0A0, 0xDF202020, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
  0x5FA0A0A0, 0xDF202020, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
  0x5FA0A0A0, 0xDF202020, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
  0x00FFFFFF, 0x00FFFFFF, 0x5FA0A0A0, 0xDF202020, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
  0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x5FA0A0A0, 0xDF202020, 0xFF000000, 0xFF000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
  0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x5FA0A0A0, 0xDF202020, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF
#else
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
  0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
#endif
};

static GUI_CONST_STORAGE GUI_BITMAP bmBackSpace = {
  16, // XSize
  8, // YSize
  64, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)acBackSpace,  // Pointer to picture data
  NULL,  // Pointer to palette
#if (GUI_USE_ARGB == 1)
  GUI_DRAW_BMPM8888I
#else
  GUI_DRAW_BMP8888
#endif
};

static GUI_CONST_STORAGE unsigned long acUp[] = {
#if (GUI_USE_ARGB == 1)
  0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x9F606060, 0x9F606060, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
  0x00FFFFFF, 0x00FFFFFF, 0x9F606060, 0xCF303030, 0xCF303030, 0x9F606060, 0x00FFFFFF, 0x00FFFFFF,
  0x00FFFFFF, 0x9F606060, 0xCF303030, 0x10EFEFEF, 0x10EFEFEF, 0xCF303030, 0x9F606060, 0x00FFFFFF,
  0x9F606060, 0xEF101010, 0x50AFAFAF, 0x00FFFFFF, 0x00FFFFFF, 0x50AFAFAF, 0xEF101010, 0x9F606060,
  0xBF404040, 0xBF404040, 0xFF000000, 0x00FFFFFF, 0x00FFFFFF, 0xFF000000, 0xBF404040, 0xBF404040,
  0x00FFFFFF, 0x00FFFFFF, 0xFF000000, 0x00FFFFFF, 0x00FFFFFF, 0xFF000000, 0x00FFFFFF, 0x00FFFFFF,
  0x00FFFFFF, 0x00FFFFFF, 0xFF000000, 0x00FFFFFF, 0x00FFFFFF, 0xFF000000, 0x00FFFFFF, 0x00FFFFFF,
  0x00FFFFFF, 0x00FFFFFF, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0x00FFFFFF, 0x00FFFFFF
#else
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x60606060, 0x60606060, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0x60606060, 0x30303030, 0x30303030, 0x60606060, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0x60606060, 0x30303030, 0xEFEFEFEF, 0xEFEFEFEF, 0x30303030, 0x60606060, 0xFFFFFFFF,
  0x60606060, 0x10101010, 0xAFAFAFAF, 0xFFFFFFFF, 0xFFFFFFFF, 0xAFAFAFAF, 0x10101010, 0x60606060,
  0x40404040, 0x40404040, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x40404040, 0x40404040,
  0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF
#endif
};

static GUI_CONST_STORAGE GUI_BITMAP bmUp = {
  8, // XSize
  8, // YSize
  32, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)acUp,  // Pointer to picture data
  NULL,  // Pointer to palette
#if (GUI_USE_ARGB == 1)
  GUI_DRAW_BMPM8888I
#else
  GUI_DRAW_BMP8888
#endif
};

static GUI_CONST_STORAGE U32 _acBackButton_53x16[] = {
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x004E4E4E, 0x00000000, 0x00000000, 0x00000000, 0x60595959, 0xF77D7D7D, 0xFF9F9F9F, 0xFF9D9D9D, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 
        0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 
        0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9F9F9F, 0xFF818181, 0xC45D5D5D,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x004E4E4E, 0x00000000, 0x00000000, 0x04151515, 0x5A535353, 0xF3898989, 0xFFE1E1E1, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 
        0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 
        0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFF2F2F2, 0xFFFCFCFC, 0xFF808080,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x08333333, 0x5F575757, 0xED8A8A8A, 0xFFDADADA, 0xFFECECEC, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 
        0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 
        0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFF0F0F0, 0xFF9C9C9C,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x004B4B4B, 0x00000000, 0x05191919, 0x60575757, 0xEA8C8C8C, 0xFFD4D4D4, 0xFFEAEAEA, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 
        0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 
        0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFE8E8E8, 0xFFEDEDED, 0xFF999999,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x004C4C4C, 0x00000000, 0x031B1B1B, 0x58515151, 0xE88A8A8A, 0xFFD1D1D1, 0xFFE8E8E8, 0xFFE6E6E6, 0xFFE6E6E6, 0xFFE6E6E6, 0xFFE6E6E6, 0xFFE6E6E6, 0xFFE6E6E6, 0xFFE6E6E6, 0xFFE6E6E6, 
        0xFFE6E6E6, 0xFFE6E6E6, 0xFFE6E6E6, 0xFFEFEFEF, 0xFFEFEFEF, 0xFFEFEFEF, 0xFFEFEFEF, 0xFFEFEFEF, 0xFFEFEFEF, 0xFFEFEFEF, 0xFFEFEFEF, 0xFFEFEFEF, 0xFFEFEFEF, 0xFFEFEFEF, 0xFFEFEFEF, 0xFFEFEFEF, 0xFFEFEFEF, 0xFFEFEFEF, 0xFFEFEFEF, 0xFFEFEFEF, 
        0xFFEFEFEF, 0xFFECECEC, 0xFFE7E7E7, 0xFFE6E6E6, 0xFFE6E6E6, 0xFFE6E6E6, 0xFFE6E6E6, 0xFFE6E6E6, 0xFFE6E6E6, 0xFFE6E6E6, 0xFFEBEBEB, 0xFF989898,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x004A4A4A, 0x00000000, 0x01202020, 0x5A4D4D4D, 0xEC858585, 0xFFD2D2D2, 0xFFE6E6E6, 0xFFE4E4E4, 0xFFE4E4E4, 0xFFE4E4E4, 0xFFE4E4E4, 0xFFE4E4E4, 0xFFE4E4E4, 0xFFE4E4E4, 0xFFE4E4E4, 0xFFE4E4E4, 
        0xFFE4E4E4, 0xFFE4E4E4, 0xFFDDDDDD, 0xFF777777, 0xFF7E7E7E, 0xFF7E7E7E, 0xFF7E7E7E, 0xFF7E7E7E, 0xFF7E7E7E, 0xFF7E7E7E, 0xFF7E7E7E, 0xFF7E7E7E, 0xFF7E7E7E, 0xFF7E7E7E, 0xFF7E7E7E, 0xFF7E7E7E, 0xFF7E7E7E, 0xFF7E7E7E, 0xFF7E7E7E, 0xFF7E7E7E, 
        0xFF7B7B7B, 0xFF9B9B9B, 0xFFDADADA, 0xFFE4E4E4, 0xFFE4E4E4, 0xFFE4E4E4, 0xFFE4E4E4, 0xFFE4E4E4, 0xFFE4E4E4, 0xFFE4E4E4, 0xFFE9E9E9, 0xFF969696,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x004A4A4A, 0x00000000, 0x00000000, 0x57494949, 0xF4818181, 0xFFD6D6D6, 0xFFE3E3E3, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 
        0xFFE1E1E1, 0xFFE1E1E1, 0xFFE0E0E0, 0xFFC3C3C3, 0xFFBEBEBE, 0xFFBFBFBF, 0xFFBFBFBF, 0xFFBFBFBF, 0xFFBFBFBF, 0xFFBFBFBF, 0xFFBFBFBF, 0xFFBFBFBF, 0xFFBFBFBF, 0xFFBFBFBF, 0xFFBFBFBF, 0xFFBFBFBF, 0xFFBFBFBF, 0xFFBFBFBF, 0xFFBFBFBF, 0xFFBFBFBF, 
        0xFFBEBEBE, 0xFFC3C3C3, 0xFFDCDCDC, 0xFFE2E2E2, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE6E6E6, 0xFF959595,
  0x00000000, 0x00000000, 0x004C4C4C, 0x005E5E5E, 0x00000000, 0x00000000, 0x51434343, 0xF97D7D7D, 0xFFD9D9D9, 0xFFE0E0E0, 0xFFDFDFDF, 0xFFDFDFDF, 0xFFDFDFDF, 0xFFDFDFDF, 0xFFDFDFDF, 0xFFDFDFDF, 0xFFDFDFDF, 0xFFDFDFDF, 0xFFDFDFDF, 0xFFDFDFDF, 0xFFDFDFDF, 
        0xFFDFDFDF, 0xFFDFDFDF, 0xFFDFDFDF, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 0xFFE1E1E1, 
        0xFFE1E1E1, 0xFFE0E0E0, 0xFFDFDFDF, 0xFFDFDFDF, 0xFFDFDFDF, 0xFFDFDFDF, 0xFFDFDFDF, 0xFFDFDFDF, 0xFFDFDFDF, 0xFFDFDFDF, 0xFFE4E4E4, 0xFF949494,
  0x00525252, 0x00000000, 0x00000000, 0x00404040, 0x0A4B4B4B, 0x55484848, 0xF67D7D7D, 0xFFD8D8D8, 0xFFDDDDDD, 0xFFDDDDDD, 0xFFDDDDDD, 0xFFDDDDDD, 0xFFDDDDDD, 0xFFDDDDDD, 0xFFDDDDDD, 0xFFDDDDDD, 0xFFDDDDDD, 0xFFDDDDDD, 0xFFDDDDDD, 0xFFDDDDDD, 0xFFDDDDDD, 
        0xFFDDDDDD, 0xFFDDDDDD, 0xFFD8D8D8, 0xFF8A8A8A, 0xFF8F8F8F, 0xFF8F8F8F, 0xFF8F8F8F, 0xFF8F8F8F, 0xFF8F8F8F, 0xFF8F8F8F, 0xFF8F8F8F, 0xFF8F8F8F, 0xFF8F8F8F, 0xFF8F8F8F, 0xFF8F8F8F, 0xFF8F8F8F, 0xFF8F8F8F, 0xFF8F8F8F, 0xFF8F8F8F, 0xFF8F8F8F, 
        0xFF8D8D8D, 0xFFABABAB, 0xFFD7D7D7, 0xFFDDDDDD, 0xFFDDDDDD, 0xFFDDDDDD, 0xFFDDDDDD, 0xFFDDDDDD, 0xFFDDDDDD, 0xFFDDDDDD, 0xFFE2E2E2, 0xFF939393,
  0x00000000, 0x00000000, 0x014D4D4D, 0x164C4C4C, 0x70545454, 0xF3818181, 0xFFD2D2D2, 0xFFDBDBDB, 0xFFDBDBDB, 0xFFDBDBDB, 0xFFDBDBDB, 0xFFDBDBDB, 0xFFDBDBDB, 0xFFDBDBDB, 0xFFDBDBDB, 0xFFDBDBDB, 0xFFDBDBDB, 0xFFDBDBDB, 0xFFDBDBDB, 0xFFDBDBDB, 0xFFDBDBDB, 
        0xFFDBDBDB, 0xFFDBDBDB, 0xFFD8D8D8, 0xFFACACAC, 0xFFAAAAAA, 0xFFABABAB, 0xFFABABAB, 0xFFABABAB, 0xFFABABAB, 0xFFABABAB, 0xFFABABAB, 0xFFABABAB, 0xFFABABAB, 0xFFABABAB, 0xFFABABAB, 0xFFABABAB, 0xFFABABAB, 0xFFABABAB, 0xFFABABAB, 0xFFABABAB, 
        0xFFAAAAAA, 0xFFB0B0B0, 0xFFD3D3D3, 0xFFDBDBDB, 0xFFDBDBDB, 0xFFDBDBDB, 0xFFDBDBDB, 0xFFDBDBDB, 0xFFDBDBDB, 0xFFDBDBDB, 0xFFDFDFDF, 0xFF929292,
  0x00000000, 0x014A4A4A, 0x1D505050, 0x7F5B5B5B, 0xF0868686, 0xFFCCCCCC, 0xFFDADADA, 0xFFD8D8D8, 0xFFD8D8D8, 0xFFD8D8D8, 0xFFD8D8D8, 0xFFD8D8D8, 0xFFD8D8D8, 0xFFD8D8D8, 0xFFD8D8D8, 0xFFD8D8D8, 0xFFD8D8D8, 0xFFD8D8D8, 0xFFD8D8D8, 0xFFD8D8D8, 0xFFD8D8D8, 
        0xFFD8D8D8, 0xFFD8D8D8, 0xFFD9D9D9, 0xFFE1E1E1, 0xFFE0E0E0, 0xFFE0E0E0, 0xFFE0E0E0, 0xFFE0E0E0, 0xFFE0E0E0, 0xFFE0E0E0, 0xFFE0E0E0, 0xFFE0E0E0, 0xFFE0E0E0, 0xFFE0E0E0, 0xFFE0E0E0, 0xFFE0E0E0, 0xFFE0E0E0, 0xFFE0E0E0, 0xFFE0E0E0, 0xFFE0E0E0, 
        0xFFE1E1E1, 0xFFDFDFDF, 0xFFD9D9D9, 0xFFD8D8D8, 0xFFD8D8D8, 0xFFD8D8D8, 0xFFD8D8D8, 0xFFD8D8D8, 0xFFD8D8D8, 0xFFD8D8D8, 0xFFDDDDDD, 0xFF919191,
  0x013E3E3E, 0x18494949, 0x7E5A5A5A, 0xEF898989, 0xFFC7C7C7, 0xFFD8D8D8, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 
        0xFFD6D6D6, 0xFFD6D6D6, 0xFFD3D3D3, 0xFF9C9C9C, 0xFFA0A0A0, 0xFFA0A0A0, 0xFFA0A0A0, 0xFFA0A0A0, 0xFFA0A0A0, 0xFFA0A0A0, 0xFFA0A0A0, 0xFFA0A0A0, 0xFFA0A0A0, 0xFFA0A0A0, 0xFFA0A0A0, 0xFFA0A0A0, 0xFFA0A0A0, 0xFFA0A0A0, 0xFFA0A0A0, 0xFFA0A0A0, 
        0xFF9F9F9F, 0xFFB2B2B2, 0xFFD2D2D2, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFDBDBDB, 0xFF8F8F8F,
  0x164B4B4B, 0x75535353, 0xED848484, 0xFFC5C5C5, 0xFFD6D6D6, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 
        0xFFD4D4D4, 0xFFD4D4D4, 0xFFD0D0D0, 0xFF969696, 0xFF989898, 0xFF989898, 0xFF989898, 0xFF989898, 0xFF989898, 0xFF989898, 0xFF989898, 0xFF989898, 0xFF989898, 0xFF989898, 0xFF989898, 0xFF989898, 0xFF989898, 0xFF989898, 0xFF989898, 0xFF989898, 
        0xFF969696, 0xFFA9A9A9, 0xFFCECECE, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD8D8D8, 0xFF8E8E8E,
  0x72525252, 0xF0808080, 0xFFC5C5C5, 0xFFD4D4D4, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 
        0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD4D4D4, 0xFFD3D3D3, 0xFFD3D3D3, 0xFFD3D3D3, 0xFFD3D3D3, 0xFFD3D3D3, 0xFFD3D3D3, 0xFFD3D3D3, 0xFFD3D3D3, 0xFFD3D3D3, 0xFFD3D3D3, 0xFFD3D3D3, 0xFFD3D3D3, 0xFFD3D3D3, 0xFFD3D3D3, 0xFFD3D3D3, 0xFFD3D3D3, 
        0xFFD3D3D3, 0xFFD1D1D1, 0xFFD1D1D1, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD2D2D2, 0xFFD6D6D6, 0xFF8F8F8F,
  0xF6626262, 0xFFCFCFCF, 0xFFD5D5D5, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 
        0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 
        0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD4D4D4, 0xFFD3D3D3, 0xFFDCDCDC, 0xFF747474,
  0xC4515151, 0xFF747474, 0xFF8E8E8E, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 
        0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 
        0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8C8C8C, 0xFF8E8E8E, 0xFF747474, 0xC4525252
};

GUI_CONST_STORAGE GUI_BITMAP bmBackButton_53x16 = {
  53, // xSize
  16, // ySize
  212, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)_acBackButton_53x16,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMPM8888I
};

/*********************************************************************
*
*       _aButtonProp: Colors for close button of notepad
*/
static BUTTON_SKINFLEX_PROPS _PropsPressed = {
#if (GUI_USE_ARGB == 1)
  { 0xFF2C628B, 0xFF9EB0BA, 0xFF98D1EF },
  { 0xFFE5F4FC, 0xFFC4E5F6 },
  { 0xFF98D1EF, 0xFF68B3DB },
#else
  { 0x008B622C, 0x00BAB09E, 0x00EFD198 },
  { 0x00FCF4E5, 0x00F6E5C4 },
  { 0x00EFD198, 0x00DBB368 },
#endif
  BUTTON_SKINFLEX_RADIUS
};

static BUTTON_SKINFLEX_PROPS _PropsFocussed = {
#if (GUI_USE_ARGB == 1)
  { 0xFF3C7FB1, 0xFF46D8FB, 0xFFDFDFDF },
  { 0xFFF3F3F3, 0xFFECECEC },
  { 0xFFDFDFDF, 0xFFD0D0D0 },
#else
  { 0x00B17F3C, 0x00FBD846, 0x00DFDFDF },
  { 0x00F3F3F3, 0x00ECECEC },
  { 0x00DFDFDF, 0x00D0D0D0 },
#endif
  BUTTON_SKINFLEX_RADIUS
};

static BUTTON_SKINFLEX_PROPS _PropsEnabled = {
#if (GUI_USE_ARGB == 1)
  { 0xFF431422, 0xFFF4D3CC, 0xFFDF9587 },
  { 0xFFE9A99C, 0xFFDF9587 },
  { 0xFFD37E6F, 0xFFDF9587 },
#else
  { 0x00221443, 0x00CCD3F4, 0x008795DF },
  { 0x009CA9E9, 0x008795DF },
  { 0x006F7ED3, 0x008795DF },
#endif
  BUTTON_SKINFLEX_RADIUS
};

static BUTTON_SKINFLEX_PROPS _PropsDisabled = {
#if (GUI_USE_ARGB == 1)
  { 0xFFADB2B5, 0xFFFCFCFC, 0xFFF4F4F4 },
  { 0xFFF4F4F4, 0xFFF4F4F4 },
  { 0xFFF4F4F4, 0xFFF4F4F4 },
#else
  { 0x00B5B2AD, 0x00FCFCFC, 0x00F4F4F4 },
  { 0x00F4F4F4, 0x00F4F4F4 },
  { 0x00F4F4F4, 0x00F4F4F4 },
#endif
  BUTTON_SKINFLEX_RADIUS
};

static BUTTON_PROP _aButtonProp[] = {
  { &_PropsPressed,  BUTTON_SKINFLEX_PI_PRESSED  },
  { &_PropsFocussed, BUTTON_SKINFLEX_PI_FOCUSSED },
  { &_PropsEnabled,  BUTTON_SKINFLEX_PI_ENABLED  },
  { &_PropsDisabled, BUTTON_SKINFLEX_PI_DISABLED },
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _GetSpecialChar
*
* Function description
*   Returns upper case special characters
*/
static char _GetSpecialChar(char c) {
  switch(c) {
  case '1':
    return '!';
  case '2':
    return '@';
  case '3':
    return '#';
  case '4':
    return '$';
  case '5':
    return '%';
  case '6':
    return '^';
  case '7':
    return '&';
  case '8':
    return '*';
  case '9':
    return '(';
  case '0':
    return ')';
  case '-':
    return '_';
  case '=':
    return '+';
  case ';':
    return ':';
  case '\'':
    return '"';
  case ',':
    return '<';
  case '.':
    return '>';
  case '/':
    return '?';
  case '`':
    return '~';
  default:
    return 0;
  }
}

/*********************************************************************
*
*       _DrawCentered
*
* Function description
*   Draws the button bitmap
*/
static void _DrawCentered(WM_HWIN hWin, const GUI_BITMAP * pBM) {
  int xSizeWin;
  int ySizeWin;
  int xSizeBMP;
  int ySizeBMP;
  int xPos;
  int yPos;

  xSizeWin = WM_GetWindowSizeX(hWin);
  ySizeWin = WM_GetWindowSizeY(hWin);
  xSizeBMP = pBM->XSize;
  ySizeBMP = pBM->YSize;
  xPos = (xSizeWin - xSizeBMP) >> 1;
  yPos = (ySizeWin - ySizeBMP) >> 1;
  GUI_DrawBitmap(pBM, xPos, yPos);
}

/*********************************************************************
*
*       _DispSCentered
*
* Function description
*   Draws the button string
*/
static void _DispSCentered(WM_HWIN hWin, const char * s) {
  GUI_RECT Rect;

  Rect.x0 = 0;
  Rect.y0 = 0;
  Rect.x1 = WM_GetWindowSizeX(hWin) - 1;
  Rect.y1 = WM_GetWindowSizeY(hWin) - 1;
  GUI_SetTextMode(GUI_TM_TRANS);
  GUI_SetColor(GUI_BLACK);
  GUI_SetFont(BUTTON_GetDefaultFont());
    //GUI_SetFont(GUI_FONT_32_1);

  GUI_DispStringInRect(s, &Rect, GUI_TA_HCENTER | GUI_TA_VCENTER);
}

/*********************************************************************
*
*       _DrawBkSpc
*       _DrawReturn
*       _DrawShift
*/
static void _DrawBkSpc (WM_HWIN hWin) {  _DrawCentered(hWin, &bmBackSpace); }
static void _DrawReturn(WM_HWIN hWin) {  _DrawCentered(hWin, &bmReturn); }
static void _DrawShift (WM_HWIN hWin) {  _DrawCentered(hWin, &bmUp); }
static void _DrawCtrl  (WM_HWIN hWin) {  _DispSCentered(hWin, "Ctrl"); }
static void _DrawAlt   (WM_HWIN hWin) {  _DispSCentered(hWin, "Alt"); }
static void _DrawAltGr (WM_HWIN hWin) {  _DispSCentered(hWin, "Alt"); }

/*********************************************************************
*
*       Static data using function pointers
*
**********************************************************************
*/
static const BUTTON_DATA _aButtonData[] = {
  // First row
  {  23,   4,  36+18, 36+36, "1", NULL, 2 },  // !
  {  61+18,   4,  36+18, 36+36, "2", NULL, 2 },  // @
  {  99+18*2,   4,  36+18, 36+36, "3", NULL, 2 },  // #
  { 137+18*3,   4,  36+18, 36+36, "4", NULL, 2 },  // $
  { 175+18*4,   4,  36+18, 36+36, "5", NULL, 2 },  // %
  { 213+18*5,   4,  36+18, 36+36, "6", NULL, 2 },  // ^
  { 251+18*6,   4,  36+18, 36+36, "7", NULL, 2 },  // &
  { 289+18*7,   4,  36+18, 36+36, "8", NULL, 2 },  // *
  { 327+18*8,   4,  36+18, 36+36, "9", NULL, 2 },  // (
  { 365+18*9,   4,  36+18, 36+36, "0", NULL, 2 },  // )
  { 403+18*10,   4,  64+18, 36+36, "\x08", _DrawBkSpc, 0 },
  // Second row
  {  13,  42+36,  36+18, 36+36, "Q", NULL, 1 },
  {  51+18,  42+36,  36+18, 36+36, "W", NULL, 1 },
  {  89+18*2,  42+36,  36+18, 36+36, "E", NULL, 1 },
  { 127+18*3,  42+36,  36+18, 36+36, "R", NULL, 1 },
  { 165+18*4,  42+36,  36+18, 36+36, "T", NULL, 1 },
  { 203+18*5,  42+36,  36+18, 36+36, "Y", NULL, 1 },
  { 241+18*6,  42+36,  36+18, 36+36, "U", NULL, 1 },
  { 279+18*7,  42+36,  36+18, 36+36, "I", NULL, 1 },
  { 317+18*8,  42+36,  36+18, 36+36, "O", NULL, 1 },
  { 355+18*9,  42+36,  36+18, 36+36, "P", NULL, 1 },
  { 393+18*10,  42+36,  36+18, 36+36, "-", NULL, 2 },  // _
  { 431+18*11,  42+36,  36+18, 36+36, "=", NULL, 2 },  // +
  // Third row
  {  23,  80+72,  36+18, 36+36, "A", NULL, 1 },
  {  61+18,  80+72,  36+18, 36+36, "S", NULL, 1 },
  {  99+18*2,  80+72,  36+18, 36+36, "D", NULL, 1 },
  { 137+18*3,  80+72,  36+18, 36+36, "F", NULL, 1 },
  { 175+18*4,  80+72,  36+18, 36+36, "G", NULL, 1 },
  { 213+18*5,  80+72,  36+18, 36+36, "H", NULL, 1 },
  { 251+18*6,  80+72,  36+18, 36+36, "J", NULL, 1 },
  { 289+18*7,  80+72,  36+18, 36+36, "K", NULL, 1 },
  { 327+18*8,  80+72,  36+18, 36+36, "L", NULL, 1 },
  { 365+18*9,  80+72,  36+18, 36+36, ";", NULL, 2 },  // :
  { 403+18*10,  80+72,  36+18, 36+36, "'", NULL, 2 },  // "
  // Fourth row
  {  13, 118+108,  36+18, 36+36, "\x19", _DrawShift, 0 },
  {  51+18, 118+108,  36+18, 36+36, "Z", NULL, 1 },
  {  89+18*2, 118+108,  36+18, 36+36, "X", NULL, 1 },
  { 127+18*3, 118+108,  36+18, 36+36, "C", NULL, 1 },
  { 165+18*4, 118+108,  36+18, 36+36, "V", NULL, 1 },
  { 203+18*5, 118+108,  36+18, 36+36, "B", NULL, 1 },
  { 241+18*6, 118+108,  36+18, 36+36, "N", NULL, 1 },
  { 279+18*7, 118+108,  36+18, 36+36, "M", NULL, 1 },
  { 317+18*8, 118+108,  36+18, 36+36, ",", NULL, 2 },  // <
  { 355+18*9, 118+108,  36+18, 36+36, ".", NULL, 2 },  // >
  { 393+18*10, 118+108,  36+18, 36+36, "/", NULL, 2 },  // ?
  { 431+18*11, 118+108,  36+18, 36+36, "`", NULL, 2 },  // ~
  // Fifth row
  {  23, 156+144,  36+18, 36+36, "\x1a", _DrawCtrl,   0 },
  {  61+18, 156+144,  36+18, 36+36, "",     _DrawAlt,    0 },  // no function
  {  99+18*2, 156+144, 226+18*6, 36+36, " ",    NULL,        0 },
  { 327+18*8, 156+144,  36+18, 36+36, "",     _DrawAltGr,  0 },  // no function
  { 365+18*9, 156+144,  36+18, 36+36, "\x1a", _DrawCtrl,   0 },
  { 403+18*10, 156+144,  64+18, 36+36, "\x0d", _DrawReturn, 0 },
};

static GUI_ANIM_HANDLE hAnim;
static ANIM_DATA       AnimData;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _AnimateWin
*/
static void _AnimateWin(GUI_ANIM_INFO * pInfo, void * pVoid) {
  int         yPos;
  ANIM_DATA * pData;

  pData = (ANIM_DATA *)pVoid;
  if (pData->Dir) {
    yPos = (LCD_GetYSize() - SIZE_Y_KEYPAD) + ((SIZE_Y_KEYPAD * pInfo->Pos) / GUI_ANIM_RANGE);
  } else {
    yPos = LCD_GetYSize() - ((SIZE_Y_KEYPAD * pInfo->Pos) / GUI_ANIM_RANGE);
  }
  WM_MoveTo(pData->hWin,    0, yPos);
  WM_MoveTo(pData->hBackButton, 480/*LCD_GetXSize()*/ - BM_BACKBUTTON.XSize, yPos - BM_BACKBUTTON.YSize);
}

/*********************************************************************
*
*       _AnimateDelete
*/
static void _AnimateDelete(void * pVoid) {
  ANIM_DATA * pData;

  pData = (ANIM_DATA *)pVoid;
  pData->Dir ^= 1;
}

/*********************************************************************
*
*       _AnimateCreate
*/
static void _AnimateCreate(GUI_ANIM_HANDLE * pAnim, ANIM_DATA * pAnimData) {
  *pAnim = GUI_ANIM_Create(ANIMATION_TIME, 10, (void *)pAnimData, 0);
  GUI_ANIM_AddItem(*pAnim, 0, ANIMATION_TIME, ANIM_ACCELDECEL, (void *)pAnimData, _AnimateWin);
  GUI_ANIM_StartEx(*pAnim, 1, _AnimateDelete);
}

/*********************************************************************
*
*       _DrawSkinFlex_BUTTON
*
* Function description
*   Skinning routine for keypad buttons
*/
static int _DrawSkinFlex_BUTTON(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  static GUI_MEMDEV_Handle ahMemEdges[4];
  GUI_MEMDEV_Handle        hMem;
  WM_HWIN                  hWin;
  unsigned                 i;
  int                      xPos;
  int                      yPos;
  int                      xSize;
  int                      ySize;
  int                      x;
  int                      y;
  int                      Id;
  int                      IsPressed;
  void (* pfDraw)(WM_HWIN hWin);

  x = 0;
  y = 0;
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_CREATE:
    if (ahMemEdges[0] == 0) {
      //
      // Create small helper window which never becomes visible
      //
      hWin = WM_CreateWindowAsChild(0, 0, 32, 32, WM_HBKWIN, WM_CF_SHOW | WM_CF_STAYONTOP, NULL, 0);
      WM_SelectWindow(hWin);
      //
      // Create memory devices for the edges of the buttons
      //
      for (i = 0; i < GUI_COUNTOF(ahMemEdges); i++) {
        switch (i) {
        case 0:
          x = 0;
          y = 0;
          break;
        case 1:
          x = -16;
          y = 0;
          break;
        case 2:
          x = -16;
          y = -16;
          break;
        case 3:
          x = 0;
          y = -16;
          break;
        }
        //
        // Create magnified device
        //
        hMem = GUI_MEMDEV_CreateFixed32(0, 0, 16, 16);
        GUI_MEMDEV_Select(hMem);
        GUI_SetBkColor(GUI_TRANSPARENT);
        GUI_Clear();
        GUI_SetColor(COLOR_BORDER);
        GUI_DrawRoundedFrame(x, y, x + 31, y + 31, 16, 4);
        //
        // Shrink to required size
        //
        ahMemEdges[i] = GUI_MEMDEV_CreateFixed32(0, 0, 4, 4);
        GUI_MEMDEV_Select(ahMemEdges[i]);
        GUI_SetBkColor(GUI_TRANSPARENT);
        GUI_Clear();
        GUI_MEMDEV_RotateHQ(hMem, ahMemEdges[i], -6, -6, 0, 250);
        GUI_MEMDEV_Delete(hMem);
        GUI_MEMDEV_Select(0);
      }
      WM_DeleteWindow(hWin);
    }
    break;
  case WIDGET_ITEM_DRAW_TEXT:
    //
    // Use private or default function for drawing bitmap or text
    //
    hWin = pDrawItemInfo->hWin;
    Id   = WM_GetId(hWin);
    i    = Id - ID_BUTTON;
    pfDraw = _aButtonData[i].pfDraw;
    if (pfDraw) {
      pfDraw(hWin);
    } else {
      BUTTON_DrawSkinFlex(pDrawItemInfo);
    }
    break;
  case WIDGET_ITEM_DRAW_BACKGROUND:
    //
    // Draw background of button
    //
    IsPressed = BUTTON_IsPressed(pDrawItemInfo->hWin);
    xPos      = WM_GetWindowOrgX(pDrawItemInfo->hWin);
    yPos      = WM_GetWindowOrgY(pDrawItemInfo->hWin);
    xSize     = WM_GetWindowSizeX(pDrawItemInfo->hWin);
    ySize     = WM_GetWindowSizeY(pDrawItemInfo->hWin);
    if (IsPressed) {
      GUI_SetColor(BUTTON_COLOR3);
    } else {
      GUI_SetColor(BUTTON_COLOR0);
    }
    GUI_FillRoundedRect(0, 0, xSize - 1, ySize - 1, 4);
    break;
  default:
    //
    // Use the default skinning routine for processing all other commands
    //
    return BUTTON_DrawSkinFlex(pDrawItemInfo);
  }
  return 0;
}

/*********************************************************************
*
*       _cbBackButton
*/
static void _cbBackButton(WM_MESSAGE * pMsg) {
  static int      Pressed;
  GUI_PID_STATE * pState;
  GUI_RECT        Rect;
  int             IsPressed;

  switch (pMsg->MsgId) {
  case WM_PAINT:
    WM_GetClientRect(&Rect);
    Rect.y1 += 10;
    IsPressed = BUTTON_IsPressed(pMsg->hWin);
    if (IsPressed) {
      GUI_SetColor(BUTTON_COLOR3);
    } else {
      GUI_SetColor(BUTTON_COLOR0);
    }
    GUI_FillRoundedRect(Rect.x0, Rect.y0, Rect.x1, Rect.y1, 4);
    GUI_SetColor(COLOR_BORDER);
    GUI_DrawRoundedRect(Rect.x0, Rect.y0, Rect.x1, Rect.y1, 4);
    GUI_DrawHLine(5 + 3, 15, Rect.x1 - 15);
    GUI_DrawHLine(5 + 5, 15, Rect.x1 - 15);
    GUI_DrawHLine(5 + 7, 15, Rect.x1 - 15);
    break;
  case WM_TOUCH:
    pState = (GUI_PID_STATE *)pMsg->Data.p;
    if (pState) {
      if (pState->Pressed && Pressed == 0) {
        _AnimateCreate(&hAnim, &AnimData);
        Pressed = 1;
      } else {
        Pressed = 0;
      }
    }
    BUTTON_Callback(pMsg);
    break;
  default:
    BUTTON_Callback(pMsg);
  }
}

/*********************************************************************
*
*       _ChangeButtonString
*/
static void _ChangeButtonString(WM_MESSAGE * pMsg, int HigherLower) {
  WM_HWIN hButton;
  int     i;
  char acBuffer[2];

  for (i = 0; i < GUI_COUNTOF(_aButtonData); i++) {
    if (_aButtonData[i].HasLowerCase == 0) {
      continue;
    }
    acBuffer[1] = '\0';
    hButton = WM_GetDialogItem(pMsg->hWin, ID_BUTTON + i);
    if(_aButtonData[i].HasLowerCase == 2 && !HigherLower) {
      acBuffer[0] = _GetSpecialChar(_aButtonData[i].acLabel[0]);
      goto SetText;
    }
    acBuffer[0] = _aButtonData[i].acLabel[0];
    if (HigherLower) {
      acBuffer[0] |= 0x20;
    }
SetText:
    BUTTON_SetText(hButton, acBuffer);
  }
}

/*********************************************************************
*
*       _cbKeyPad
*
* Function description
*   Callback routine of keypad window
*/
static void _cbKeyPad(WM_MESSAGE * pMsg) {
  static int LowerCase;
  WM_HWIN    hWin;
  WM_HWIN    hButton;
  char       c;
  int        Id;
  int        NCode;
  unsigned   i;

  hWin = pMsg->hWin;
  switch (pMsg->MsgId) {
  case APP_INIT_LOWERCASE:
    LowerCase = 1;
    break;
  case WM_CREATE:
    //
    // Create the keyboard buttons
    //
    for (i = 0; i < GUI_COUNTOF(_aButtonData); i++) {
      hButton = BUTTON_CreateEx(_aButtonData[i].xPos, _aButtonData[i].yPos, _aButtonData[i].xSize, _aButtonData[i].ySize, 
                                hWin, WM_CF_SHOW | WM_CF_HASTRANS, 0, ID_BUTTON + i);
      BUTTON_SetText(hButton, _aButtonData[i].acLabel);
      BUTTON_SetFocussable(hButton, 0);
        BUTTON_SetFont(hButton, GUI_FONT_32_1);

      BUTTON_SetSkin(hButton, _DrawSkinFlex_BUTTON);
    }
    //LowerCase ^= 1;
        LowerCase = 1;

    _ChangeButtonString(pMsg, LowerCase);
    AnimData.hBackButton = BUTTON_CreateEx(WM_GetWindowSizeX(pMsg->hWin) - BM_BACKBUTTON.XSize,
                              WM_GetWindowOrgY(pMsg->hWin) - BM_BACKBUTTON.YSize,
                              BM_BACKBUTTON.XSize,
                              BM_BACKBUTTON.YSize, 
                              WM_GetParent(hWin),
                              WM_CF_SHOW | WM_CF_HASTRANS,
                              0,
                              ID_BUTTON + i);
    BUTTON_SetFocussable(AnimData.hBackButton, 0);
    WM_SetCallback(AnimData.hBackButton, _cbBackButton);
    AnimData.Dir  = 0;
    AnimData.hWin = pMsg->hWin;
    break;
  case WM_PAINT:
    //
    // Draw background
    //
    GUI_SetBkColor(COLOR_BORDER);
    GUI_Clear();
    break;
  case MSG_ANIMATE:
    if (AnimData.Dir != pMsg->Data.v) {
      _AnimateCreate(&hAnim, &AnimData);
    }
    break;
  case WM_DELETE:
    WM_DeleteWindow(AnimData.hBackButton);
    break;
  case WM_NOTIFY_PARENT:
    //
    // Send key message to currently focused window
    //
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch (NCode) {
    case WM_NOTIFICATION_RELEASED:
      if (_aButtonData[Id - ID_BUTTON].acLabel) {
        c = _aButtonData[Id - ID_BUTTON].acLabel[0];        
        switch (c) {
        case '\0':
          return;
        case GUI_KEY_SHIFT:
          LowerCase ^= 1;
          _ChangeButtonString(pMsg, LowerCase);
          break;
        case GUI_KEY_ENTER:
          _AnimateCreate(&hAnim, &AnimData);
          //LowerCase = 0;
            LowerCase = 1;

          _ChangeButtonString(pMsg, LowerCase);
          break;
        case GUI_KEY_CONTROL:
          return;
        case GUI_KEY_BACKSPACE:
          //LowerCase = 0;
          //_ChangeButtonString(pMsg, LowerCase);
          break;
        default:
          if (LowerCase) {
            if (c < '0' || c > '9') {
              c |= 0x20;
            }
          } else {
            if(c >= '0' && c <= '9' || c == '-' || c == '=' || c == ';' || c == '\'' || c == ',' || c == '.' || c == '/' || c == '`') {
              c = _GetSpecialChar(c);
            }
          }
          break;
        }
        GUI_StoreKeyMsg(c, 1);
        GUI_StoreKeyMsg(c, 0);
        break;
      }
    }
  }
}

/*********************************************************************
*
*       _cbEdit
*/
#if 0
static void _cbEdit(WM_MESSAGE * pMsg) {
  GUI_RECT Rect;

  switch (pMsg->MsgId) {
  case WM_PAINT:
    EDIT_Callback(pMsg);
    if (EDIT_GetNumChars(pMsg->hWin) == 0) {
      WM_GetClientRect(&Rect);
      Rect.x0 += 5;
      GUI_SetFont(GUI_FONT_32_1);
      GUI_SetTextMode(GUI_TM_TRANS);
      GUI_SetColor(GUI_MAKE_COLOR(0x00F0F0F0));
      GUI_DispStringInRect("Click here", &Rect, GUI_TA_LEFT | GUI_TA_VCENTER);
    }
    break;
  default:
    EDIT_Callback(pMsg);
    break;
  }
}
#endif

/*********************************************************************
*
*       _cbDialog
*/
#if 0
static void _cbDialog(WM_MESSAGE * pMsg) {
  WM_HWIN        hItem;
  int            NCode;
  int            Id;
  static WM_HWIN hKeyPad;
  WM_MESSAGE     Message;

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    //
    // Initialization of 'Window'
    //
    hItem = pMsg->hWin;
    WINDOW_SetBkColor(hItem, GUI_MAKE_COLOR(0x00400040));
    //
    // Initialization of 'Edit'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_EDIT_0);
    EDIT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
    EDIT_SetFont(hItem, GUI_FONT_32B_1);
    EDIT_EnableBlink(hItem, 500, 1);
    WM_SetCallback(hItem, _cbEdit);
    //
    // Create keypad
    //
    hKeyPad = GUI_CreateKeyPad(pMsg->hWin);
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case ID_EDIT_0: // Notifications sent by 'Edit'
      switch(NCode) {
      case WM_NOTIFICATION_RELEASED:
        Message.Data.v = 1;  // 1 stand for show keypad
        Message.hWin   = hKeyPad;
        Message.MsgId  = MSG_ANIMATE;
        WM_SendMessage(hKeyPad, &Message);
        break;
      }
      break;
    }
    break;
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}
#endif

/*********************************************************************
*
*       _cbBk
*/
#if 0
static void _cbBk(WM_MESSAGE * pMsg) {
  switch (pMsg->MsgId) {
  case WM_PAINT:
    GUI_SetBkColor(GUI_BLACK);
    GUI_Clear();
    break;
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
WM_HWIN GUI_CreateKeyPad(WM_HWIN hParent) {
  return WM_CreateWindowAsChild(0, 480, 700, SIZE_Y_KEYPAD, hParent, WM_CF_SHOW | WM_CF_STAYONTOP, _cbKeyPad, 0);
}

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       MainTask
*/
#if 0
void MainTask(void) {
  GUI_Init();
  WM_MULTIBUF_Enable(1);
  WM_SetCallback(WM_HBKWIN, _cbBk);
  GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
  while (1) {
    GUI_Delay(100);
  }
}
#endif

/*************************** End of file ****************************/
