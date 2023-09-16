MEMORY
{
  /* ram used by the FlexSPI boot program */
  BOOTTEXT (xrw):  ORIGIN = 0x20008000, LENGTH = 32K
  BOOTRAM  (xrw):  ORIGIN = 0x20004000, LENGTH = 16K
}

REGION_ALIAS("RAM", BOOTRAM);
REGION_ALIAS("TEXT", BOOTTEXT);

INCLUDE vihal_stack_predef.ld

ENTRY(soft_entry)

SECTIONS
{
  .flexspi_bootsector : ALIGN(4)
  {
    FILL(0xFF)
    KEEP(*(.flexspi_boot_cfg))
    . = 0x0F80;
    KEEP(*(.bootloader_header))
    . = 0x1000;
    KEEP(*(.flexspi_boot_ivt))
    . = 0x1020;
    KEEP(*(.flexspi_boot_data))
    . = 0x1040;
    KEEP(*(.rom_startup))
    . = 0x1100;
  } > BOOTTEXT

  .isr_vector : ALIGN(1024)
  {
    FILL(0xFF)
    __vectors_start = ABSOLUTE(.) ;
    KEEP(*(.isr_vector))
    __vectors_end = ABSOLUTE(.) ;

  } > TEXT

  .inits : ALIGN(16)
  {
    /* DATA COPY REGIONS */

    __data_regions_array_start = .;

    LONG(LOADADDR(.data));         LONG(ADDR(.data));         LONG(ADDR(.data)+SIZEOF(.data));

    __data_regions_array_end = .;

    /* DATA CLEAR REGIONS */

    __bss_regions_array_start = .;

    LONG(ADDR(.bss));         LONG(ADDR(.bss)+SIZEOF(.bss));

    __bss_regions_array_end = .;

  } > TEXT

  .resetentry : ALIGN(4)
  {
    KEEP(*(.resetentry))
    KEEP(*(.startup))
  } > TEXT

  .text : ALIGN(4)
  {
    INCLUDE vihal_text_section.ld

  } > TEXT
  
  /* .ARM.exidx is sorted, so has to go in its own output section.  */
  PROVIDE_HIDDEN (__exidx_start = .);
  .ARM.exidx :
  {
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
  } > TEXT
  PROVIDE_HIDDEN (__exidx_end = .);  

  . = ALIGN(4);
  _etext = .;
  __etext = .;

  .data : ALIGN(4)
  {
    *(.data_begin .data_begin.*)
    *(.data .data.*)
    *(.data_end .data_end.*)
    . = ALIGN(4);

  } > RAM AT > TEXT
  
	/* end of the bootloader image: */
	__image_end = LOADADDR(.data) + SIZEOF(.data);  
  
  .bss (NOLOAD) : ALIGN(4)
  {
    INCLUDE vihal_bss_section.ld
    
  } > RAM

  .noinit (NOLOAD) : ALIGN(4)
  {
    _noinit = .;
    *(.noinit .noinit.*)
    . = ALIGN(4) ;
    _end_noinit = .;
  } > RAM

  INCLUDE vihal_stack_heap.ld
  INCLUDE vihal_debug_sections.ld
}
