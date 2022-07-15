ENTRY(Reset_Handler)

__stack_size = 0x400;

MEMORY
{
  APROM (rx)  : ORIGIN = 0x00000000, LENGTH = 68K
  LDROM (rx)  : ORIGIN = 0x00100000, LENGTH = 4K
  SRAM (rwx)  : ORIGIN = 0x20000000 + __stack_size, LENGTH = 20K - __stack_size
}

REGION_ALIAS("ROM", SRAM);
REGION_ALIAS("RAM", SRAM);

__StackTop = ORIGIN(RAM);
end = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
	.text :
	{
		KEEP(*(.vectors))

		*(.text*)
		*(.rodata*)

		KEEP(*(.init))
		KEEP(*(.fini))
		
		/* .ctors */
		*crtbegin.o(.ctors)
		*crtbegin?.o(.ctors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
		*(SORT(.ctors.*))
		*(.ctors)

		/* .dtors */
 		*crtbegin.o(.dtors)
 		*crtbegin?.o(.dtors)
 		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
 		*(SORT(.dtors.*))
 		*(.dtors)

		KEEP(*(.eh_frame*))

	} > ROM
	
	.ARM.extab :
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > ROM

	.ARM.exidx :
	{
		__exidx_start = .;
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
		__exidx_end = .;
	} > ROM

	__etext = .;

	.data : ALIGN(4)
	{
		__data_start__ = .;
		*(.data*)
		. = ALIGN(4);
		__data_end__ = .;
	} > RAM AT > ROM

	__data_init_rom__ = LOADADDR(.data);

	.bss : ALIGN(4)
	{
		__bss_start__ = .;
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;
	} > RAM

	
	/DISCARD/ :
  	{
    	libc.a ( * )
    	libm.a ( * )
    	libgcc.a ( * )
	}

}











/* Library configurations */
GROUP(libgcc.a libc.a libm.a libnosys.a)

/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions FLASH and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   __exidx_start
 *   __exidx_end
 *   __copy_table_start__
 *   __copy_table_end__
 *   __zero_table_start__
 *   __zero_table_end__
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 *   __Vectors_End
 *   __Vectors_Size
 */

 