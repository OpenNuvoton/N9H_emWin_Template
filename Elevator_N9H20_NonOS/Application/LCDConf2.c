/*********************************************************************
*                 SEGGER Software GmbH                               *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2018  SEGGER Microcontroller GmbH                *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.48 - Graphical user interface for embedded applications **
All  Intellectual Property rights in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product. This file may
only be used in accordance with the following terms:

The  software has  been licensed by SEGGER Software GmbH to Nuvoton Technology Corporationat the address: No. 4, Creation Rd. III, Hsinchu Science Park, Taiwan
for the purposes  of  creating  libraries  for its
Arm Cortex-M and  Arm9 32-bit microcontrollers, commercialized and distributed by Nuvoton Technology Corporation
under  the terms and conditions  of  an  End  User
License  Agreement  supplied  with  the libraries.
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
Licensing information
Licensor:                 SEGGER Software GmbH
Licensed to:              Nuvoton Technology Corporation, No. 4, Creation Rd. III, Hsinchu Science Park, 30077 Hsinchu City, Taiwan
Licensed SEGGER software: emWin
License number:           GUI-00735
License model:            emWin License Agreement, signed February 27, 2018
Licensed platform:        Cortex-M and ARM9 32-bit series microcontroller designed and manufactured by or for Nuvoton Technology Corporation
----------------------------------------------------------------------
Support and Update Agreement (SUA)
SUA period:               2018-03-26 - 2019-03-27
Contact to extend SUA:    sales@segger.com
----------------------------------------------------------------------
File        : LCDConf.c
Purpose     : Display controller configuration (single layer)
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "GUIDRV_Lin.h"
#include "LCDConf.h"
#include "N9H20.h"
#include "N9H20TouchPanel.h"

/*********************************************************************
*
*       Layer configuration
*
**********************************************************************
*/
//
// Physical display size
//
//
// Color conversion
//
#define COLOR_CONVERSION GUICC_M565

//
// Display driver
//
//#define DISPLAY_DRIVER GUIDRV_FLEXCOLOR
#define DISPLAY_DRIVER GUIDRV_LIN_16
//
// Buffers / VScreens
//
#define NUM_BUFFERS  3 // Number of multiple buffers to be used
#define NUM_VSCREENS 1 // Number of virtual screens to be used

//
// Orientation
//
//#define DISPLAY_ORIENTATION (0)
//#define DISPLAY_ORIENTATION (GUI_MIRROR_X)
//#define DISPLAY_ORIENTATION (GUI_MIRROR_Y)
//#define DISPLAY_ORIENTATION (GUI_MIRROR_X | GUI_MIRROR_Y)
//#define DISPLAY_ORIENTATION (GUI_SWAP_XY)
//#define DISPLAY_ORIENTATION (GUI_MIRROR_X | GUI_SWAP_XY)
//#define DISPLAY_ORIENTATION (GUI_MIRROR_Y | GUI_SWAP_XY)
//#define DISPLAY_ORIENTATION (GUI_MIRROR_X | GUI_MIRROR_Y | GUI_SWAP_XY)


/*********************************************************************
*
*       Configuration checking
*
**********************************************************************
*/
#ifndef   VXSIZE_PHYS
#define VXSIZE_PHYS XSIZE_PHYS
#endif
#ifndef   VYSIZE_PHYS
#define VYSIZE_PHYS YSIZE_PHYS
#endif
#ifndef   XSIZE_PHYS
#error Physical X size of display is not defined!
#endif
#ifndef   YSIZE_PHYS
#error Physical Y size of display is not defined!
#endif
#ifndef   COLOR_CONVERSION
#error Color conversion not defined!
#endif
#ifndef   DISPLAY_DRIVER
#error No display driver defined!
#endif
#ifndef   DISPLAY_ORIENTATION
#define DISPLAY_ORIENTATION 0
#endif

// Touch panel
//#define GUI_TOUCH_AD_LEFT   72
//#define GUI_TOUCH_AD_TOP    926  //82
//#define GUI_TOUCH_AD_RIGHT  938
//#define GUI_TOUCH_AD_BOTTOM 82  //926
/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
extern UINT8 *u8FrameBufPtr;
//extern int ts_phy2log(int *sumx, int *sumy);

extern UINT8 u8FrameBuf[];






#if (NUM_BUFFERS > 1)
UINT32      g_address;
volatile int        g_pending_buffer;
//int     g_buffer_index;
INT32 g_VdmaCh = 0;
volatile BOOL g_bVdmaInt = FALSE;

void    VPOST_InterruptServiceRiuntine(void);

