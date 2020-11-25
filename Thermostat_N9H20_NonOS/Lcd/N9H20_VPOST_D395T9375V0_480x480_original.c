/***************************************************************************
 *                                                                         *
 * Copyright (c) 2007 - 2009 Nuvoton Technology Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     N9H20_VPOST_FW050TFT_800x480.c
 *
 * VERSION
 *     0.1 
 *
 * DESCRIPTION
 *
 *
 *
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *
 *
 *     
 * HISTORY
 *     2009.03.16		Created by Shu-Ming Fan
 *
 *
 * REMARK
 *     None
 *
 *
 **************************************************************************/
#include "stdio.h"
#include "stdlib.h"
#include "N9H20_vpost.h"

extern void LCDDelay(unsigned int nCount);

#if defined(__HAVE_D395T9375V0_480x480__)

static UINT32 g_nScreenWidth;
static UINT32 g_nScreenHeight;

typedef enum 
{
	eEXT 	= 0,
	eX32K 	= 1,
	eAPLL  	= 2,
	eUPLL  	= 3
}E_CLK;

#define SpiEnable()		outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) & ~0x20)
#define SpiDisable()	outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) |  0x20)	

#define SpiHighSCK()	outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) |  0x10)	
#define SpiLowSCK()		outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) & ~0x10)
#define SpiHighSDA()	outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) |  0x8)	
#define SpiLowSDA()		outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) & ~0x8)


static UINT32 GetPLLOutputKhz(
	E_CLK eSysPll,
	UINT32 u32FinKHz
	)
{
	UINT32 u32Freq, u32PllCntlReg;
	UINT32 NF, NR, NO;
	
	UINT8 au8Map[4] = {1, 2, 2, 4};
	if(eSysPll==eSYS_APLL)
		u32PllCntlReg = inp32(REG_APLLCON);
	else if(eSysPll==eSYS_UPLL)	
		u32PllCntlReg = inp32(REG_UPLLCON);		
	
	NF = (u32PllCntlReg&FB_DV)+2;
	NR = ((u32PllCntlReg & IN_DV)>>9)+2;
	NO = au8Map[((u32PllCntlReg&OUT_DV)>>14)];
//	sysprintf("PLL regster = 0x%x\n", u32PllCntlReg);	
//	sysprintf("NF = %d\n", NF);
//	sysprintf("NR = %d\n", NR);
//	sysprintf("NOr = %d\n", NO);
		
	u32Freq = u32FinKHz*NF/NR/NO;
//	sysprintf("PLL Freq = %d\n", u32Freq);
	return u32Freq;
}

static void BacklightControl(int OnOff)
{	
#if 1
	// GPD[0] set OUTPUT mode  => control the backlight
	outpw(REG_GPIOD_OMD, (inpw(REG_GPIOD_OMD) & 0x0000FFFF)| 0x00000001);
	if(OnOff==TRUE) {
		// GPD[0] turn on the backlight
		outpw(REG_GPIOD_DOUT, (inpw(REG_GPIOD_DOUT) & 0x0000FFFF)| 0x00000001);
	} else {
		// GPD[0] diable backlight
		outpw(REG_GPIOD_DOUT, (inpw(REG_GPIOD_DOUT) & 0x0000FFFF) & 0xFFFFFFFE);
	}
#else
	// GPA[11] set OUTPUT mode  => control the backlight
	outpw(REG_GPIOA_OMD, (inpw(REG_GPIOA_OMD) & 0x0000FFFF)| 0x00000800);
	if(OnOff==TRUE) {
		// GPA[11] turn on the backlight
		outpw(REG_GPIOA_DOUT, (inpw(REG_GPIOA_DOUT) & 0x0000FFFF)| 0x00000800);
	} else {
		// GPA[11] diable backlight
		outpw(REG_GPIOA_DOUT, (inpw(REG_GPIOA_DOUT) & 0x0000FFFF) & 0xFFFFF7FF);
	}
#endif
}

static void delay_loop(UINT32 u32delay)
{
	volatile UINT32 ii, jj;
	for (jj=0; jj<u32delay; jj++)
		for (ii=0; ii<200; ii++);
}

void SpiDelay(UINT32 u32TimeCnt)
{
	UINT32 ii, jj;
	
	for (ii=0; ii<u32TimeCnt; ii++)
		for (jj=0; jj<100; jj++)
			jj++;
}

void SpiInit()
{	
	outp32(REG_GPIOB_OMD, inp32(REG_GPIOB_OMD) | 0x38);
	outp32(REG_GPIOB_PUEN, inp32(REG_GPIOB_PUEN) | 0x38);
}

