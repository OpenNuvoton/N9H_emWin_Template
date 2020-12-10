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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "Render.h"

#include "PCMPlayback.h"
#include "N9H26_VPOST.h"

static uint8_t *s_pu8CurFBAddr;
extern UINT8 *pu8VideoBufPtr;

void VPOST_InterruptServicerRoutine()
{
    vpostSetFrameBuffer((uint32_t)s_pu8CurFBAddr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t s_u32PCMPlaybackBufSize;
uint8_t *s_pu8PCMPlaybackBuf;
uint32_t s_u32DataLenInPCMBuf;

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void
Render_VideoFlush(
    S_NM_VIDEO_CTX  *psVideoCtx
)
{
    //uint64_t u64CurTime;
    //u64CurTime = NMUtil_GetTimeMilliSec();
    //printf("Render_VideoFlush %d, %d\n", (uint32_t)u64CurTime, (uint32_t)psVideoCtx->u64DataTime);

    s_pu8CurFBAddr = psVideoCtx->pu8DataBuf;
}

void
Render_AudioFlush(
    S_NM_AUDIO_CTX  *psAudioCtx
)
{
    uint64_t u64CurTime;
    u64CurTime = NMUtil_GetTimeMilliSec();
    //printf("Render_AudioFlush %d, %d %dbytes\n", (uint32_t)u64CurTime, (uint32_t)psAudioCtx->u64DataTime, psAudioCtx->u32DataSize);

    if (psAudioCtx->u32DataSize <= s_u32PCMPlaybackBufSize - s_u32DataLenInPCMBuf)
    {
        memcpy(s_pu8PCMPlaybackBuf + s_u32DataLenInPCMBuf, psAudioCtx->pu8DataBuf, psAudioCtx->u32DataSize);
        s_u32DataLenInPCMBuf += psAudioCtx->u32DataSize;
    }
    else
    {
        NMLOG_INFO("No buffer space for PCM data psG711DecFrame->u32FrameSize %d, u32DataLenInPCMBuf %d\n", psAudioCtx->u32DataSize, s_u32DataLenInPCMBuf);
    }

    if (s_u32DataLenInPCMBuf)
    {
        PCMPlayback_Send(s_pu8PCMPlaybackBuf, &s_u32DataLenInPCMBuf);
    }

}

int
Render_Init(
    uint32_t u32AudioSampleRate,
    uint32_t u32AudioChannel
)
{
    uint32_t u32SPUFragSize;
    PFN_DRVVPOST_INT_CALLBACK fun_ptr;
    int ret = 0;

    printf("[%s %d]\n", __func__, __LINE__);

    s_pu8CurFBAddr = pu8VideoBufPtr ;

    //Init VPOST
    vpostInstallCallBack(eDRVVPOST_VINT, (PFN_DRVVPOST_INT_CALLBACK)VPOST_InterruptServicerRoutine, (PFN_DRVVPOST_INT_CALLBACK *)&fun_ptr);
    vpostEnableInt(eDRVVPOST_VINT);
    sysEnableInterrupt(IRQ_VPOST);

    //Enable SPU
    if (PCMPlayback_Start(u32AudioSampleRate, u32AudioChannel, 80, &u32SPUFragSize) != 0)
    {
        printf("Unable start PCM playback \n");
        ret = -1;
        goto exit_Render_Init;
    }

    s_u32PCMPlaybackBufSize = u32AudioSampleRate * u32AudioChannel * 2; //1Sec buffer size
    s_pu8PCMPlaybackBuf = malloc(s_u32PCMPlaybackBufSize);
    s_u32DataLenInPCMBuf = 0;

    if (s_pu8PCMPlaybackBuf == NULL)
    {
        printf("Unable allocate playback buffer\n");
        PCMPlayback_Stop();
        ret = -2;
        goto exit_Render_Init;
    }

    printf("[%s %d]\n", __func__, __LINE__);

exit_Render_Init:
    printf("[%s %d]\n", __func__, __LINE__);

    return ret;
}

void
Render_Final(void)
{
    printf("[%s %d]\n", __func__, __LINE__);
    if (s_pu8PCMPlaybackBuf)
    {
        PCMPlayback_Stop();
        free(s_pu8PCMPlaybackBuf);
        s_pu8PCMPlaybackBuf = NULL;
        s_u32PCMPlaybackBufSize = 0;
    }

    printf("[%s %d]\n", __func__, __LINE__);

    sysDisableInterrupt(IRQ_VPOST);
    vpostDisableInt(eDRVVPOST_VINT);
    vpostSetFrameBuffer((uint32_t)pu8VideoBufPtr);

    printf("[%s %d]\n", __func__, __LINE__);
}