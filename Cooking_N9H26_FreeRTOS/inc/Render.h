//      Copyright (c) 2019 Nuvoton Technology Corp. All rights reserved.
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either m_uiVersion 2 of the License, or
//      (at your option) any later m_uiVersion.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.

#ifndef __RENDER_H__
#define __RENDER_H__

#include "NVTMedia.h"
#include "LCDConf.h"

#define LCD_PANEL_WIDTH     LCD_XSIZE
#define LCD_PANEL_HEIGHT    LCD_YSIZE

int
Render_Init(
    uint32_t u32AudioSampleRate,
    uint32_t u32AudioChannel
);

void
Render_VideoFlush(
    S_NM_VIDEO_CTX  *psVideoCtx
);

void
Render_AudioFlush(
    S_NM_AUDIO_CTX  *psAudioCtx
);

void
Render_Final(void);

#endif
