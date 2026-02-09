ENTRY(_start)

SECTIONS {
  . =  0x01000000 ; 
    /* Start at 13MB — match the OS loader */

  .text : {
    *(.text*)
  }

  .rodata : {
    *(.rodata*)
  }

  .data : {
    *(.data*)
  }

  .bss : {
    *(.bss*)
    *(COMMON)
  }
}