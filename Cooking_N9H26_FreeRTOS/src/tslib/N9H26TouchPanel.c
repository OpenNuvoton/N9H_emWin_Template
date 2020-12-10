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

#include "GUI.h"
#include "N9H26.h"
#include "N9H26TouchPanel.h"

extern int ts_phy2log(int *sumx, int *sumy);
int Init_TouchPanel(void)
{
    DrvADC_Open();
    return 1;
}


//return 0: faiure, 1: success.
int Read_TouchPanel(int *x, int *y)
{
    unsigned short adc_x, adc_y;
    if (IsPenDown() == TRUE)
    {
        if (adc_read(0, (unsigned short *)&adc_x, (unsigned short *)&adc_y) == 1)
        {
            *x = adc_x;
            *y = adc_y;
            return TRUE;
        }
    }
    return FALSE;
}

int Uninit_TouchPanel(void)
{
    DrvADC_Close();
    return 1;
}

void TouchTask(void)
{
    static U16 xOld;
    static U16 yOld;
    static U8  PressedOld = 0;
    int x, y, xDiff, yDiff;
    int32_t g_i32X, g_i32Y;

    int  Pressed = Read_TouchPanel(&g_i32X, &g_i32Y);
    /* Touch screen is pressed */
    if (Pressed)
    {
        x =  g_i32X;// TBD: Insert function which reads current x value
        y =  g_i32Y;// TBD: Insert function which reads current y value
        ts_phy2log(&x, &y);
        //
        // The touch has already been pressed
        //
        if (PressedOld == 1)
        {
            //
            // Calculate difference between new and old position
            //
            xDiff = (x > xOld) ? (x - xOld) : (xOld - x);
            yDiff = (y > yOld) ? (y - yOld) : (yOld - y);
            //
            // Store state if new position differs significantly from old position
            //
            if (xDiff + yDiff > 2)
            {
                xOld = x;
                yOld = y;
                GUI_TOUCH_StoreState(x, y);
                /*
                                    sysprintf("X=%d, Y=%d\n", x,y);
                                    State.x = x;
                                    State.y = y;
                                    State.Layer = 0;
                                    State.Pressed = 1;
                                    GUI_PID_StoreState(&State);
                                    */
            }
        }
        //
        // The touch was previously released
        // Store state regardless position
        //
        else
        {
            if ((x != 0) && (y != 0))
            {
//                  if ( g_u32Release > 1200 )
//                      return;
                xOld = x;
                yOld = y;
                PressedOld = 1;
                GUI_TOUCH_StoreState(x, y);
                /*
                                    sysprintf("X=%d, Y=%d\n", x,y);
                                    State.x = x;
                                    State.y = y;
                                    State.Layer = 0;
                                    State.Pressed = 1;
                                    GUI_PID_StoreState(&State);
                                    */
            }
        }
    }
    else
    {
        if (PressedOld == 1)
        {
            PressedOld = 0;
            GUI_TOUCH_StoreState(-1, -1);
            /*
                            sysprintf("X=-1, Y=-1\n");
                                State.x = 0;
                                State.y = 0;
                                State.Layer = 0;
                                State.Pressed = 0;
                                GUI_PID_StoreState(&State);
                             */
        }
    }
    //
    // Make sure
    //
}
