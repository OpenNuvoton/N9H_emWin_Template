/*
 * Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <errno.h>
//#include <termios.h>
#include "modbus.h"


#define ADDR_READSENSOR				0x2000
#define ADDR_SENSORDEVICE			0x2002
#define ADDR_LED_1				0x2003

// definietion
// 12 bytes
// Addr No Cmd 0x5C Value_Byte0 Value_Byte1 Value_Byte2 Value_Byte3 
#define RS485_ADDR  		0xC0

#define	RS485_NO		0x01    //2

#define RS485_SEND		0x5C

#define RS485_FEEDBACKOK	 0xA9
#define RS485_CHECKSTATUS	0xA6
#define RS485_UNKNOWN		0xB6

#define RS485_OPEN		0x01
#define RS485_READ		0x02
#define RS485_WRITE		0x03
#define RS485_CLOSE		0x04

#define RS485_CONNECTED		0x05
#define RS485_SETADDRESS	0x06
#define RS485_ONLINE		0x07
#define RS485_RESET		0x08


#define RS485_LEDON		0x10
#define RS485_LEDOFF		0x11

#define RS485_POWEROFF		-1
#define RS485_BREAKLINE		-2
#define RS485_OK		0
#define RS485_INIT_OK		1
#define RS485_SENSOR_OK		2
#define RS485_TRYAGAIN		3		
#define RS485_CONNECTION_OK	4
#define RS485_SETADDRESS_OK	5

#define CONNECTION		1
#define DISCONNECT		3
#define SENSOR_NOTREADY		2
#define SETADDRESS		4

/* The goal of this program is to check all major functions of
   libmodbus:
   - write_coil
   - read_bits
   - write_coils
   - write_register
   - read_registers
   - write_registers
   - read_registers

   All these functions are called with random values on a address
   range defined by the following defines.
*/
#define LOOP             1
#define SERVER_ID        1
#define SERVER_ID_2      2
#define ADDRESS_START    0
#define ADDRESS_END     99

/* At each loop, the program works in the range ADDRESS_START to
 * ADDRESS_END then ADDRESS_START + 1 to ADDRESS_END and so on.
 */
//struct termios old, new;
//void initTermios(int echo);
//void resetTermios(void);
//char getch_(int echo);
//char getonech(void);

int c_div, c_rem, f_div, f_rem;

char g_au8PzemVoltage[256];
char g_au8PzemCurrent[256];
char g_au8PzemPower[256];
char g_au8PzemEnergy[256];

