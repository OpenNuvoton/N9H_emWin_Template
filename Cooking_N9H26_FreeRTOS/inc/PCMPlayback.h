//      Copyright (c) 2017 Nuvoton Technology Corp. All rights reserved.
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

#ifndef __PCM_PLAYBACK__
#define __PCM_PLAYBACK__

#define PCM_TASK_STACK_MIDDLE   4096
#define PCM_TASK_PRIORITY_MIDDLE    4

int32_t
PCMPlayback_Start(
    uint32_t u32SampleRate,
    uint32_t u32Channel,
    int32_t i32Volume,
    uint32_t *pu32FragmentSize
);

void
PCMPlayback_Stop(void);

void
PCMPlayback_Send(
    uint8_t *pu8DataBuf,
    uint32_t *pu8SendLen
);

int32_t
PCMPlayback_GetBufFreeSpace(void);

typedef enum
{
    eSOUNDEFFECT_ID_DING_DONG,
    eSOUNDEFFECT_ID_ENTER_PAIRING_MODE,
    eSOUNDEFFECT_ID_NET_UNCONNECTED,
    eSOUNDEFFECT_ID_NET_CONNECTED,
    eSOUNDEFFECT_ID_FW_UPGRADING,
    eSOUNDEFFECT_ID_FW_DONE,
    eSOUNDEFFECT_ID_FW_FAIL,
    eSOUNDEFFECT_ID_CNT,
} E_SOUNDEFFECT_ID;

typedef enum
{
    eSOUNDEFFECT_LANG_EN,
    eSOUNDEFFECT_LANG_CN,
    eSOUNDEFFECT_LANG_CNT,
} E_SOUNDEFFECT_LANG;

int
SoundEffect_Playback(
    E_SOUNDEFFECT_LANG  eSoundEffectLang,
    E_SOUNDEFFECT_ID    eSoundEffectID
);

#endif