extern int vpostInstallCallBack(
    E_DRVVPOST_INT eIntSource,
    PFN_DRVVPOST_INT_CALLBACK   pfnCallback,
    PFN_DRVVPOST_INT_CALLBACK   *pfnOldCallback
);
extern VOID vpostEnableInt(E_DRVVPOST_INT eInt);
extern VOID vpostSetFrameBuffer(UINT32 pFramebuf);

void    VPOST_InterruptServiceRiuntine(void)
{
    UINT32 Addr;
    if ( g_pending_buffer >= 0 )
    {
//sysprintf("VPOST ISR\n");
        Addr = g_address +  XSIZE_PHYS*YSIZE_PHYS*2* g_pending_buffer;
        vpostSetFrameBuffer(Addr);
        GUI_MULTIBUF_Confirm(g_pending_buffer);
        g_pending_buffer = -1;
    }
}

void EdmaIrqHandler(unsigned int arg)
{
    EDMA_Free(g_VdmaCh);
    g_bVdmaInt = TRUE;
}

void DMA_CopyBuffer(int LayerIndex, UINT32 AddrSrc, UINT32 AddrDst, UINT32 BufferSize)
{
    g_bVdmaInt = FALSE;
    g_VdmaCh = VDMA_FindandRequest();

    if (g_VdmaCh < 0)
    {
        sysprintf("Request VDMA fail.\n");
        return;
    }
    EDMA_SetupSingle(g_VdmaCh, AddrSrc, AddrDst, BufferSize);
    EDMA_SetupHandlers(g_VdmaCh, eDRVEDMA_BLKD_FLAG, EdmaIrqHandler, 0);

    EDMA_Trigger(g_VdmaCh);

    while(g_bVdmaInt == FALSE);
}

static UINT32 GetBufferSize(UINT32 LayerIndex)
{
    return (YSIZE_PHYS*XSIZE_PHYS*2);
}


/**
  * @brief  Customized copy buffer
  * @param  LayerIndex : Layer Index
  * @param  IndexSrc:    index source
  * @param  IndexDst:    index destination
  * @retval None.
  */
static void LCD_CopyBuffer(int LayerIndex, int IndexSrc, int IndexDst)
{
    UINT32 BufferSize, AddrSrc, AddrDst;
//  if ( count_no < 30 )
//    sysprintf("COpy buffer src=%d, des=%d\n", IndexSrc, IndexDst);
    BufferSize = GetBufferSize(LayerIndex);
    AddrSrc    = g_address + BufferSize * IndexSrc;
    AddrDst    = g_address + BufferSize * IndexDst;
    DMA_CopyBuffer(LayerIndex, AddrSrc, AddrDst, BufferSize);
    //g_buffer_index = IndexDst;
}

#endif


void GUI_TOUCH_X_ActivateX(void)
{
}

void GUI_TOUCH_X_ActivateY(void)
{
}



int  GUI_TOUCH_X_MeasureX(void)
{
#if 0
    int sumx;
    int sumy;
    if (Read_TouchPanel(&sumx, &sumy))
    {
        sysprintf("X = %d\n", sumx);
        ts_phy2log(&sumx, &sumy);
        return sumx;
    }
#endif
    return -1;
}

int  GUI_TOUCH_X_MeasureY(void)
{
#if 0
    int sumx;
    int sumy;
    if ( Read_TouchPanel(&sumx, &sumy) )
    {
        sysprintf("Y = %d\n", sumy);
        ts_phy2log(&sumx, &sumy);
        return sumy;
    }
#endif
    return -1;
}
/*********************************************************************
*
*       LCD_X_Config
*
* Purpose:
*   Called during the initialization process in order to set up the
*   display driver configuration.
*
*/


void LCD_X_Config(void)
{

    // LCD calibration
#if (NUM_BUFFERS > 1)
    PFN_DRVVPOST_INT_CALLBACK fun_ptr;
    g_pending_buffer = -1;
    EDMA_Init();  // open edma
    vpostInstallCallBack(eDRVVPOST_VINT, (PFN_DRVVPOST_INT_CALLBACK)VPOST_InterruptServiceRiuntine,  (PFN_DRVVPOST_INT_CALLBACK*)&fun_ptr);
    vpostEnableInt(eDRVVPOST_VINT);
    GUI_MULTIBUF_Config(NUM_BUFFERS);
#endif
    //
    // Set display driver and color conversion for 1st layer
    //
    GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER, COLOR_CONVERSION, 0, 0);
#if 1
    if (LCD_GetSwapXY())
    {
        LCD_SetSizeEx (0, YSIZE_PHYS, XSIZE_PHYS);
        LCD_SetVSizeEx(0, YSIZE_PHYS * NUM_VSCREENS, XSIZE_PHYS);
    }
    else
    {
        LCD_SetSizeEx (0, XSIZE_PHYS, YSIZE_PHYS);
        LCD_SetVSizeEx(0, XSIZE_PHYS, YSIZE_PHYS * NUM_VSCREENS);
    }
