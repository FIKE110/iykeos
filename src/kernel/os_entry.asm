[bits 32]

global _start
extern os_main

section .text
_start:
    ; Directly call the OS main function
    call os_main
    ret