void ILI_RdWtRegInit()
{
	outp32(REG_GPBFUN, inp32(REG_GPBFUN) & ~(MF_GPB3+MF_GPB4+MF_GPB5));	//GPB3/4/5 
		
	SpiInit();
	SpiHighSDA();		// serial data pin low
	SpiHighSCK();		// serial clock pin low
	SpiDisable();
	SpiDelay(5);				
}

static VOID WriteComm
(
 UINT8  u8AddrIndex		// LCD register address
)
{
	UINT32 i;
	SpiEnable();	
	SpiHighSCK();				
	// send WR COMMAND bit		
	SpiLowSCK();
	SpiDelay(2);		
	SpiLowSDA();		
	SpiDelay(2);
	SpiHighSCK();		
	SpiDelay(2);						
	// Send register data LSB first
	for( i = 0; i < 8; i ++ )
	{
		SpiLowSCK();						
		if ( u8AddrIndex&0x80 )
			SpiHighSDA();
		else
			SpiLowSDA();
		
		SpiDelay(3);
		SpiHighSCK();
		u8AddrIndex<<=1;
		SpiDelay(2);
	}

} // WriteComm

static VOID WriteData
(
 UINT8  u8WriteData		// LCD register data
 )
{
	UINT32 i;
	
	SpiEnable();	  
	SpiHighSCK();			
	// send WR data bit		
	SpiLowSCK();
	SpiDelay(2);		
	SpiHighSDA();		
	SpiDelay(2);
	SpiHighSCK();		
	SpiDelay(2);				
	for( i = 0; i < 8; i ++ )
	{
		SpiLowSCK();						
		if ( u8WriteData&0x80 )
			SpiHighSDA();
		else
			SpiLowSDA();

		SpiDelay(3);
		SpiHighSCK();
		u8WriteData<<=1;
		SpiDelay(2);
	}
} // WriteData

