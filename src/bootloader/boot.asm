BITS 16
ORG 0x7C00

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov si, 0x7C00
    mov di, 0x0600
    mov cx, 256
    rep movsw

    jmp 0x0000:.relocated

.relocated:
    mov [boot_drive], dl

find_partition:
    mov si, partition_table
    mov cx, 4

.partition_loop:
    cmp byte [si], 0x80
    je found_bootable

    add si, 16
    loop .partition_loop

    mov si, msg_no_bootable
    call print_string
    jmp hang

found_bootable:
    mov eax, [si + 8]
    mov [dap_lba], eax

load_vbr:
    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, dap
    int 0x13
    jc disk_error

    jmp 0x0000:0x7C00

disk_error:
    mov si, msg_disk_error
    call print_string

hang:
    cli
    hlt
    jmp hang

print_string:
    mov ah, 0x0E
.char_loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .char_loop
.done:
    ret

boot_drive      db 0
msg_no_bootable db 'Error: No bootable partition found!', 0x0D, 0x0A, 0
msg_disk_error  db 'Error: Disk read failed!', 0x0D, 0x0A, 0

dap:
    db 0x10
    db 0
    dw 1
    dw 0x7C00
    dw 0x0000
dap_lba:
    dd 0
    dd 0

times 0x1BE - ($ - $$) db 0

partition_table:
    db 0x80
    db 0x01
    db 0x01
    db 0x00
    db 0x06
    db 0xFE
    db 0xFF
    db 0xFF
    dd 1
    dd 64000

    times 16 * 3 db 0

dw 0xAA55
