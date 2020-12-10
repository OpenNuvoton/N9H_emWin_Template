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

#include "N9H26TouchPanel.h"

extern int ts_writefile(int hFile);
extern int ts_readfile(int hFile);
extern int ts_calibrate(int xsize, int ysize);
extern void TouchTask(void);

#if GUI_SUPPORT_TOUCH

static int TouchScreen_Calibrate(void)
{
    char szFileName[32];
    char szCalibrationFile[64];
    int hFile;

    // Initialize
    Init_TouchPanel();

    sprintf(szFileName, "C:\\ts_calib");
    fsAsciiToUnicode(szFileName, szCalibrationFile, TRUE);

#if 0
    // Calibration function test.
    hFile = fsDeleteFile(szCalibrationFile, NULL);
    fsFlushIOCache();
    printf("Deleted %s. %x\n", szFileName, hFile);
#endif

    hFile = fsOpenFile(szCalibrationFile, szFileName, O_RDONLY | O_FSEEK);
    printf("file = %d\n", hFile);
    if (hFile < 0)
    {
        // file does not exists, so do calibration
        hFile = fsOpenFile(szCalibrationFile, szFileName, O_CREATE | O_RDWR | O_FSEEK);
        if (hFile < 0)
        {
            printf("CANNOT create the calibration file\n");
            goto exit_do_calibrate;
        }
        fsFlushIOCache();
        GUI_Init();
        ts_calibrate(LCD_XSIZE, LCD_YSIZE);
        ts_writefile(hFile);
        fsFlushIOCache();
    }
    else
        ts_readfile(hFile);

    fsCloseFile(hFile);

    return 0;

exit_do_calibrate:
    return -1;
}

extern void *worker_guiman(void *pvArgs);
void *worker_touching(void *pvArgs)
{
    int ret = TouchScreen_Calibrate();
    pthread_t pxID_worker_guiman;
    pthread_attr_t  gui_attr;

    if (ret < 0)
        goto exit_worker_touching;

    pthread_attr_init(&gui_attr);
    pthread_attr_setstacksize(&gui_attr, 32 * 1024);

    pthread_create(&pxID_worker_guiman, &gui_attr, worker_guiman, NULL);

    while (1)
    {
//        usleep(1000);   //1ms
        TouchTask();
    }

exit_worker_touching:
    return NULL;
}
#endif
