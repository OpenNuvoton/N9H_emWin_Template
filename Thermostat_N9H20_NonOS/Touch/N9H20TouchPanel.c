/***************************************************************************
 *                                                                                                               *
 * Copyright (c) Nuvoton Technolog. All rights reserved.                                    *
 *                                                                                                               *
 ***************************************************************************/

#include "GUI.h"
#include "N9H20.h"
#include "N9H20TouchPanel.h"
#include "FT6336.h"

INT32 g_u16X, g_u16Y;
volatile BOOL g_bXY;
#define RETRY   10
INT32 g_u16TouchX, g_u16TouchY;
volatile BOOL g_bTdStatus;

volatile unsigned char  TPInit_Flag = 0;

//extern int ts_phy2log(int *sumx, int *sumy);

static void PortB2_isr(void)
{
    //sysprintf("GPODB2 IQR...\n");
    gpio_cleartriggersrc(GPIO_PORTB);
    TPInit_Flag = 1;
//
//  g_u16TouchX = FT6336_read_touch1_x();
//  g_u16TouchY = FT6336_read_touch1_y();
//
//  g_bTdStatus = FT6336_read_td_status();

//  sysprintf("g_bXY = %d\n", g_bXY);
//  sysprintf("FT6336 touch2 X = 0x%x\n", g_u16TouchX);
//  sysprintf("FT6336 touch2 Y = 0x%x\n", g_u16TouchY);
    return;
}
void gpioisr_init(void)
{
    outp32(REG_GPBFUN, inp32(REG_GPBFUN) & (~MF_GPB1));
    gpio_setportval(GPIO_PORTB, 1 << 1, 1 << 1);    //GPIOB 1 set high default
    gpio_setportpull(GPIO_PORTB, 1 << 1, 1 << 1);   //GPIOB 1 pull-up
    gpio_setportdir(GPIO_PORTB, 1 << 1, 1 << 1);    //GPIOB 1 output mode
    sysDelay(50);
    gpio_setportval(GPIO_PORTB, 1 << 1, 0 << 1);    //GPIOB 1 set low
    sysDelay(50);
    gpio_setportval(GPIO_PORTB, 1 << 1, 1 << 1);    //GPIOB 1 set high

    outp32(REG_GPBFUN, (inp32(REG_GPBFUN) & ~MF_GPB2));
    gpio_setportdir(GPIO_PORTB, (1 << 2), 0 << 2);
    gpio_setportpull(GPIO_PORTB, (1 << 2), 0 << 2);
    gpio_setintmode(GPIO_PORTB, (1 << 2), (1 << 2), 0 << 2);
    gpio_setsrcgrp(GPIO_PORTB, (1 << 2), 3);//
    gpio_setdebounce(128, 1 << 3);
    gpio_setlatchtrigger(1 << 3);
    sysInstallISR(IRQ_LEVEL_7, (INT_SOURCE_E)(3 + 2), (PVOID)PortB2_isr);
    sysSetInterruptType((INT_SOURCE_E)(3 + 2), HIGH_LEVEL_SENSITIVE);
    sysSetLocalInterrupt(ENABLE_IRQ);
    sysEnableInterrupt((INT_SOURCE_E)(3 + 2));
}
/*
UINT8 FT6336_read_firmware_id(void)
{
    UINT8 u8ID = 0;
    int j = RETRY;
    i2cIoctl(I2C_IOC_SET_SUB_ADDRESS, FT6336_ADDR_FIRMARE_ID, 1);
    j = RETRY;

    while(j-- > 0)
    {
        if(i2cRead(&u8ID, 1) == 1)
        break;
    }
    if(j <= 0)
    {
        sysprintf("Read ERROR [%x]!\n", FT6336_ADDR_FIRMARE_ID);
        return 0xFF;
    }
    return u8ID;
}

UINT8 FT6336_read_device_mode(void)
{
    UINT8 u8ID = 0;
    int j = RETRY;
    i2cIoctl(I2C_IOC_SET_SUB_ADDRESS, FT6336_ADDR_DEVICE_MODE, 1);

    while(j-- > 0)
    {
        if(i2cRead(&u8ID, 1) == 1)
        break;
    }
    if(j <= 0)
    {
        sysprintf("Read ERROR [%x]!\n", FT6336_ADDR_DEVICE_MODE);
        return 0xFF;
    }
    return u8ID;
}

UINT8 FT6336_write_device_mode(void)
{
    UINT8 u8ID = 0x00;
    int j = RETRY;
    i2cIoctl(I2C_IOC_SET_SUB_ADDRESS, FT6336_ADDR_DEVICE_MODE, 1);

    while(j-- > 0)
    {
        if(i2cWrite(&u8ID, 1) == 1)
        break;
    }
    if(j <= 0)
    {
        sysprintf("i2cWrite ERROR [%x]!\n", FT6336_ADDR_DEVICE_MODE);
        return 0xFF;
    }
    return u8ID;
}
*/
UINT8 FT6336_read_td_status(void)
{
    UINT8 u8ID = 0;
    int j = RETRY;
    i2cIoctl(I2C_IOC_SET_SUB_ADDRESS, FT6336_ADDR_TD_STATUS, 1);

    while(j-- > 0)
    {
        if(i2cRead(&u8ID, 1) == 1)
            break;
    }
    if(j <= 0)
    {
        sysprintf("Read ERROR [%x]!\n", FT6336_ADDR_TD_STATUS);
        return 0xFF;
    }
    return u8ID;
}

