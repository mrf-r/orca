/****************************************************************************//**
 * @file     startup_NUC123.S
 * @version  V1.00
 * @brief    CMSIS Device Startup File
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/



    .syntax unified
    .arch   armv8 - m.base
    
    .section .vectors
    .align  2
    .global __Vectors
__Vectors:
    .long   __StackTop            /* Top of Stack */
    .long   Reset_Handler         /* Reset Handler */
    .long   NMI_Handler           /* NMI Handler */
    .long   HardFault_Handler     /* Hard Fault Handler */
    .long   0                     /* Reserved */
    .long   0                     /* Reserved */
    .long   0                     /* Reserved */
    .long   0                     /* Reserved */
    .long   0                     /* Reserved */
    .long   0                     /* Reserved */
    .long   0                     /* Reserved */
    .long   SVC_Handler           /* SVCall Handler */
    .long   0                     /* Reserved */
    .long   0                     /* Reserved */
    .long   PendSV_Handler        /* PendSV Handler */
    .long   SysTick_Handler       /* SysTick Handler */
    
    /* External interrupts */
    
    .long   BOD_IRQHandler  
    .long   WDT_IRQHandler  
    .long   EINT0_IRQHandler
    .long   EINT1_IRQHandler
    .long   GPAB_IRQHandler 
    .long   GPCDF_IRQHandler
    .long   PWMA_IRQHandler 
    .long   Default_Handler 
    .long   TMR0_IRQHandler 
    .long   TMR1_IRQHandler 
    .long   TMR2_IRQHandler 
    .long   TMR3_IRQHandler 
    .long   UART0_IRQHandler
    .long   UART1_IRQHandler
    .long   SPI0_IRQHandler 
    .long   SPI1_IRQHandler 
    .long   SPI2_IRQHandler 
    .long   SPI3_IRQHandler 
    .long   I2C0_IRQHandler 
    .long   I2C1_IRQHandler 
    .long   CAN0_IRQHandler 
    .long   CAN1_IRQHandler
    .long   SC012_IRQHandler 
    .long   USBD_IRQHandler  
    .long   PS2_IRQHandler  
    .long   ACMP_IRQHandler 
    .long   PDMA_IRQHandler
    .long   I2S_IRQHandler 
    .long   PWRWU_IRQHandler
    .long   ADC_IRQHandler
    .long   Default_Handler  
    .long   RTC_IRQHandler  
    
    .size   __Vectors, . - __Vectors
    
    
    
    .text
    .thumb
    .thumb_func
    .align  2
    .global Reset_Handler
    .type   Reset_Handler, % function
    
Reset_Handler:
    bl SystemInit
    bl main
    b .
    
    .pool
    .size   Reset_Handler, . - Reset_Handler
    
    .align  1
    .thumb_func
    .weak   Default_Handler
    .type   Default_Handler, % function
    
Default_Handler:
    b   .
    .size   Default_Handler, . - Default_Handler
    
    /*    Macro to define default handlers. Default handler
     *    will be weak symbol and just dead loops. They can be
     *    overwritten by other handlers */
     
    .macro  def_irq_handler handler_name
    .weak   \handler_name
    .set    \handler_name, Default_Handler
    .endm
    
    def_irq_handler NMI_Handler
    def_irq_handler HardFault_Handler
    def_irq_handler SVC_Handler
    def_irq_handler PendSV_Handler
    def_irq_handler SysTick_Handler


    def_irq_handler BOD_IRQHandler
    def_irq_handler WDT_IRQHandler
    def_irq_handler EINT0_IRQHandler
    def_irq_handler EINT1_IRQHandler
    def_irq_handler GPAB_IRQHandler
    def_irq_handler GPCDF_IRQHandler
    def_irq_handler PWMA_IRQHandler
    def_irq_handler TMR0_IRQHandler
    def_irq_handler TMR1_IRQHandler
    def_irq_handler TMR2_IRQHandler
    def_irq_handler TMR3_IRQHandler
    def_irq_handler UART0_IRQHandler
    def_irq_handler UART1_IRQHandler
    def_irq_handler SPI0_IRQHandler
    def_irq_handler SPI1_IRQHandler
    def_irq_handler SPI2_IRQHandler
    def_irq_handler SPI3_IRQHandler
    def_irq_handler I2C0_IRQHandler
    def_irq_handler I2C1_IRQHandler
    def_irq_handler CAN0_IRQHandler
    def_irq_handler CAN1_IRQHandler
    def_irq_handler SC012_IRQHandler
    def_irq_handler USBD_IRQHandler
    def_irq_handler PS2_IRQHandler
    def_irq_handler ACMP_IRQHandler
    def_irq_handler PDMA_IRQHandler
    def_irq_handler I2S_IRQHandler
    def_irq_handler PWRWU_IRQHandler
    def_irq_handler ADC_IRQHandler
    def_irq_handler RTC_IRQHandler
    
