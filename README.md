# IYKEOS

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-x86-orange.svg)](https://en.wikipedia.org/wiki/X86)
[![Language](https://img.shields.io/badge/Language-C%2FAssembly-green.svg)](https://en.wikipedia.org/wiki/C_(programming_language))

A lightweight, educational 32-bit operating system for x86 architecture featuring a microkernel design, boot menu, multiple games, graphical user interface, command-line shell, and FAT16 filesystem support.

![IYKEOS Logo](https://via.placeholder.com/600x200/003399/FFFFFF?text=IYKEOS)

## Overview

IYKEOS is a hobby operating system developed for educational purposes. It demonstrates core operating system concepts including process management, memory management, filesystem operations, and device drivers. The OS boots from a hard disk image and provides both a text-mode shell and a graphical windowing environment.

**Version:** 0.3  
**Author:** Chihurum Fortune  
**Architecture:** x86 (32-bit Protected Mode)
**Design:** Microkernel (Kernel + OS split)

## Features

### Core System
- **Microkernel Architecture** - Minimal kernel loads OS.BIN separately
- **32-bit Protected Mode** kernel with GDT and basic segmentation
- **FAT16 Filesystem** with full read/write support
- **PS/2 Keyboard and Mouse** drivers with interrupt handling
- **VGA Text Mode** (80x25) and basic graphics mode support
- **Real-Time Clock (RTC)** integration for time/date
- **Program Loading** - Execute .BIN and .ike files
- **Virtual Graphics (vgraphics)** - Double-buffered, flicker-free rendering

### Boot Menu
On startup, choose from 4 options:
1. **Boot into FORTUNE_OS CLI** - Command-line shell
2. **Boot into FORTUNE_OS Graphics** - Launch GUI directly
3. **Check out Kernel Games** - Game launcher
4. **Quit/Exit** - Shutdown system

### Command Line Interface
The shell provides a Unix-like command environment:

| Command | Description |
|---------|-------------|
| `help` | Display available commands |
| `ls` | List files in current directory |
| `cd <dir>` | Change directory |
| `mkdir <dir>` | Create directory |
| `rmdir <dir>` | Remove directory |
| `cat <file>` | Display file contents |
| `touch <file>` | Create empty file |
| `rm <file>` | Delete file |
| `cp <src> <dst>` | Copy file |
| `mv <src> <dst>` | Move/rename file |
| `edit <file>` | Open file in text editor |
| `basic <file>` | Run BASIC program |
| `window` | Launch GUI |
| `gfx` | Switch to graphics mode |
| `text` | Return to text mode |
| `clear` | Clear screen |
| `date` / `time` | Show current date/time |
| `uptime` | Show CPU cycles since boot |
| `reboot` | Restart system |
| `exit` / `shutdown` | Halt system |

### Graphical User Interface
The GUI provides a desktop environment with:

- **Window Manager** - Full-screen and windowed applications
- **Start Menu** with Programs submenu
- **Desktop Icons** for quick access to applications
- **Mouse Support** - Point and click interface
- **Multiple Applications:**
  - **Files** - File manager with directory navigation
  - **Edit** - Text editor with New/Open/Save
  - **Notes** - Simple notepad
  - **Calc** - Calculator with basic operations
  - **Web** - Web browser placeholder
  - **Music** - Music player placeholder
  - **Video** - Video player placeholder
  - **Config** - System information display
  - **Help** - Documentation viewer

## Architecture

### Boot Process
```
BIOS → boot.asm (MBR) → stage2.asm (VBR) → kernel_entry_low.asm → kernel_32_low.c → OS.BIN → os_32_low.c
```

1. **boot.asm** - Master Boot Record (MBR) loads Stage 2
2. **stage2.asm** - Volume Boot Record (VBR) sets up 32-bit mode
3. **kernel_entry_low.asm** - Assembly entry point, initializes segments
4. **kernel_32_low.c** - Minimal kernel with boot menu (CLI/Graphics/Games/Exit)
5. **OS.BIN** - Loaded from disk at 0xB00000, contains full OS
6. **os_32_low.c** - Main OS with shell, GUI, and game launcher

### Memory Layout
```
0x00000000 - 0x0009FFFF  : Free memory (640KB)
0x000A0000 - 0x000BFFFF  : Video memory (VGA)
0x000C0000 - 0x000C7FFF  : VGA font buffer
0x000C8000 - 0x000CFFFF  : Screen buffer
0x00010000 - 0x0001FFFF  : Boot sector area
0x00020000 - 0x000FFFFF  : Kernel space
0x00100000 - 0x00FFFFFF  : Extended memory
0xB8000                 : VGA text buffer (80x25)
0xA0000                 : VGA graphics buffer
```

### System APIs
The OS exposes APIs through a structure at `0x5F0F0`:

- **Screen Functions:** `screen_clear()`, `screen_print()`, `screen_putc()`
- **Graphics Functions:** `graphics_init()`, `draw_pixel()`, `put_char()`, `draw_window()`
- **Virtual Graphics (vgraphics):** `vgraphics_init()`, `vgraphics_clear()`, `vgraphics_put_char()`, `vgraphics_put_string()`, `vgraphics_draw_box()`, `vgraphics_repaint()`
- **File Functions:** `fat16_file_load()`, `fat16_file_save()`, `fat16_create_file()`, `fat16_delete_file()`
- **Input Functions:** `keyboard_getchar()` (non-blocking), `keyboard_read()` (blocking), `mouse_handler()`
- **Utility Functions:** `strlen()`, `strcpy()`, `strcmp()`, `memcpy()`, `memset()`

## Project Structure

```
iykeos/
├── Makefile              # Build system
├── build-hdd.sh          # Disk image creation script
├── linker.ld             # Kernel linker script
├── linker2.ld            # Program linker script
├── lib/                  # OS Libraries
│   ├── graphics.c/h      # VGA graphics and drawing
│   ├── screen.c/h        # Screen management
│   ├── keyboard.c/h      # Keyboard driver
│   ├── file.c/h          # FAT16 filesystem
│   ├── disk.c/h          # Disk I/O
│   ├── strings.c/h       # String utilities
│   ├── memory_os.c/h     # Memory management
│   └── print.c/h         # Printing utilities
├── src/
│   ├── bootloader/
│   │   ├── boot.asm      # MBR (512 bytes)
│   │   └── stage2.asm    # Stage 2 bootloader
│   ├── kernel/
│   │   ├── kernel_entry_low.asm  # Kernel entry point
│   │   ├── kernel_32_low.c       # Early kernel
│   │   └── os_32_low.c           # Main OS kernel
│   └── programs/
│       ├── code_entry.asm        # Program entry stub
│       ├── HELLO.c               # Hello world demo
│       ├── EDITOR.c              # Text editor program
│       ├── BASIC.c               # BASIC interpreter
│       ├── gui.c                 # Basic GUI (legacy)
│       └── gui2.c                # Enhanced GUI with all features
└── build/                # Build artifacts
    ├── harddisk.img      # Final OS disk image
    ├── *.bin             # Compiled binaries
    └── *.elf             # ELF executables
```

## Building

### Prerequisites
- GCC (cross-compiler for i686-elf recommended)
- NASM (Netwide Assembler)
- GNU Make
- QEMU (for testing)
- Linux environment with sudo access

### Build Commands

```bash
# Build everything and create disk image
sudo make

# Build specific components
make compile-lib        # Compile libraries
make build-kernel       # Build kernel only
make build-programs     # Build user programs
make clean              # Clean build artifacts

# Build manually
make build/gui2.bin     # Build enhanced GUI
```

The build process:
1. Compiles libraries (`lib/*.c` → `lib/out/*.o`)
2. Assembles bootloader (`boot.asm`, `stage2.asm`)
3. Compiles kernel (`kernel_32_low.c`, `os_32_low.c`)
4. Links kernel with libraries
5. Compiles user programs (HELLO, EDITOR, BASIC, GUI)
6. Creates 32MB FAT16 disk image
7. Copies files to disk image
8. Launches QEMU for testing

## Usage

### Running in QEMU
```bash
sudo make
```

This builds the OS and launches it in QEMU with the hard disk image.

### Running on Real Hardware
**⚠️ Warning: This will overwrite your disk!**

```bash
# Write disk image to USB drive (replace /dev/sdX with your device)
sudo dd if=build/harddisk.img of=/dev/sdX bs=4M status=progress

# Or use a tool like Etcher for a safer GUI approach
```

### Boot Options
The OS can boot in two modes:

1. **Shell Mode** (default from CLI) - Text-based command line
2. **GUI Mode** (default on boot) - Graphical desktop environment

From the shell, type `window` to launch the GUI. From the GUI Start Menu, select "Shell" to return to the command line.

## Technical Details

### Bootloader
- **MBR (boot.asm):** 512 bytes, loads Stage 2 from disk
- **VBR (stage2.asm):** Sets up GDT, switches to 32-bit protected mode, loads kernel

### Kernel
- Written in C with inline assembly
- No standard library (freestanding environment)
- Custom memory allocator (`simple_malloc`)
- Interrupt-driven keyboard and mouse
- Custom FAT16 implementation

### Filesystem
- FAT16 with 512-byte sectors
- Supports long file names (8.3 format)
- Directory creation and navigation
- File read/write operations

### Graphics
- VGA Mode 0x03 (80x25 text mode) for shell
- Custom 80x25 text mode for GUI
- Double buffering support
- Windowing system with borders and titles

### Programs
Programs are compiled as flat binaries and loaded at `0xD00000`. They use the OS API structure passed during initialization.

Example program structure:
```c
void main(void) {
    // API is automatically set up at 0x5F0F0
    os_api_t* os_api = (os_api_t*)0x5F0F0;
    
    // Use OS functions
    os_api->print_shell("Hello from program!\n");
    os_api->keyboard_read();  // Wait for key
}
```

## Development

### Adding New Programs
1. Create `src/programs/YOURAPP.c`
2. Implement `main()` function
3. Add build rule to `Makefile`
4. Copy binary in `build-hdd.sh`

### Adding System Calls
1. Add function pointer to `os_api_t` structure in `os_32_low.c`
2. Implement function in `os_32_low.c`
3. Assign in `init_api()` function
4. Declare in program headers

### Debugging
- Use `os_api->debug_print()` for serial/debug output
- Check QEMU monitor (Ctrl+Alt+2) for low-level debugging
- Enable verbose boot messages in stage2.asm

## Known Limitations

- No multitasking (single-threaded)
- No virtual memory (physical memory only)
- Limited to 80x25 text mode for GUI
- No networking stack
- Basic memory management (no heap expansion)
- PS/2 devices only (no USB support)
- No sound support

## Future Enhancements

- [ ] Multitasking and process scheduling
- [ ] Virtual memory and paging
- [ ] Higher resolution graphics modes
- [ ] Networking stack (TCP/IP)
- [ ] USB HID support
- [ ] Sound driver (PC Speaker/AC97)
- [ ] More sophisticated GUI toolkit
- [ ] Package manager for programs

## Contributing

This is an educational project. Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Follow existing code style
4. Test thoroughly in QEMU
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- OSDev Wiki (https://wiki.osdev.org) for invaluable resources
- NASM documentation for assembly reference
- The open-source OS community for inspiration

## Contact

**Author:** Chihurum Fortune  
**Project:** IYKEOS (Ike Operating System)  
**Version:** 0.2

---

*Built with passion for learning how operating systems work.*