UINT8 FT6336_read_touch1_event(void)
{

    UINT8 u8ID = 0;
    int j = RETRY;
    i2cIoctl(I2C_IOC_SET_SUB_ADDRESS, FT6336_ADDR_TOUCH1_EVENT, 1);

    while(j-- > 0)
    {
        if(i2cRead(&u8ID, 1) == 1)
            break;
    }
    if(j <= 0)
    {
        sysprintf("Read ERROR [%x]!\n", FT6336_ADDR_TOUCH1_EVENT);
        return 0xFF;
    }
    return (u8ID >> 4);
}

UINT8 FT6336_read_touch1_id(void)
{

    UINT8 u8ID = 0;
    int j = RETRY;
    i2cIoctl(I2C_IOC_SET_SUB_ADDRESS, FT6336_ADDR_TOUCH1_ID, 1);

    while(j-- > 0)
    {
        if(i2cRead(&u8ID, 1) == 1)
            break;
    }
    if(j <= 0)
    {
        sysprintf("Read ERROR [%x]!\n", FT6336_ADDR_TOUCH1_ID);
        return 0xFF;
    }
    return (u8ID >> 4);
}

UINT16 FT6336_read_touch1_x(void)
{
    UINT8 u8ID[2];
    int j = RETRY;
    i2cIoctl(I2C_IOC_SET_SUB_ADDRESS, FT6336_ADDR_TOUCH1_X, 1);

    while(j-- > 0)
    {
        if(i2cRead(u8ID, 2) == 2)
            break;
    }
    if(j <= 0)
    {
        sysprintf("Read ERROR [%x]!\n", FT6336_ADDR_TOUCH1_X);
        return 0xFFFF;
    }
    return ((u8ID[0] & 0x0f) << 8) | u8ID[1];
}

UINT16 FT6336_read_touch1_y(void)
{
    UINT8 u8ID[2];
    int j = RETRY;
    i2cIoctl(I2C_IOC_SET_SUB_ADDRESS, FT6336_ADDR_TOUCH1_Y, 1);

    while(j-- > 0)
    {
        if(i2cRead(u8ID, 2) == 2)
            break;
    }
    if(j <= 0)
    {
        sysprintf("Read ERROR [%x]!\n", FT6336_ADDR_TOUCH1_Y);
        return 0xFFFF;
    }
    return ((u8ID[0] & 0x0f) << 8) | u8ID[1];
}

int Init_TouchPanel(void)
{
//    adc_init();
//    adc_open(ADC_TS_4WIRE, XSIZE_PHYS, YSIZE_PHYS);  //320x240
//    return 1;
    INT32 rtval;

    gpioisr_init();
    i2cInit();
    rtval = i2cOpen();
    if(rtval < 0)
    {
        sysprintf("Open I2C error!\n");
    }
    i2cIoctl(I2C_IOC_SET_DEV_ADDRESS, I2C_ADDR_FT6336, 0);
    i2cIoctl(I2C_IOC_SET_SPEED, 100, 0);
    return 1;
}

int Read_TouchPanel(int *x, int *y)
{
//return 0 fai;ure, 1 success.
    //unsigned short adc_x, adc_y;

    if(TPInit_Flag || g_bTdStatus)
        //if ( adc_read(ADC_NONBLOCK, (unsigned short *)&adc_x, (unsigned short *)&adc_y) == 1)
    {
        TPInit_Flag = 0;

        g_u16TouchX = FT6336_read_touch1_x();
        g_u16TouchY = FT6336_read_touch1_y();
        g_bTdStatus = FT6336_read_td_status();
        *x = g_u16TouchX;
        *y = g_u16TouchY;
        //sysprintf("x = %d, y = %d\n", *x, *y);
        g_bXY = g_bTdStatus;
        return 1;
    }
    g_bXY = FALSE;
    return 0;
}

int Uninit_TouchPanel(void)
{
    //adc_close();
    i2cClose();
    i2cExit();
    return 1;
}

int Check_TouchPanel(void)
{
//    if ( (inp32(REG_ADC_TSC) & ADC_UD) == ADC_UD)
//        return 1;   //Pen down;
//    else
//        return 0;   //Pen up;
    return g_bTdStatus;
}

void TouchTask(void)
{
    static U16 xOld;
    static U16 yOld;
    static U8  PressedOld = 0;
    int x, y, xDiff, yDiff;
    BOOL  Pressed;

    Read_TouchPanel(&g_u16X, &g_u16Y);
    Pressed = g_bXY;// TBD: Insert function which returns:
    //      1, if the touch screen is pressed
    //      0, if the touch screen is released
    //
    // Touch screen is pressed
    //
    if (Pressed)
    {
        x =  g_u16X;// TBD: Insert function which reads current x value
        y =  g_u16Y;// TBD: Insert function which reads current y value
//        ts_phy2log(&x, &y);
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

            //sysprintf("33333333333333 xDiff=%d, yDiff=%d\n", xDiff,yDiff);
            if (xDiff + yDiff > 2)
            {
                xOld = x;
                yOld = y;
                GUI_TOUCH_StoreState(x, y);
                //sysprintf("1111111111 X=%d, Y=%d\n", x,y);
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
                //sysprintf("222222222222 X=%d, Y=%d\n", x,y);
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
