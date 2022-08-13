/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include "MassStorage_ISP.h"
//volatile uint32_t GPIOA_TEMP,GPIOB_TEMP,GPIOC_TEMP,GPIOD_TEMP;
//volatile uint32_t GPIOA_TEMP1,GPIOB_TEMP1,GPIOC_TEMP1,GPIOD_TEMP1;
/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
#define GCR_BA		0x50000000
#define GPA_MFP		GCR_BA+0x030
#define GPB_MFP		GCR_BA+0x034
#define GPC_MFP		GCR_BA+0x038
#define ALT_MFP		GCR_BA+0x50

#define GP_BA		0x50004000
#define GPIOA_PIN	GP_BA+0x010
#define GPIOB_PIN	GP_BA+0x050
#define GPIOC_PIN	GP_BA+0x090

#ifdef PROTOTYPE
	#define SWITCH_PORT	GPIOA_PIN
	#define SWITCH_PIN	BIT10
#else
	#define SWITCH_PORT	GPIOB_PIN
	#define SWITCH_PIN	BIT9
#endif

#define FMC_ISPCMD_READ        0x00     /*!< ISP Command: Read Flash       */
#define FMC_ISPCMD_PROGRAM     0x21     /*!< ISP Command: Program Flash    */
#define FMC_ISPCMD_PAGE_ERASE  0x22     /*!< ISP Command: Page Erase Flash */
#define FMC_ISPCMD_VECMAP      0x2e     /*!< ISP Command: Set VECMAP       */
#define FMC_ISPCMD_READ_UID    0x04     /*!< ISP Command: Read Unique ID   */
#define FMC_ISPCMD_READ_CID    0x0B     /*!< ISP Command: Read Company ID   */
#define FMC_ISPCMD_READ_DID    0x0C     /*!< ISP Command: Read Device ID    */

void myFMC_Write(uint32_t u32addr, uint32_t u32data);
void myFMC_Erase(uint32_t u32addr);

#define FMC_Erase myFMC_Erase
#define FMC_Write myFMC_Write

static __inline uint32_t FMC_Read(uint32_t u32addr)
{
    outp32(FMC_ISPCMD, FMC_ISPCMD_READ);
    outp32(FMC_ISPADR, u32addr);
    outp32(FMC_ISPTRG, 0x01);
    __ISB();
    while(FMC->ISPTRG.ISPGO);
    return FMC->ISPDAT;
}

void update_flash(void)
{
	int i;
	uint32_t u32Data = 0;

	FMC->ISPCON.ISPEN = 1;

	//read the firmware header
	u32Data = FMC_Read(0x10000 - 8);		//identification

	if(u32Data == 0xDEADBEEF) {
		FMC->ISPCON.APUEN = 1;

		//erase lower 32k
		for(i=0;i<0x8000;i+=512) {
			FMC_Erase(i);
		}

		//copy upper flash to lower flash
		for(i=0;i<0x8000;i+=4) {
			u32Data = FMC_Read(i + 0x8000);
			FMC_Write(i, u32Data);
		}

		//erase lower 32k
		for(i=0x8000;i<0x10000;i+=512) {
			FMC_Erase(i);
		}

		FMC->ISPCON.APUEN = 0;

	}
	FMC->ISPCON.ISPEN = 0;
}

int32_t main(void)
{
  
	volatile uint32_t u32INTSTS, temp;

	UNLOCKREG();

	update_flash();

	if ((inp32(SWITCH_PORT) & SWITCH_PIN) != 0)	 {
		goto MSDISP;
	}

	FMC->ISPCON.BS = 0;  
	outp32(&SYS->IPRSTC1, 0x02);
	while(1);

MSDISP:				
    /* Enable internal osc */
	outp32(&SYSCLK->PWRCON,0x1C);
    RoughDelay(0x4000);                     

    /* Enable PLL */
	outp32(&SYSCLK->AHBCLK,0x04);
	outp32(&SYSCLK->CLKDIV,0x21);
	outp32(&SYSCLK->PLLCON,0x00080418);
	RoughDelay(0x4000);

	/* Switch HCLK source to PLL */
	outp32(&SYSCLK->CLKSEL0,0x3A);
    RoughDelay(0x4000);                     

    
    /* Initialize USB Device function */

    /* Enable PHY to send bus reset event */
    _DRVUSB_ENABLE_USB();

    outp32(&USBD->DRVSE0, 0x01);
    RoughDelay(1000);
    outp32(&USBD->DRVSE0, 0x00);
    RoughDelay(1000);

    /* Disable PHY */
    _DRVUSB_DISABLE_USB();

    /* Enable USB device clock */
    outp32(&SYSCLK->APBCLK, BIT27);

    /* Reset IP */	
    outp32(&SYS->IPRSTC2, BIT27);
    outp32(&SYS->IPRSTC2, 0x0);	

    _DRVUSB_ENABLE_USB();
    outp32(&USBD->DRVSE0, 0x01);
    RoughDelay(1000);
    outp32(&USBD->DRVSE0, 0x00);

	g_u8UsbState = USB_STATE_DETACHED;
	_DRVUSB_TRIG_EP(1, 0x08);
	UsbFdt();

    /* Initialize mass storage device */
    udcFlashInit();  
      
    /* Start USB Mass Storage */

    /* Handler the USB ISR by polling */
	while(1)
	{
	    u32INTSTS = _DRVUSB_GET_EVENT_FLAG();
	
        if (u32INTSTS & INTSTS_FLDET)
	    {
	        /* Handle the USB attached/detached event */
		    UsbFdt();
	    }
	    else if(u32INTSTS & INTSTS_BUS)
	    {
	        /* Handle the USB bus event: Reset, Suspend, and Resume */
		    UsbBus();
	    }
	    else if(u32INTSTS & INTSTS_USB)
	    {
	        /* Handle the USB Protocol/Clase event */
		    UsbUsb(u32INTSTS);
        }
	}
}
