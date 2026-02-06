[bits 32]

global _start
extern kernel_main

section .text
_start:
    pop eax
    call kernel_main
    jmp $
  