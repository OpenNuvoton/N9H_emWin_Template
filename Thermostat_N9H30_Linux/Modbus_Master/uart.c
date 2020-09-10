/*
 * Copyright (c) 2014 Nuvoton technology corporation
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include     <stdio.h>
#include     <stdlib.h>
#include     <unistd.h> 
#include     <stdint.h>
//#include     <sys/types.h>
#include     <sys/stat.h> 
#include     <fcntl.h> 
#include     <termios.h>  
#include     <errno.h>
#include     <string.h>
#include 	<signal.h>
#include    <pthread.h>
#include   <linux/serial.h>



#define FALSE 0
#define TRUE  1

/* Driver-specific ioctls: ...\linux-3.10.x\include\uapi\asm-generic\ioctls.h */
#define TIOCGRS485      0x542E
#define TIOCSRS485      0x542F

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
// 
#define IDLETIME		1000     // 1000 ms, 

#define RS485_NUMBER		1     // 1 devices
#define	RS485_CMD_LEN		12


// function from
#if 0
extern  void begin(uint8_t);
extern  uint8_t  readInputRegisters(uint16_t, uint8_t);
extern  uint8_t  writeSingleRegister(uint16_t, uint16_t);
extern  uint8_t readHoldingRegisters(uint16_t, uint16_t);
extern 	uint8_t calibrateRegister(uint16_t, uint8_t);
extern  uint8_t resetenergyRegister(uint16_t, uint8_t);
extern  uint8_t  writeSingleCoil(uint16_t, uint8_t);
extern  uint8_t readCoils(uint16_t, uint16_t);
#endif

void RS485_TX_Data(int length, uint8_t *pData);

int fd[2];

//pthread_t threads[2];


static struct termios newtios,oldtios; /*termianal settings */
static int saved_portfd=-1;            /*serial port fd */
int g_portfd;


volatile int g_Cmd;
volatile char bConnected[RS485_NUMBER+1];


/* Test GCC version, this structure is consistent in GCC 4.8, thus no need to overwrite */
#if (__GNUC__ == 4 && __GNUC_MINOR__ == 3)

struct my_serial_rs485
{
	unsigned long	flags;			/* RS485 feature flags */
#define SER_RS485_ENABLED		(1 << 0)	/* If enabled */
#define SER_RS485_RTS_ON_SEND		(1 << 1)	/* Logical level for
							   RTS pin when
							   sending */
#define SER_RS485_RTS_AFTER_SEND	(1 << 2)	/* Logical level for
							   RTS pin after sent*/
#define SER_RS485_RX_DURING_TX		(1 << 4)
	unsigned long	delay_rts_before_send;	/* Delay before send (milliseconds) */
	unsigned long	delay_rts_after_send;	/* Delay after send (milliseconds) */
	unsigned long	padding[5];		/* Memory is cheap, new structs
					   are a royal PITA .. */
};

#endif


static void reset_tty_atexit(void)
{
	if(saved_portfd != -1)
	{
		tcsetattr(saved_portfd,TCSANOW,&oldtios);
	} 
}

/*cheanup signal handler */
static void reset_tty_handler(int signal)
{
	if(saved_portfd != -1)
	{
		tcsetattr(saved_portfd,TCSANOW,&oldtios);
	}
	_exit(EXIT_FAILURE);
}

