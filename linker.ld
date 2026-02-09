/*
 * LINKER SCRIPT: kernel.ld
 * This defines the memory layout of your kernel.
 */

OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)            

/* Kernel starts at the linear address 0x90000, as loaded by stage2.asm. */
KERNEL_LOAD_ADDRESS = 0x90000;

SECTIONS
{
    /* .text section (Code and Read-Only Data) */
    .text KERNEL_LOAD_ADDRESS : 
    {
        KEEP(*(.start))       /* Entry point defined in assembly */
        *(.text)              /* Executable code */
        *(.rodata*)           /* Read-only data (strings) */
        . = ALIGN(0x1000);
    }

    /* .data section (Initialized Global Variables) */
    .data : 
    {
        *(.data)
        . = ALIGN(0x1000);
    }

    /* .bss section (Uninitialized Global Variables - Must be zeroed) */
    .bss : 
    {
        /* --- SYMBOL DEFINITION IS HERE --- */
        __bss_start = .;       /* Define __bss_start as the current address (start of .bss) */
        *(.bss)                /* Include all uninitialized variables */
        __bss_end = .;         /* Define __bss_end as the current address (end of .bss) */
        /* ---------------------------------- */
        . = ALIGN(0x1000);
    }

    /DISCARD/ : 
    {
        *(.eh_frame)
        *(.note.gnu.build-id)
        *(.comment)
    }
}

