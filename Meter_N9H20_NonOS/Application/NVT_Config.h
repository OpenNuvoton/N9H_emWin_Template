/**************************************************************************//**
 * @file     NVT_Config.h
 * @brief    Nuvoton config header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#ifndef __NVT_CONFIG_H__
#define __NVT_CONFIG_H__

#include "N9H20.h"
#include "GUI.h"
#include "WM.h"

int JPEG_X_Draw(GUI_GET_DATA_FUNC * pfGetData, void * p, int x0, int y0);

U32 NVT_GetImage(char * szFileName, UINT8 * pu8Image);

void NVT_DrawBitmap(char * szFileName, UINT8 * pu8Image, int x, int y);

void NVT_Logo1(int x, int y);
void NVT_Meter1(int x, int y);

void NVT_CxtUpdate(void);

#endif
