section .start
global _start
extern kernel_main
extern __bss_start
extern __bss_end

_start:
    call zero_bss
    call kernel_main
    mov edi, 0x000B8000
    mov byte [edi], 'P'

.halt:
    hlt
    jmp .halt

zero_bss:
    mov edx, __bss_end
    mov ecx, __bss_start
    sub edx, ecx
    mov edi, ecx
    xor eax, eax
    mov ecx, edx
    shr ecx, 2
    cld
    rep stosd
    ret