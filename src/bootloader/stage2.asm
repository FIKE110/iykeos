BITS 16
ORG 0x7C00

jmp start

; BPB used by mkfs.fat and kernel
OEMLabel        db "IYKEOS   "
BytesPerSector  dw 512
SectorsPerCluster db 4
ReservedSectors dw 128
NumFATs         db 2
RootEntries     dw 512
TotalSectors16  dw 0
MediaDescriptor db 0xF8
SectorsPerFAT   dw 63
SectorsPerTrack dw 32
NumHeads        dw 64
HiddenSectors   dd 1
TotalSectors32  dd 64000
DriveNumber     db 0x80

start:
    xor bx,bx
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov [DriveNumber], dl

    mov si, msg_hello
    call print_string

    mov si, msg_press_key
    call print_string
    xor ax, ax
    int 0x16

    ; Load Kernel (LBA 2+) to 0x90000
    mov ax, 0x9000
    mov es, ax
    xor bx, bx

    mov ah, 0x42
    mov dl, [DriveNumber]
    mov si, dap
    int 0x13
    jc disk_error

    cli

    ; Enable A20 Line (Fast A20)
    in al, 0x92
    or al, 2
    out 0x92, al

    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:init_pm

BITS 32
init_pm:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov ebp, 0x90000
    mov esp, ebp

    jmp 0x90000

BITS 16
disk_error:
    mov si, msg_error
    call print_string
hang:
    cli
    hlt
    jmp hang

print_string:
    mov ah, 0x0E
.loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
.done:
    ret

msg_hello db 'Hello World from Stage 2!', 0x0D, 0x0A, 0
msg_press_key db 'Press any key to load kernel...', 0x0D, 0x0A, 0
msg_error db 'Kernel Load Error', 0x0D, 0x0A, 0

dap:
    db 0x10, 0
    dw 127
    dw 0x0000
    dw 0x9000
    dd 2
    dd 0

gdt_start:
    dq 0x0
gdt_code:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00
gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510 - ($ - $$) db 0
dw 0xAA55
