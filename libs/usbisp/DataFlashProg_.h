/******************************************************************************
 * @file     DataFlashProg.h
 * @brief    Data flash programming driver header
 *
 * @note
 * Copyright (C) 2014~2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __DATA_FLASH_PROG_H__
#define __DATA_FLASH_PROG_H__

#define MASS_STORAGE_OFFSET       0x0001F000 // 0x00008000  /* To avoid the code to write APROM */
#define DATA_FLASH_STORAGE_SIZE   0x1000  // (36*1024)  /* Configure the DATA FLASH storage size. To pass USB-IF MSC Test, it needs > 64KB */
#define FLASH_PAGE_SIZE           512
#define BUFFER_PAGE_SIZE          512


#endif  /* __DATA_FLASH_PROG_H__ */

/*** (C) COPYRIGHT 2014~2015 Nuvoton Technology Corp. ***/