static int open_port(const char *portname)
{
	struct sigaction sa;
	int portfd;
#if (__GNUC__ == 4 && __GNUC_MINOR__ == 3)
	struct my_serial_rs485 rs485conf;
	struct my_serial_rs485 rs485conf_bak;
#else
	struct serial_rs485 rs485conf;
	struct serial_rs485 rs485conf_bak;
#endif	
	//printf("opening serial port:%s\n",portname);
	/*open serial port */
	if((portfd=open(portname,O_RDWR | O_NOCTTY, 0)) < 0 )
	{
   		printf("open serial port %s fail \n ",portname);
   		return portfd;
	}

	printf("opening serial port:%s\n",portname);

	/*get serial port parnms,save away */
	tcgetattr(portfd,&newtios);
	memcpy(&oldtios,&newtios,sizeof newtios);
	/* configure new values */
	cfmakeraw(&newtios); /*see man page */
	newtios.c_iflag |=IGNPAR; /*ignore parity on input */
	newtios.c_oflag &= ~(OPOST | ONLCR | OLCUC | OCRNL | ONOCR | ONLRET | OFILL); 
	newtios.c_cflag = CS8 | CLOCAL | CREAD | CSTOPB;
	newtios.c_cc[VMIN]=1; /* block until 1 char received */
	newtios.c_cc[VTIME]=0; /*no inter-character timer */
	/* 115200 bps */
	cfsetospeed(&newtios,B9600);
	cfsetispeed(&newtios,B9600);
	/* register cleanup stuff */
	atexit(reset_tty_atexit);
	memset(&sa,0,sizeof sa);
	sa.sa_handler = reset_tty_handler;
	sigaction(SIGHUP,&sa,NULL);
	sigaction(SIGINT,&sa,NULL);
	sigaction(SIGPIPE,&sa,NULL);
	sigaction(SIGTERM,&sa,NULL);
	/*apply modified termios */
	saved_portfd=portfd;
	tcflush(portfd,TCIFLUSH);
	tcsetattr(portfd,TCSADRAIN,&newtios);
	
		
	if (ioctl (portfd, TIOCGRS485, &rs485conf) < 0) 
	{
		/* Error handling.*/ 
		printf("ioctl TIOCGRS485 error.\n");
	}
	/* Enable RS485 mode: */
	rs485conf.flags |= SER_RS485_ENABLED;

	/* Set logical level for RTS pin equal to 1 when sending: */
	rs485conf.flags |= SER_RS485_RTS_ON_SEND;
	//rs485conf.flags |= SER_RS485_RTS_AFTER_SEND;

	/* set logical level for RTS pin equal to 0 after sending: */ 
	rs485conf.flags &= ~(SER_RS485_RTS_AFTER_SEND);
	//rs485conf.flags &= ~(SER_RS485_RTS_ON_SEND);

	/* Set rts delay after send, if needed: */
	rs485conf.delay_rts_after_send = 0x80;

	if (ioctl (portfd, TIOCSRS485, &rs485conf) < 0)
	{
		/* Error handling.*/ 
		printf("ioctl TIOCSRS485 error.\n");
	}

	if (ioctl (portfd, TIOCGRS485, &rs485conf_bak) < 0)
	{
		/* Error handling.*/ 
		printf("ioctl TIOCGRS485 error.\n");
	}
	else
	{
		printf("rs485conf_bak.flags 0x%x.\n", rs485conf_bak.flags);
		printf("rs485conf_bak.delay_rts_before_send 0x%x.\n", rs485conf_bak.delay_rts_before_send);
		printf("rs485conf_bak.delay_rts_after_send 0x%x.\n", rs485conf_bak.delay_rts_after_send);
	}

	return portfd;
}


// Ray add 
void RS485_TX_Data(int length, uint8_t *pData)
{
  int i, rev1;

  i = 0;
  while (i< length)
  {
    rev1 = write(g_portfd,(uint8_t *)pData, 1);
    
    if ( rev1 != 1 )
    {
        printf("fail write %d\n",rev1);
	break;
    }
    pData++;
    i++;
  }
}
#if 0
unsigned char OpenDevice(unsigned char id)
{
  begin(id);
  return(writeSingleCoil(ADDR_SENSORDEVICE, 1));
}

unsigned char ReadDevice(unsigned char id)
{
  begin(id);
  return(readInputRegisters(ADDR_READSENSOR, 2));
}

unsigned char DeviceLedOn(unsigned char id)
{
  begin(id);
  return(writeSingleCoil(ADDR_LED_1,1));
}