void 	ModbusDisplayMessage(uint16_t *u8ModbusADU, uint8_t cmd, int number)
{
  uint8_t count, i;
  uint8_t *pu8Cur;
  uint16_t hibyte, lobyte,value, div,rem, hivalue, address;
  uint32_t u32val;
  uint16_t *pu16Cur;
  uint16_t c_temp, f_temp;//, c_div, c_rem, f_div, f_rem;
  pu8Cur = (uint8_t *)&u8ModbusADU[0];

  pu16Cur = (uint16_t *)&u8ModbusADU[0];
  if ( number < 0 )
  {
  if ( *pu8Cur != SERVER_ID )
  {
    if ( *pu8Cur == 0xF8 )
    {
      pu8Cur++;
      if ( *pu8Cur == 0x41 )
      {
	printf("Calibration OK\n");
      }
      else if ( *pu8Cur == 0xC1 )
      {
	printf("Calibration Error\n");
      }
      else
      {
	printf("Error feedback\n");				
      }
    }
    else
    {
      printf("Error feedback\n");
    }
    printf("\n\n");
    return;
  }
  }
  switch ( cmd )
  {
    case 0x03:
      count = 0;
      while ( number > 0 )
      {
	value = *pu16Cur++ ;
	if ( count == 0 )
	{
	  div = value/100;
	  rem = value % 100;
	  printf("High Voltage alarm= %u.%02uV\n",div, rem);
	}
	else if ( count == 1 )
	{
	  div = value/100;
	  rem = value % 100;
	  printf("Low Voltage alarm threshold = %u.%02uV\n",div, rem);
	}
	else if ( count == 2 )
	{
	  printf("Modbus-RTU address = %x\n", value);
	}
	else if ( count == 3 )
	{
	  printf("Register 4 = %u\n",value);
	}
	count++;
	number -= 1;
      }
      break;
    case 0x04:
      count = 0;
      for (i=0; i<2; i++)
      {
	value = *pu16Cur++;
	if ( count == 0 )
	{
	  div = value/100;
	  rem = value % 100;
	  printf("Voltage = %u.%02uV\n",div, rem);
      sprintf(g_au8PzemVoltage, "Voltage: %u.%02uV\n",div, rem);
	  count++;
	}
	else if ( count == 1 )
	{
	  div = value/100;
	  rem = value % 100;
	  printf("Current = %u.%02uA\n",div, rem);
      sprintf(g_au8PzemCurrent, "Current: %u.%02uA\n",div, rem);
	  count++;
	}
      }
      number -= 2;
      for (i=0; i<2; i++)
      {
	value = *pu16Cur++;
	hivalue = *pu16Cur++;
	u32val = hivalue;
	u32val <<=16;
	u32val |= value;
	if ( count == 2 )
	{
	  div = u32val/10;
	  rem = u32val % 10;
	  printf("Power = %u.%01uW\n",div, rem);
      sprintf(g_au8PzemPower, "Power: %u.%01uW\n",div, rem);
	  count++;
	}
	else if ( count == 3 )
	{
	  printf("Energy = %uW\n",u32val);
      sprintf(g_au8PzemEnergy, "Energy: %uW\n",u32val);
	  count++;
	}
      }
      number -= 4;
      for (i=0; i<number; i++)
      {
	value = *pu16Cur++;
	if ( count == 4 )
	{
	  //printf("High voltage alarm status = %04x\n", value);
	  count++;
	}
	else if ( count == 5 )
	{
	  //printf("Low voltage alarm status = %04x\n", value);
	  count++;
	}
      }
      break;
    case 0x06:
      address = *pu16Cur++;
      value = *pu16Cur++;
      if ( address == 0 )
      {
	div = value/100;
	rem = value % 100;
	printf("Set High Voltage alarm OK = %u.%02uV\n",div, rem);
      }
      else if ( address == 1 )
      {
	div = value/100;
	rem = value % 100;
	printf("Set Low Voltage alarm threshold OK= %u.%02uV\n",div, rem);
      }
      else if ( address == 2 )
      {
	printf("Set Modbus-RTU address OK= %x\n", value);
      }
      else if ( address == 3 )
      {
	printf("Set Register 4 = %u\n",value);
      }
      break;
    case 0x42:
      printf("Reset energy OK\n");	
      break;

   case 0x84:// read thermal sensor
      c_temp = *pu16Cur++;
      f_temp = *pu16Cur++;
      c_div = c_temp/100;
      c_rem = abs(c_temp) % 100;
      f_div = f_temp/100;
      f_rem = abs(f_temp) % 100;
   printf("Temperature is %d.%02d Celsius, %d.%02d Fahrenheit\r\n", c_div,c_rem, f_div, f_rem);	
      break;
 }
 printf("\n\n");
}
#if 0
/* Initialize new terminal i/o settings */
void initTermios(int echo) 
{
 tcgetattr(0, &old);/* grab old terminal i/o settings */
 new = old;/* make new settings same as old settings */
 new.c_lflag &= ~ICANON;/* disable buffered i/o */
 new.c_lflag &= echo? ECHO : ~ECHO;/* set echo mode */
 tcsetattr(0, TCSANOW, &new);/* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) 
{
 tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo) 
{
 char ch;
 initTermios(echo);
 ch = getchar();
 resetTermios();
 return ch;
}

/* Read 1 character without echo */
char getonech(void) 
{
 return getch_(0);
}
#endif

int g_libmodbus_flag;

int main_modbus(char select)
{
    modbus_t *ctx;
    int rc;
    int nb_fail;
 //   int nb_loop;
    int addr;
    int nb;
    uint8_t *tab_rq_bits;
    uint8_t *tab_rp_bits;
    uint16_t *tab_rq_registers;
    uint16_t *tab_rw_rq_registers;
    uint16_t *tab_rp_registers;
    int u32Item;
    int i; 
   short  hi_voltage,lo_voltage;
   uint16_t div, rem;
  
    /* RTU */

    ctx = modbus_new_rtu("/dev/ttyS6", 9600, 'N', 8, 2);

    modbus_set_slave(ctx, SERVER_ID);
    modbus_set_debug(ctx, FALSE);

    if (modbus_connect(ctx) == -1) {
        printf("Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
    modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
//    modbus_rtu_set_rts(ctx,MODBUS_RTU_RTS_UP);
//    modbus_rtu_set_rts_delay(ctx, 0x80);
    /* Allocate and initialize the different memory spaces */
    nb = ADDRESS_END - ADDRESS_START;

    tab_rq_bits = (uint8_t *) malloc(nb * sizeof(uint8_t));
    memset(tab_rq_bits, 0, nb * sizeof(uint8_t));

    tab_rp_bits = (uint8_t *) malloc(nb * sizeof(uint8_t));
    memset(tab_rp_bits, 0, nb * sizeof(uint8_t));

    tab_rq_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
    memset(tab_rq_registers, 0, nb * sizeof(uint16_t));

    tab_rp_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
    memset(tab_rp_registers, 0, nb * sizeof(uint16_t));

    tab_rw_rq_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
    memset(tab_rw_rq_registers, 0, nb * sizeof(uint16_t));

   //while (1)
   {
     //printf(" [1] Open sensor 1\n");
     //printf(" [2] Read sensor 1\n");	
     //printf(" [3] LED ON 1\n");	
     //printf(" [4] LED OFF 1\n");
     //printf(" [5] Open sensor 2\n");
     //printf(" [6] Read sensor 2\n");	
     //printf(" [7] LED ON 2\n");	
     //printf(" [8] LED OFF 2\n");	
     //printf(" [9] exit \n");
     //printf(" [A] Read the measurement\n");
 //   u32Item = getchar();
    //u32Item = getonech();
    u32Item = select;
	
    if (u32Item == '1')
    {
        modbus_set_slave(ctx, SERVER_ID);
	rc = modbus_write_bit(ctx, ADDR_SENSORDEVICE, 1);
        if ( rc >= 0 )
        {
           printf("Open Sensor 1 OK. result=%d\n\n", rc);
        }
       else {  
             printf("Open Sensor 1 Fail ! result= %d\n\n", rc);
             nb_fail++;
        }
 
    }
    else if (u32Item == '2')
    {
        modbus_set_slave(ctx, SERVER_ID);
        addr = ADDR_READSENSOR;
        nb = 2;
        rc = modbus_read_input_registers(ctx, addr, nb, tab_rp_registers);
        if ( rc != -1 )
        {
       	   ModbusDisplayMessage(tab_rp_registers, 0x84, rc); 
        }
       else 
       {  
           printf("Read sensor 1 fails, result=%d\n\n", rc);
        }
    }
    else if ( u32Item == '3')
    {
        modbus_set_slave(ctx, SERVER_ID);
	rc = modbus_write_bit(ctx, ADDR_LED_1, 1);
        if ( rc >= 0 )
        {
           printf("LED ON 1 OK. result=%d\n\n", rc);
        }
       else {  
             printf("LED ON 1 Fail ! result= %d\n\n", rc);
             nb_fail++;
        }
            
    }
    else if ( u32Item == '4')
    {
        modbus_set_slave(ctx, SERVER_ID);
	rc = modbus_write_bit(ctx, ADDR_LED_1, 0);
        if ( rc >= 0 )
        {
           printf("LED OFF 1 OK. result=%d\n\n", rc);
        }
       else {  
             printf("LED OFF 1 Fail ! result= %d\n\n", rc);
             nb_fail++;
        }
    }
    else if (u32Item == '5')
    {
        modbus_set_slave(ctx, SERVER_ID_2);
	rc = modbus_write_bit(ctx, ADDR_SENSORDEVICE, 1);
        if ( rc >= 0 )
        {
           //printf("Open Sensor 2 OK. result=%d\n\n", rc);
           ;
        }
       else {  
             printf("Open Sensor 2 Fail ! result= %d\n\n", rc);
             nb_fail++;
        }
 
    }
    else if (u32Item == '6')
    {
        modbus_set_slave(ctx, SERVER_ID_2);
        addr = ADDR_READSENSOR;
        nb = 2;
        rc = modbus_read_input_registers(ctx, addr, nb, tab_rp_registers);
        if ( rc != -1 )
        {
       	   ModbusDisplayMessage(tab_rp_registers, 0x84, rc); 
        }
       else 
       {  
           printf("Read sensor 2 fails, result=%d\n\n", rc);
        }
    }
    else if ( u32Item == '7')
    {
        modbus_set_slave(ctx, SERVER_ID_2);
	rc = modbus_write_bit(ctx, ADDR_LED_1, 1);
        if ( rc >= 0 )
        {
           printf("LED ON 2 OK. result=%d\n\n", rc);
        }
       else {  
             printf("LED ON 2 Fail ! result= %d\n\n", rc);
             nb_fail++;
        }
            
    }
    else if ( u32Item == '8')
    {
        modbus_set_slave(ctx, SERVER_ID_2);
	rc = modbus_write_bit(ctx, ADDR_LED_1, 0);
        if ( rc >= 0 )
        {
           printf("LED OFF 2 OK. result=%d\n\n", rc);
        }
       else {  
             printf("LED OFF 2 Fail ! result= %d\n\n", rc);
             nb_fail++;
        }
    }
    else if ( u32Item == '9')
    {
       //break;
    }
    else if (u32Item == 'A')
    {
        modbus_set_slave(ctx, SERVER_ID);
//      readInputRegisters(0x0, 8);
        addr = 0;
        nb = 8;
        rc = modbus_read_input_registers(ctx, addr, nb, tab_rp_registers);
#if 0
        if (rc != 1) {
                    printf("ERROR modbus_read_registers single (%d)\n", rc);
                    printf("Address = %d\n", addr);
                    nb_fail++;
        } else {
         for (i=0; i<16; i++)
         {
           printf(" %x ", tab_rp_registers[i] );
         }
         printf( "\n"); 
       }
#endif

       if ( rc != -1 )
       {
       	 ModbusDisplayMessage(tab_rp_registers, 0x4, rc); 
       }
       else {  
             printf("ERROR modbus_read_registers single (%d)\n", rc);
             printf("Address = %d\n", addr);
             nb_fail++;
        }
            
       
 
    }
    else
    {
      printf("error input\n");

    }     
  }

    /* Free the memory */
    free(tab_rq_bits);
    free(tab_rp_bits);
    free(tab_rq_registers);
    free(tab_rp_registers);
    free(tab_rw_rq_registers);

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
