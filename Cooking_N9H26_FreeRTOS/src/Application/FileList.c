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
#include <string.h>

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

#include "NVTMedia_SAL_FS.h"
#include "FileList.h"
#include "re.h"

// Define filter pattern in here.
const char *szFileFilter[] =
{
    ".[aA][vV][iI]",
    ".[mM][pP]4"
};
const int g_i32MaxFilteringItemNum  = sizeof(szFileFilter) / sizeof(const char *) ;

static S_FILELIST g_sFileList = {0};

// Get absolute path of file.
static char *GetAbsPath(const char *str1, const char *str2)
{
    char *res;
    size_t strlen1 = strlen(str1);
    size_t strlen2 = strlen(str2);
    int i, j;
    res = malloc((strlen1 + strlen2 + 1));
    strcpy(res, str1);
    for (i = strlen1, j = 0; ((i < (strlen1 + strlen2)) && (j < strlen2)); i++, j++)
        res[i] = str2[j];
    res[strlen1 + strlen2] = '\0';
    return res;
}

static void filelist_init(S_FILELIST *psFileList)
{
    if (!psFileList)
        memset((void *)psFileList, 0, sizeof(S_FILELIST));
    psFileList->m_i32MaxSize = DEF_MAX_FILELIST_SIZE;
}

int filelist_dump(void)
{
    int i = 0;
    S_FILELIST *psFileList = &g_sFileList;
    printf("**********************************************************\n");
    printf("%s, used: %d max: %d\n", __func__, psFileList->m_i32UsedNum, psFileList->m_i32MaxSize);
    for (i = 0; i < psFileList->m_i32UsedNum; i++)
        printf("[%s] %d -> %s\n", __func__, i, psFileList->m_szFilePath[i]);
    printf("**********************************************************\n");
    return psFileList->m_i32MaxSize;
}

static int filelist_filter_and_append(S_FILELIST *psFileList, const char *pcDirPath, const char *pcFileName)
{
    int i;
    re_t pattern;
    char *fullpath;
    char *pcAbsPath;

    for (i = 0; i < g_i32MaxFilteringItemNum; i++)
    {
        int match_idx;
        pattern = re_compile(szFileFilter[i]);
        match_idx = re_matchp(pattern, pcFileName);
        if (match_idx != -1)
        {
            if (psFileList->m_i32UsedNum < psFileList->m_i32MaxSize)
            {
                fullpath = GetAbsPath(pcDirPath, pcFileName);
                psFileList->m_szFilePath[psFileList->m_i32UsedNum] = fullpath ;
                psFileList->m_i32UsedNum++;
            } //if ( psFileList->m_i32UsedNum < psFileList->m_i32MaxSize )
        } //if (match_idx != -1)
    } // for (i=0; i<g_i32MaxFilteringItemNum; i++)

    return 0;
}

int filelist_create(const char *pcDirPath)
{
    int n = 0;
    DIR *psDIR = NULL;
    struct dirent *dp;

    if (!pcDirPath)
        goto exit_filelist_Create;

    filelist_init(&g_sFileList);

    psDIR = opendir(pcDirPath);
    if (!psDIR)
        goto exit_filelist_Create;

    while (dp = readdir(psDIR)) // if dp is null, there's no more content to read
    {
        printf("[%s] %s\\%s\n", __func__, pcDirPath, dp->d_name);
        filelist_filter_and_append(&g_sFileList, pcDirPath, dp->d_name);
        n++;
    }
    closedir(psDIR); // close the handle (pointer)

    return n;

exit_filelist_Create:

    return -1;
}

int filelist_destroy(void)
{
    int i = 0;
    S_FILELIST *psFileList = &g_sFileList;
    for (i = 0; i < psFileList->m_i32UsedNum; i++)
        free(psFileList->m_szFilePath[i]);
    filelist_init(psFileList);
    return 0;
}

S_FILELIST *filelist_getInstance(void)
{
    return &g_sFileList;
}

const char *filelist_getFileName(int idx)
{
    return (const char *) filelist_getInstance()->m_szFilePath[ idx ];
}