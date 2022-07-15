
#include "NUC123.h"
#include "usbd.h"
#include "fmc.h"
#include "usbd.h"
#include "massstorage.h"


#define USERCONFIG_0 0xFFFFFF7F
#define USERCONFIG_1 0x0001F000 //dataflash base

//#define CONFIG_BASE      0x00300000

//#define FMC_APROM_BASE          0x00000000UL    /*!< APROM  Base Address         */
//#define FMC_LDROM_BASE          0x00100000UL    /*!< LDROM  Base Address         */

void usbflasher_start()
{
    SYS_UnlockReg();
    // Enable FMC ISP function
    FMC->ISPCON |=  FMC_ISPCON_ISPEN_Msk;
	// Read User Configuration
	uint32_t au32Config[2];
	au32Config[0] = FMC_Read(FMC_CONFIG_BASE);
	au32Config[1] = FMC_Read(FMC_CONFIG_BASE + 4);
	
	// check config word - start from ldrom
	if (au32Config[0] != USERCONFIG_0)
    {
        // Enable User Configuration update
		FMC->ISPCON |= FMC_ISPCON_CFGUEN_Msk;
        au32Config[0] = USERCONFIG_0;
        au32Config[1] = USERCONFIG_1;
		// Write User Configuration
        FMC_Write(FMC_CONFIG_BASE, au32Config[0]);
        FMC_Write(FMC_CONFIG_BASE + 4, au32Config[1]);
		
		// Disable ISP Functions
		FMC->ISPCON &= ~FMC_ISPCON_ISPEN_Msk;

		while(1);
        /* Reset Chip to reload new CONFIG value */
        SYS->IPRSTC1 = SYS_IPRSTC1_CHIP_RST_Msk;
    }

    USBD_Open(&gsInfo, MSC_ClassRequest, NULL);
    
    USBD_SetConfigCallback(MSC_SetConfig);

    /* Endpoint configuration */
    MSC_Init();
    USBD_Start();
    //NVIC_EnableIRQ(USBD_IRQn);

    while(1)
    {
		// polling mode
		USBD_IRQHandler();
        MSC_ProcessCmd();
    }
}