uint8_t DeviceLedOff(uint8_t id)
{
  begin(id);
  return(writeSingleCoil(ADDR_LED_1,0));
}
#endif
#if 0
void * process1(void* arg)
{
  unsigned int u32Item;
  int rev1, rev2;
	

  g_portfd = (int) arg;

  printf(" [1] Open sensor 1\n");
  printf(" [2] Read sensor 1\n");	
  printf(" [3] LED ON 1\n");	
  printf(" [4] LED OFF 1\n");	
  printf(" [5] Open sensor 2\n");
  printf(" [6] Read sensor 2\n");	
  printf(" [7] LED ON 2\n");	
  printf(" [8] LED OFF 2\n");	
 
  while (1)
  {		 
  u32Item = getchar();
  if (u32Item == '1')
  {
    if ( OpenDevice(1) == 0 )
    {
      printf("Open Sensor 1 OK.\n\n");
    }
    else
    {
      printf("Open Sensor 1 Fail !\n\n");
    }
  }
  else if (u32Item == '2')
  {
    if (ReadDevice(1) != 0 )
    {
      printf("Read sensor 1 fails\n\n");
    }
  }
  else if ( u32Item == '3')
  {
    if ( DeviceLedOn(1) == 0 )
    {
      printf("LED on 1 OK\n\n");
    }
    else
    {
      printf("LED on  1 fails\n\n");
    }
  }
  else if ( u32Item == '4')
  {
    if ( DeviceLedOff(1) == 0 )
    {
      printf("LED off 1 OK\n\n");
    }
    else
    {
      printf("LED off 1 fails\n\n");
    }
  }
  else if (u32Item == '5')
  {
    if ( OpenDevice(2) == 0 )
    {
      printf("Open Sensor 2 OK.\n\n");
    }
    else
    {
      printf("Open Sensor 2 Fail !\n\n");
    }
  }
  else if (u32Item == '6')
  {
    if (ReadDevice(2) != 0 )
    {
      printf("Read sensor 2 fails\n\n");
    }
  }
  else if ( u32Item == '7')
  {
    if ( DeviceLedOn(2 ) == 0 )
    {
      printf("LED on 2 OK\n\n");
    }
    else
    {
      printf("LED on 2 fails\n\n");
    }
  }
  else if ( u32Item == '8')
  {
    if ( DeviceLedOff(2) == 0 )
    {
      printf("LED off 2 OK\n\n");
    }
    else
    {
      printf("LED off 2 fails\n\n");
    }
  }
  else if ( u32Item == '9')
    break;
  }
}	
//#else
void * process1(void* arg)
{
  int status;
  int idNo;
  int number;
  int u32Item;
  int  retval, count, toggle;
  int i, no1;

  g_portfd = (int) arg;
  for (i=1; i<= RS485_NUMBER; i++)
  {
    bConnected[i] = DISCONNECT;   // -1 no connect, 1 connected
  }

  count = 0;
  toggle= 0;
  no1 = 0;
  while (1)
  {
    if ( bConnected[idNo] == CONNECTION )
    {
    no1++;
    if ( no1 % 10 == 5 )
      g_Cmd = RS485_LEDON | 0x0100;
    else if (no1 % 10 == 0 )
      g_Cmd = RS485_LEDOFF | 0x0100;
    }
    usleep(200*1000);
    count++;
    count %= RS485_NUMBER;
    if ( count == 0 )
      count = RS485_NUMBER;
    idNo = count;
    toggle++;
    toggle %= 5;
    if ( toggle == 1 )
    {
      if ( bConnected[idNo] == CONNECTION )
      {
	if ( ReadDevice(idNo) == 0 )
	{
				
	}
	else
	{
	  bConnected[idNo] = DISCONNECT;							
	}
      }
      else
      {
	if ( OpenDevice(idNo) == 0 )
	{
	  bConnected[idNo] = CONNECTION;	
	  printf("\n %d Open Sensor\n", idNo);
	}					
      }
      toggle = 1;
    }
    if ( (g_Cmd & 0x00FF) == RS485_LEDON )
    {
      number = (g_Cmd & 0xFF00) >> 8;
      if ( bConnected[number] == CONNECTION )
      {
	if ( DeviceLedOn(number) == 0 )
	{ 
	  printf("\n %d LED ON \n",number);
	}
      }
      else
      {
	printf("\n %d disconnection\n", number);
      }
      g_Cmd = 0;
    }
    else if ( (g_Cmd & 0x00FF)== RS485_LEDOFF )
    {
      number = (g_Cmd & 0xFF00) >> 8;
      if ( bConnected[number] == CONNECTION )
      {
	if ( DeviceLedOff(number) == 0 )
	{ 
	  printf("\n %d LED OFF \n",number);
	}
      }
      else
      {
	printf("\n %d disconnection\n", number);
      }
      g_Cmd = 0;
    }			
  }
}
#endif

/**
*@breif 	main()
*/
int rs485_main(void)
{
  char *dev[10]={"/dev/ttyS6"};
  unsigned int i;
    uint32_t flags;

  printf("\n RS485 for UART 6\n");

  for(i = 0; i < 1; i++)
  {
    if((fd[i] = open_port(dev[i]))<0)
      return -1;
  }
	g_portfd = (int) fd[0];

// Ray added write with nonblocking
  flags = fcntl(g_portfd,F_GETFL);
  flags = O_NONBLOCK;
  fcntl(g_portfd, F_SETFL, flags);
  //pthread_create(&threads[0], NULL, process1, (void*)(fd[0]));
  //pthread_join(threads[0], NULL);
		   
  return 0;
}

