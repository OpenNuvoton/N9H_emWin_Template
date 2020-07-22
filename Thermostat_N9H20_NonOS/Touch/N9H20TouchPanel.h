/***************************************************************************
 *                                                                                                               *
 * Copyright (c) Nuvoton Technolog. All rights reserved.                                    *
 *                                                                                                               *
 ***************************************************************************/

#ifndef __N9H20TOUCHPANEL_H__
#define __N9H20TOUCHPANEL_H__

#define XSIZE_PHYS  480
#define YSIZE_PHYS  480

#define LCD_XSIZE       XSIZE_PHYS
#define LCD_YSIZE       YSIZE_PHYS


int Init_TouchPanel(void);
int Read_TouchPanel(int *x, int *y);
int Uninit_TouchPanel(void);
int Check_TouchPanel(void);
int Wait_PressDown(void);
#endif