static void LCM_Init(void)  
{  
    ILI_RdWtRegInit();
  
    WriteComm (0xFF);
	WriteData (0x77);
	WriteData (0x01);
	WriteData (0x00);
	WriteData (0x00);
	WriteData (0x13);
	WriteComm (0xEF);
	WriteData (0x08);
	WriteComm (0xFF);
	WriteData (0x77);
	WriteData (0x01);
	WriteData (0x00);
	WriteData (0x00);
	WriteData (0x10);
	WriteComm (0xC0);
	WriteData (0x3B);
	WriteData (0x00);
	WriteComm (0xC1);
	WriteData (0x0D);
	WriteData (0x02);
	WriteComm (0xC2);
	WriteData (0x21);
	WriteData (0x08);
	WriteComm (0xCD);
	WriteData (0x08);//18-bit/pixel: MDT=0:D[21:16]=R,D[13:8]=G,D[5:0]=B(CDH=00) ;
	                 //              MDT=1:D[17:12]=R,D[11:6]=G,D[5:0]=B(CDH=08) ;
	WriteComm (0xB0);
	WriteData (0x00);
	WriteData (0x11);
	WriteData (0x18);
	WriteData (0x0E);
	WriteData (0x11);
	WriteData (0x06);
	WriteData (0x07);
	WriteData (0x08);
	WriteData (0x07);
	WriteData (0x22);
	WriteData (0x04);
	WriteData (0x12);
	WriteData (0x0F);
	WriteData (0xAA);
	WriteData (0x31);
	WriteData (0x18);
	WriteComm (0xB1);
	WriteData (0x00);
	WriteData (0x11);
	WriteData (0x19);
	WriteData (0x0E);
	WriteData (0x12);
	WriteData (0x07);
	WriteData (0x08);
	WriteData (0x08);
	WriteData (0x08);
	WriteData (0x22);
	WriteData (0x04);
	WriteData (0x11);
	WriteData (0x11);
	WriteData (0xA9);
	WriteData (0x32);
	WriteData (0x18);
	WriteComm (0xFF);
	WriteData (0x77);
	WriteData (0x01);
	WriteData (0x00);
	WriteData (0x00);
	WriteData (0x11);
	WriteComm (0xB0);
	WriteData (0x60);
	WriteComm (0xB1);
	WriteData (0x30);
	WriteComm (0xB2);
	WriteData (0x87);
	WriteComm (0xB3);
	WriteData (0x80);
	WriteComm (0xB5);
	WriteData (0x49);
	WriteComm (0xB7);
	WriteData (0x85);
	WriteComm (0xB8);
	WriteData (0x21);
	WriteComm (0xC1);
	WriteData (0x78);
	WriteComm (0xC2);
	WriteData (0x78);
//	WriteComm (0xC3);
//	WriteData (0x80);
	
	delay_loop(20);
	
	WriteComm (0xE0);
	WriteData (0x00);
	WriteData (0x1B);
	WriteData (0x02);
	WriteComm (0xE1);
	WriteData (0x08);
	WriteData (0xA0);
	WriteData (0x00);
	WriteData (0x00);
	WriteData (0x07);
	WriteData (0xA0);
	WriteData (0x00);
	WriteData (0x00);
	WriteData (0x00);
	WriteData (0x44);
	WriteData (0x44);
	WriteComm (0xE2);
	WriteData (0x11);
	WriteData (0x11);
	WriteData (0x44);
	WriteData (0x44);
	WriteData (0xED);
	WriteData (0xA0);
	WriteData (0x00);
	WriteData (0x00);
	WriteData (0xEC);
	WriteData (0xA0);
	WriteData (0x00);
	WriteData (0x00);
	WriteComm (0xE3);
	WriteData (0x00);
	WriteData (0x00);
	WriteData (0x11);
	WriteData (0x11);
	WriteComm (0xE4);
	WriteData (0x44);
	WriteData (0x44);
	WriteComm (0xE5);
	WriteData (0x0A);
	WriteData (0xE9);
	WriteData (0xD8);
	WriteData (0xA0);
	WriteData (0x0C);
	WriteData (0xEB);
	WriteData (0xD8);
	WriteData (0xA0);
	WriteData (0x0E);
	WriteData (0xED);
	WriteData (0xD8);
	WriteData (0xA0);
	WriteData (0x10);
	WriteData (0xEF);
	WriteData (0xD8);
	WriteData (0xA0);
	WriteComm (0xE6);
	WriteData (0x00);
	WriteData (0x00);
	WriteData (0x11);
	WriteData (0x11);
	WriteComm (0xE7);
	WriteData (0x44);
	WriteData (0x44);
	WriteComm (0xE8);
	WriteData (0x09);
	WriteData (0xE8);
	WriteData (0xD8);
	WriteData (0xA0);
	WriteData (0x0B);
	WriteData (0xEA);
	WriteData (0xD8);
	WriteData (0xA0);
	WriteData (0x0D);
	WriteData (0xEC);
	WriteData (0xD8);
	WriteData (0xA0);
	WriteData (0x0F);
	WriteData (0xEE);
	WriteData (0xD8);
	WriteData (0xA0);
	WriteComm (0xEB);
	WriteData (0x02);
	WriteData (0x00);
	WriteData (0xE4);
	WriteData (0xE4);
	WriteData (0x88);
	WriteData (0x00);
	WriteData (0x40);
	WriteComm (0xEC);
	WriteData (0x3C);
	WriteData (0x00);
	WriteComm (0xED);
	WriteData (0xAB);
	WriteData (0x89);
	WriteData (0x76);
	WriteData (0x54);
	WriteData (0x02);
	WriteData (0xFF);
	WriteData (0xFF);
	WriteData (0xFF);
	WriteData (0xFF);
	WriteData (0xFF);
	WriteData (0xFF);
	WriteData (0x20);
	WriteData (0x45);
	WriteData (0x67);
	WriteData (0x98);
	WriteData (0xBA);
	WriteComm (0xFF);
	WriteData (0x77);
	WriteData (0x01);
	WriteData (0x00);
	WriteData (0x00);
	WriteData (0x00);
	WriteComm (0x3A);
	WriteData (0x66);//55/50=16bit(RGB565);66=18bit(RGB666);77或默认不写3AH是=24bit(RGB888)
	WriteComm (0x21);
	WriteComm (0x11);
	delay_loop(120);
	WriteComm (0x29);
	delay_loop(20);
}

