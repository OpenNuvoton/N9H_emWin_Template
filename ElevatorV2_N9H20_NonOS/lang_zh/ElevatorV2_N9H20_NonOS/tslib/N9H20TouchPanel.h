/**************************************************************************//**
 * @file     N9H20TouchPanel.h
 * @brief    N9H20 series emWin touch header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#ifndef __N9H20TOUCHPANEL_H__
#define __N9H20TOUCHPANEL_H__

/*********************************************************************
Constraints on LCD panel N9H20_VPOST_FW050TFT_800x480.lib:

1. Resolution is 800x480. (N9H20K1 has no such target because of the DRAM size limitation)
2. VPOST OSD function must disable. (VPOST H/W limitation)
3. PLL clock is 192 MHz, the pixel clock is 192/6 = 32 MHz.
4. 32,000,000/1056/525 = 57.7 FPS (frame per second)
5. If the bus bandwidth condition is too busy it may causes blinking result.
*/

#ifdef __800x480__
#define XSIZE_PHYS  800
#define YSIZE_PHYS  480
#else
#ifdef __480x272__
#define XSIZE_PHYS  480
#define YSIZE_PHYS  272
//
// Bounding box of floor number, up & down (OSD layer)
//
#define XSIZE_PHYS_OSD  (127 + 43 + 100 + 47 + 21)
#define YSIZE_PHYS_OSD  (73 + 7 + 73)
#else
#define XSIZE_PHYS  320
#define YSIZE_PHYS  240
#endif
#endif
//
// Background layer size
//
#define LCD_XSIZE       XSIZE_PHYS
#define LCD_YSIZE       YSIZE_PHYS
//
// OSD layer size
//
#define LCD_XSIZE_OSD       XSIZE_PHYS_OSD
#define LCD_YSIZE_OSD       YSIZE_PHYS_OSD


int Init_TouchPanel(void);
int Read_TouchPanel(int *x, int *y);
int Uninit_TouchPanel(void);
int Check_TouchPanel(void);
int Wait_PressDown(void);
#endif
