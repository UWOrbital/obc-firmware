INCLUDE memory.ld

ENTRY(_c_int00)

_estack = 0x8040000;    /* end of 256K RAM */

_Min_Heap_Size = 0x400;      /* required amount of heap  */

/* The size of the app (used for erasing the app from flash) */
__APP_IMAGE_TOTAL_SECTION_SIZE = LENGTH(APP_VECTORS) + LENGTH(APP_KERNEL) + LENGTH(APP_FLASH) + LENGTH(APP_METADATA);

SECTIONS
{

  /* The ISR vector goes first into RAM */
  .intvecs :
  {
    . = ALIGN(4);
    KEEP(*(.intvecs)) /* ISR vector */
    . = ALIGN(4);
  } >APP_VECTORS

  .metadata :
  {
    KEEP(*(.metadata))
    . = ALIGN(32);
  } >APP_METADATA

   /* FreeRTOS Kernel in protected region of Flash */
  .kernelTEXT :
  {
    . = ALIGN(4);
    KEEP(*(.kernelTEXT))
    . = ALIGN(4);
  } >APP_KERNEL

  /* The program code and other data goes into RAM */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >APP_FLASH

  /* Constant data goes into RAM */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >APP_FLASH

   .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >RAM
    .ARM : {
    __exidx_start = .;
      *(.ARM.exidx*)
      __exidx_end = .;
    } >APP_FLASH

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >APP_FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >APP_FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >APP_FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM AT> APP_FLASH

  .kernelBSS :
  {
    . = ALIGN(4);
    _kernel_bss = .;
    *(.kernelBSS)
    *(.kernelBSS*)

    _ekernel_bss = .;         /* define a global symbol at bss end */
    __kernel_bss_end__ = _ekernel_bss;
  } >KRAM

  /* FreeRTOS Kernel data in protected region of RAM */
  /* used by the startup to initialize data */
  _ksidata = LOADADDR(.kernelBSS);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    _ksdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _kedata = .;        /* define a global symbol at data end */
  } >RAM AT> APP_FLASH
 
  .crc :
  {
    . = ALIGN(8);
    __crc_addr = .;
  } > APP_FLASH

  .kernelHEAP : {} >RAM

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM

  PROVIDE ( end = _ebss );
  PROVIDE ( _end = _ebss );

  /* MEMORY_bank1 section, code must be located here explicitly            */
  /* Example: extern int foo(void) __attribute__ ((section (".mb1text"))); */
  .memory_b1_text :
  {
    *(.mb1text)        /* .mb1text sections (code) */
    *(.mb1text*)       /* .mb1text* sections (code)  */
    *(.mb1rodata)      /* read-only data (constants) */
    *(.mb1rodata*)
  } >MEMORY_B1

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