#else
    if (LCD_GetSwapXYEx(0))
    {
        LCD_SetSizeEx (0, _InitApp.aLayer[0].ySize, _InitApp.aLayer[0].xSize);
        LCD_SetVSizeEx(0, _InitApp.aLayer[0].ySize, _InitApp.aLayer[0].xSize);
    }
    else
    {
        LCD_SetSizeEx (0, _InitApp.aLayer[0].xSize, _InitApp.aLayer[0].ySize);
        LCD_SetVSizeEx(0, _InitApp.aLayer[0].xSize, _InitApp.aLayer[0].ySize);
    }
    GUI_SOFTLAYER_Enable(_InitApp.aLayer, NUM_LAYER, GUI_DARKBLUE);
#endif
#if (NUM_BUFFERS > 1)
    g_address = (UINT32)u8FrameBufPtr;

    /* Set custom functions for several operations */
    LCD_SetDevFunc(0, LCD_DEVFUNC_COPYBUFFER, (void(*)(void))LCD_CopyBuffer);

#endif
    LCD_SetVRAMAddrEx(0, (void *)u8FrameBufPtr);

    //
    // Set user palette data (only required if no fixed palette is used)
    //
#if defined(PALETTE)
    LCD_SetLUTEx(0, PALETTE);
#endif

#if 0
// LCD calibration
//
// Calibrate touch screen
//
    GUI_TOUCH_Calibrate(GUI_COORD_X, 0, XSIZE_PHYS, 0, XSIZE_PHYS);
    GUI_TOUCH_Calibrate(GUI_COORD_Y, 0, YSIZE_PHYS, 0, YSIZE_PHYS);
#endif

}

/*********************************************************************
*
*       LCD_X_DisplayDriver
*
* Purpose:
*   This function is called by the display driver for several purposes.
*   To support the according task the routine needs to be adapted to
*   the display controller. Please note that the commands marked with
*   'optional' are not cogently required and should only be adapted if
*   the display controller supports these features.
*
* Parameter:
*   LayerIndex - Index of layer to be configured
*   Cmd        - Please refer to the details in the switch statement below
*   pData      - Pointer to a LCD_X_DATA structure
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData)
{
    int r;

    switch (Cmd)
    {
    //
    // Required
    //
    case LCD_X_INITCONTROLLER:
    {
        //
        // Called during the initialization process in order to set up the
        // display controller and put it into operation. If the display
        // controller is not initialized by any external routine this needs
        // to be adapted by the customer...
        //
//        LCD_ON();
        return 0;
    }
    case LCD_X_SETVRAMADDR:
    {
        //
        // Required for setting the address of the video RAM for drivers
        // with memory mapped video RAM which is passed in the 'pVRAM' element of p
        //
        LCD_X_SETVRAMADDR_INFO * p;
        p = (LCD_X_SETVRAMADDR_INFO *)pData;
        pData=(void*)u8FrameBufPtr;
        GUI_USE_PARA(p);
        //...
        return 0;
    }
    case LCD_X_SETORG:
    {
        //
        // Required for setting the display origin which is passed in the 'xPos' and 'yPos' element of p
        //
        LCD_X_SETORG_INFO * p;
        p = (LCD_X_SETORG_INFO *)pData;
        pData=(void*)u8FrameBufPtr;
        GUI_USE_PARA(p);
        //...
        return 0;
    }
    case LCD_X_SHOWBUFFER:
    {
        //
        // Required if multiple buffers are used. The 'Index' element of p contains the buffer index.
        //
        LCD_X_SHOWBUFFER_INFO * p;
        p = (LCD_X_SHOWBUFFER_INFO *)pData;
#if (NUM_BUFFERS > 1)
        g_pending_buffer = p->Index;
//              GUI_MULTIBUF_Confirm(g_pending_buffer);
//          sysprintf("Show %d\n", g_pending_buffer);
#endif
        GUI_USE_PARA(p);
        //...
        return 0;
    }
    case LCD_X_SETLUTENTRY:
    {
        //
        // Required for setting a lookup table entry which is passed in the 'Pos' and 'Color' element of p
        //
        LCD_X_SETLUTENTRY_INFO * p;
        p = (LCD_X_SETLUTENTRY_INFO *)pData;
        GUI_USE_PARA(p);
        //...
        return 0;
    }
    case LCD_X_ON:
    {
        //
        // Required if the display controller should support switching on and off
        //

        return 0;
    }
    case LCD_X_OFF:
    {
        //
        // Required if the display controller should support switching on and off
        //
        // ...
        return 0;
    }
    default:
        r = -1;
    }
    return r;
}

/*************************** End of file ****************************/
