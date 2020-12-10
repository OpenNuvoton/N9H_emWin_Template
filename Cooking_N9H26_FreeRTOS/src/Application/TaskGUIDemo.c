/**************************************************************************//**
* @copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*   1. Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright notice,
*      this list of conditions and the following disclaimer in the documentation
*      and/or other materials provided with the distribution.
*   3. Neither the name of Nuvoton Technology Corp. nor the names of its contributors
*      may be used to endorse or promote products derived from this software
*      without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

/* Standard includes. */
#include <stdlib.h>

/* Scheduler includes. */
/* FreeRTOS+POSIX. */
#include "FreeRTOS_POSIX.h"
#include "FreeRTOS_POSIX/pthread.h"
#include "FreeRTOS_POSIX/mqueue.h"
#include "FreeRTOS_POSIX/time.h"
#include "FreeRTOS_POSIX/errno.h"
#include "FreeRTOS_POSIX/unistd.h"

/* Nuvoton includes. */
#include "wblib.h"
#include "N9H26.h"

#include "GUI.h"
#include "LCDConf.h"
#include "WM.h"
#include "TEXT.h"
#include "DIALOG.h"

#include "player.h"

#define DEF_IDLE_CNT    100
static WM_HWIN hWin_Playback = NULL;
static int i32IdleCnt = 0;

WM_HWIN CreatePlayback(void);
void update_gui_widgets(WM_HWIN, E_NM_PLAY_STATUS);

static void cbBackgroundWin(WM_MESSAGE *pMsg)
{
    printf("[%s] msgid=%d, %d->%d \n", __func__, pMsg->MsgId, pMsg->hWinSrc, pMsg->hWin);
    i32IdleCnt = 0;
    switch (pMsg->MsgId)
    {
    case WM_TOUCH:
        if (hWin_Playback)
            WM_ShowWin(hWin_Playback);


        break;
    case WM_PAINT:
        GUI_SetBkColor(DEF_OSD_COLORKEY);
        GUI_Clear();
        break;
    }
    WM_DefaultProc(pMsg);
}

static WM_HWIN hWin_MainMenu = NULL;
WM_HWIN CreateWindow_Cooking(void);

static WM_HWIN hWin_TempMenu = NULL;
WM_HWIN CreateWindow_Temp(void);

static WM_HWIN hWin_SettingMenu = NULL;
WM_HWIN CreateWindow_Setting(void);

extern U8 g_u8Flag1;

void *worker_guiman(void *pvArgs)
{
#ifdef DEF_WITHOUT_GUI

    // Create file list.
    filelist_create(DEF_PATH_MEDIA_FOLDER);

    // Dump media file name in file list.
    filelist_dump();

    player_start();
    player_play();

    while (1)
    {
        E_NM_PLAY_STATUS ePlayerStatusNow =  player_status();
        if (ePlayerStatusNow == eNM_PLAY_STATUS_EOM)
        {
            printf("##########################\n");
            player_next();
        }
        usleep(100000);
    }
#else
    WM_SetCreateFlags(WM_CF_MEMDEV);
    GUI_Init();
    hWin_MainMenu = CreateWindow_Cooking();
    GUI_Delay(3000);
    WM_DeleteWindow(hWin_MainMenu);

    // Hijacking callback of Desktop.
    WM_SetCallback(WM_HBKWIN, cbBackgroundWin);

    // Fill color key.
    GUI_SetBkColor(DEF_OSD_COLORKEY);
    GUI_Clear();

    printf("BKhandle:%d\n", WM_HBKWIN);

    hWin_Playback = CreatePlayback();
    if (!hWin_Playback) goto exit_worker_guiman;

    // Hijacking callback of hWin_Playback.
    //WM_SetCallback(hWin_Playback, cbBackgroundWin);

    while (1)
    {
        E_NM_PLAY_STATUS ePlayerStatusNow =  player_status();

        if (i32IdleCnt > DEF_IDLE_CNT)
            WM_HideWindow(hWin_Playback);
        else
        {
            // Update Play & Pause widget & progressbar & time label
            update_gui_widgets(hWin_Playback, ePlayerStatusNow);
        }

//        printf("ePlayerStatusNow=%d\n", ePlayerStatusNow);
        // If end of media file, play next.
        if ((ePlayerStatusNow == eNM_PLAY_STATUS_EOM) || (ePlayerStatusNow == eNM_PLAY_STATUS_ERROR))
        {
#if 1
            g_u8Flag1 = 0;
            hWin_MainMenu = CreateWindow_Cooking();
            while (g_u8Flag1 == 0)
                GUI_Delay(100);
            WM_DeleteWindow(hWin_MainMenu);
            printf("##########################\n");
            if (g_u8Flag1 == 7)
            {
                g_u8Flag1 = 0;
//                sysprintf("### temp b\n");
                hWin_TempMenu = CreateWindow_Temp();
                while (g_u8Flag1 == 0)
                    GUI_Delay(100);
                WM_DeleteWindow(hWin_TempMenu);
//                sysprintf("### temp e\n");
                continue;
            }
            else if (g_u8Flag1 == 8)
            {
                g_u8Flag1 = 0;
//                sysprintf("### set b\n");
                hWin_SettingMenu = CreateWindow_Setting();
                while (g_u8Flag1 == 0)
                    GUI_Delay(100);
                WM_DeleteWindow(hWin_SettingMenu);
//                sysprintf("### set e\n");
                continue;
            }
//            player_next();
            player_playselect(g_u8Flag1 - 1);
#else
            player_next();
#endif
        }
        i32IdleCnt++;
        GUI_Delay(100);
    }
#endif

exit_worker_guiman:
    return NULL;
}
