/***************************************************************************
 *                                                                                                               *
 * Copyright (c) 2008 Nuvoton Technolog. All rights reserved.                                    *
 *                                                                                                               *
 ***************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "wblib.h"
#include "N9H20.h"


unsigned char RX_buf[256];

//extern UARTDEV_T* pUartDevISR;

void RS485_RX_Data(UARTDEV_T* pUartDev,int length);

void RS485_TX_Data(UARTDEV_T* pUartDev, int length, UINT8 *pData)
{
    int i;

#ifdef NUV_BOARD
    gpio_setportval(GPIO_PORTD, 0x10, 0x10);
#else
    gpio_setportval(GPIO_PORTD, 0x8, 0x8);
#endif

    // Write RS485 Data
//    outp32(REG_UART_LCR, 0x37);
    for(i=1; i<=length; i++)
    {
        while ((inpw(REG_UART_FSR) & 0x800000));
        pUartDev->UartPutChar(*pData);
        pData++;
    }
    // Wait last stop bit send
    while (!(inpw(REG_UART_FSR) & 0x10000000));

}
#if 0
int RS485_UART_TX(UARTDEV_T* pUartDev, int length, char cmd)
{
    int i;
    unsigned char buf[12];

    for(i=0; i<12; i++)
        buf[i]=0;
//  buf[0]= RS485_ADDR; // 0xC0
    buf[0]= 0x01;
    buf[1]= 0x06;
    buf[2]= 0x00;
    buf[3]= 0x00;
    buf[4]= 0x00;
    buf[5]= 0x00;
    buf[6]= 0x89;
    buf[7]= 0xCA;
    buf[8] = 0x0;

    sysprintf("Send:\n");
    for (i=0; i<8; i++)
        sysprintf(" %x,",buf[i]);
    sysprintf("\n");

    RS485_TX_Data(pUartDev, 8, buf);

    return 0;

}
#endif
int RS485_RX(void)
{
    int status= 0;

#ifdef NUV_BOARD
    gpio_setportval(GPIO_PORTD, 0x10, 0x00);
#else
    gpio_setportval(GPIO_PORTD, 0x8, 0x00);
#endif

    // Set Uart as 8-None-2
//    outp32(REG_UART_LCR, 0x07);

    // Reset RX FIFO to clear RX pointer for next test.
    outp32(REG_UART_FCR, inp32(REG_UART_FCR) | 0x02);
    return status;
}
#if 0
void RS485_RX_Data(UARTDEV_T* pUartDev,int length)
{
    int i;

#ifdef NUV_BOARD
    gpio_setportval(GPIO_PORTD, 0x10, 0x00);
#else
    gpio_setportval(GPIO_PORTD, 0x8, 0x00);
#endif

    // Set Uart as 8-Odd-1
    outp32(REG_UART_LCR, 0x0F);

    // Reset RX FIFO to clear RX pointer for next test.
    outp32(REG_UART_FCR, inp32(REG_UART_FCR) | 0x02);
    // Read Uart port to receive RX data
    for(i=0; i<length; i++)
        RX_buf[i] = pUartDev->UartGetChar();
}
#endif
#if 0
void RS485_UART(UARTDEV_T* pUartDev)
{
    int i;

//  pUartDevISR = pUartDev;

    RS485_UART_TX(pUartDev, 8, 0); //256);
// Receive RS485 data
    for(i=0; i<8; i++)
        RX_buf[i]= 0;

    RS485_RX_Data(pUartDev,8);
    sysprintf("Receive:\n");
    for(i=0; i<8; i++)
        sysprintf("0x%x ",RX_buf[i]);
    sysprintf("\n");


}
#endif