INT vpostLCMInit_D395T9375V0_480x480(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf)
{
	volatile S_DRVVPOST_SYNCLCM_WINDOW sWindow = {480,480,480};	
	volatile S_DRVVPOST_SYNCLCM_HTIMING sHTiming = {20,30,(UINT8)10};
	volatile S_DRVVPOST_SYNCLCM_VTIMING sVTiming = {10,10,10};
//	volatile S_DRVVPOST_SYNCLCM_POLARITY sPolarity = {FALSE,FALSE,TRUE,FALSE};
	volatile S_DRVVPOST_SYNCLCM_POLARITY sPolarity = {TRUE,TRUE,FALSE,TRUE};
//	volatile S_DRVVPOST_SYNCLCM_POLARITY sPolarity = {TRUE,TRUE,FALSE,TRUE};

	UINT32 nBytesPixel, u32PLLclk, u32ClockDivider;

//#define OPT_24BIT_MODE

	// VPOST clock control
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) | VPOST_CKE | HCLK4_CKE);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) | VPOSTRST);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) & ~VPOSTRST);	
	
	u32PLLclk = GetPLLOutputKhz(eUPLL, 12000);			// CLK_IN = 12 MHz
	u32ClockDivider = u32PLLclk / 30000;		

	u32ClockDivider--;
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N0));
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N1) | ((u32ClockDivider & 0xFF) << 8));						
	outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) & ~VPOST_S);
	outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) | (3<<3));		// VPOST clock from UPLL

	LCM_Init();
	
	vpostVAStopTrigger();	

	// Enable VPOST function pins
#ifdef	OPT_24BIT_MODE		
	vpostSetDataBusPin(eDRVVPOST_DATA_24BITS);
#else
	//vpostSetDataBusPin(eDRVVPOST_DATA_16BITS);
	vpostSetDataBusPin(eDRVVPOST_DATA_18BITS);	
#endif	
		  
	// LCD image source select
	vpostSetLCM_ImageSource(eDRVVPOST_FRAME_BUFFER);
	
	// configure LCD interface
	vpostSetLCM_TypeSelect(eDRVVPOST_HIGH_RESOLUTINO_SYNC);

	// configure LCD timing sync or async with TV timing	
	vpostsetLCM_TimingType(eDRVVPOST_ASYNC_TV);
	
    // Configure Parallel LCD interface (16/18/24-bit data bus)
#ifdef	OPT_24BIT_MODE		
    vpostSetParalelSyncLCM_Interface(eDRVVPOST_PRGB_24BITS);
#else    
    //vpostSetParalelSyncLCM_Interface(eDRVVPOST_PRGB_16BITS);
    vpostSetParalelSyncLCM_Interface(eDRVVPOST_PRGB_18BITS);    
#endif    
    
    // set Horizontal scanning line timing for Syn type LCD 
    vpostSetSyncLCM_HTiming(&sHTiming);

	// set Vertical scanning line timing for Syn type LCD   
    vpostSetSyncLCM_VTiming(&sVTiming);
	
	// set both "active pixel per line" and "active lines per screen" for Syn type LCD   
	vpostSetSyncLCM_ImageWindow(&sWindow);

  	// set Hsync/Vsync/Vden/Pclk poalrity
	vpostSetSyncLCM_SignalPolarity(&sPolarity);  	
    
    // set frambuffer base address
    if(pFramebuf != NULL) {
		vpostAllocVABufferFromAP(pFramebuf);
	} else {
    	if( vpostAllocVABuffer(plcdformatex, nBytesPixel)==FALSE)
    		return ERR_NULL_BUF;
    }
	
	// set frame buffer data format
	vpostSetFrameBuffer_DataType(plcdformatex->ucVASrcFormat);
	
	vpostSetYUVEndianSelect(eDRVVPOST_YUV_LITTLE_ENDIAN);
	
	// enable LCD controller
	vpostVAStartTrigger();
	
	//BacklightControl(TRUE);			
	return 0;
}

INT32 vpostLCMDeinit_D395T9375V0_480x480(VOID)
{
	vpostVAStopTrigger();
	vpostFreeVABuffer();
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) & ~VPOST_CKE);	
	return 0;
}

VOID vpostEnaBacklight(void)
{
#define VPOST_ICE_DEBUG	
#ifndef VPOST_ICE_DEBUG
	/* set BL_EN (GPA6) to High */
	outpw(REG_GPAFUN, inpw(REG_GPAFUN) & ~MF_GPE6);
	outpw(REG_GPIOA_OMD, inpw(REG_GPIOA_OMD) | 0x00000040);
	outpw(REG_GPIOA_DOUT, inpw(REG_GPIOA_DOUT) | 0x00000040);

	/* set BL_CTL (GPD0) to Low */
	outpw(REG_GPDFUN, inpw(REG_GPDFUN) & ~MF_GPD0);
	outpw(REG_GPIOD_OMD, inpw(REG_GPIOD_OMD) | 0x00000001);
	outpw(REG_GPIOD_DOUT, inpw(REG_GPIOD_DOUT) & ~0x00000001);
#endif
}

#endif    //__HAVE_D395T9375V0_480x480__