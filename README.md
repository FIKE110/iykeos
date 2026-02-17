# IYKEOS

[![Platform](https://img.shields.io/badge/Platform-x86-orange.svg)](https://en.wikipedia.org/wiki/X86)
[![Language](https://img.shields.io/badge/Language-C%2FAssembly-green.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Version](https://img.shields.io/badge/Version-0.3-purple.svg)]()

A lightweight, educational 32-bit operating system for x86 architecture featuring a microkernel design, boot menu, multiple arcade games, graphical user interface, command-line shell, FAT16 filesystem support, and a web-based emulator.

![IYKEOS Logo](https://via.placeholder.com/600x200/003399/FFFFFF?text=IYKEOS)

## Overview

IYKEOS is a hobby operating system developed for educational purposes. It demonstrates core operating system concepts including process management, memory management, filesystem operations, device drivers, and game development. The OS boots from a hard disk image and provides both a text-mode shell and a graphical windowing environment with multiple built-in games.

**Version:** 0.3  
**Author:** Chihurum Fortune  
**Architecture:** x86 (32-bit Protected Mode)  
**Design:** Microkernel (Kernel + OS split)

## Features

### Core System
- **Microkernel Architecture** - Minimal kernel loads OS.BIN separately for modular design
- **32-bit Protected Mode** kernel with GDT (Global Descriptor Table) and basic segmentation
- **FAT16 Filesystem** with full read/write support, directory management, and file operations
- **PS/2 Keyboard and Mouse** drivers with interrupt-driven input handling
- **VGA Graphics** - Text mode (80x25) and graphics mode support with color capabilities
- **Real-Time Clock (RTC)** integration for accurate time and date display
- **Program Loading** - Dynamic execution of .BIN and .ike executable files
- **Virtual Graphics (vgraphics)** - Double-buffered, flicker-free rendering system
- **Memory Management** - Custom memory allocator with simple_malloc implementation

### Boot Menu
On startup, users are presented with an interactive boot menu:

1. **Boot into FORTUNE_OS CLI** - Command-line shell interface
2. **Boot into FORTUNE_OS Graphics** - Launch GUI desktop directly
3. **Check out Kernel Games** - Arcade game launcher with multiple titles
4. **Quit/Exit** - Safe system shutdown

### Games Collection

IYKEOS includes several fully-functional arcade games built natively for the OS:

| Game | Description | Controls |
|------|-------------|----------|
| **Space Invaders** | Classic arcade shooter with alien formations, bunkers, and scoring | Arrow keys to move, Space to shoot |
| **Tetris** | Block-stacking puzzle game with piece preview and line clearing | Arrow keys to move/rotate, Down to drop faster |
| **Snake** | Classic snake game with food collection and growing tail | Arrow keys to change direction |
| **Snake II** | Enhanced version with improved graphics and gameplay | Arrow keys to control |
| **Pong** | Two-player paddle and ball game | Player 1: W/S, Player 2: Up/Down arrows |
| **Breakout** | Brick-breaking paddle game | Arrow keys to move paddle, Space to launch ball |

### Command Line Interface

The shell provides a Unix-like command environment with the following commands:

| Command | Description |
|---------|-------------|
| `help` | Display available commands with descriptions |
| `ls` | List files in current directory with details |
| `cd <dir>` | Change directory (supports relative paths) |
| `mkdir <dir>` | Create new directory |
| `rmdir <dir>` | Remove empty directory |
| `cat <file>` | Display file contents on screen |
| `touch <file>` | Create empty file or update timestamp |
| `rm <file>` | Delete file from filesystem |
| `cp <src> <dst>` | Copy file to new location |
| `mv <src> <dst>` | Move or rename file |
| `edit <file>` | Open file in built-in text editor |
| `basic <file>` | Run BASIC interpreter program |
| `game` | Launch game menu directly |
| `window` | Launch GUI desktop environment |
| `gfx` | Switch to graphics mode |
| `text` | Return to text mode |
| `clear` | Clear screen and reset cursor |
| `date` / `time` | Show current system date/time from RTC |
| `uptime` | Show CPU cycles elapsed since boot |
| `reboot` | Restart the system |
| `exit` / `shutdown` | Halt system safely |

### Graphical User Interface

The GUI provides a complete desktop environment with:

- **Window Manager** - Supports full-screen and windowed applications with borders
- **Start Menu** - Hierarchical menu system with Programs submenu
- **Desktop Icons** - Clickable shortcuts for quick application access
- **Mouse Support** - Full point-and-click interface with cursor tracking
- **Task Bar** - Shows running applications and system status

**Built-in Applications:**

| Application | Function |
|-------------|----------|
| **Files** | File manager with directory navigation and file operations |
| **Edit** | Full-featured text editor with New, Open, Save, and Exit |
| **Notes** | Simple notepad for quick text entry |
| **Calc** | Calculator supporting basic arithmetic operations |
| **Web** | Web browser framework (placeholder for future networking) |
| **Music** | Music player interface (placeholder for audio support) |
| **Video** | Video player interface (placeholder for multimedia) |
| **Config** | System information and configuration display |
| **Help** | Built-in documentation viewer |

## Architecture

### Boot Process

```
BIOS → boot.asm (MBR) → stage2.asm (VBR) → kernel_entry_low.asm → kernel_32_low.c → OS.BIN → os_32_low.c
```

1. **boot.asm** - Master Boot Record (512 bytes), loads Stage 2 from active partition
2. **stage2.asm** - Volume Boot Record, configures GDT, switches to 32-bit protected mode
3. **kernel_entry_low.asm** - Assembly entry point, initializes segment registers
4. **kernel_32_low.c** - Early kernel with boot menu (CLI/Graphics/Games/Exit)
5. **OS.BIN** - Loaded from disk at 0xB00000, contains full OS implementation
6. **os_32_low.c** - Main OS with shell, GUI, filesystem, and game launcher

### Memory Layout

```
0x00000000 - 0x0009FFFF  : Low memory / Free RAM (640KB)
0x000A0000 - 0x000BFFFF  : Video memory (VGA graphics)
0x000B8000               : VGA text buffer (80x25 color text)
0x000C0000 - 0x000C7FFF  : VGA font buffer
0x000C8000 - 0x000CFFFF  : Screen buffer
0x00010000 - 0x0001FFFF  : Boot sector and stage2 area
0x00020000 - 0x000FFFFF  : Kernel space and stack
0x00100000 - 0x00FFFFFF  : Extended memory (up to 16MB)
0x00B00000               : OS.BIN load address
0x00D00000               : User programs load address
0x0005F0F0               : OS API structure address
```

### System APIs

Programs interact with the OS through an API structure located at `0x5F0F0`:

**Screen Functions:**
- `screen_clear()` - Clear text mode screen
- `screen_print()` - Print string to screen
- `screen_putc()` - Print single character

**Graphics Functions:**
- `graphics_init()` - Initialize graphics mode
- `draw_pixel()` - Draw individual pixel
- `put_char()` - Draw character at coordinates
- `draw_window()` - Draw window frame

**Virtual Graphics (vgraphics):**
- `vgraphics_init()` - Initialize double buffer
- `vgraphics_clear()` - Clear back buffer
- `vgraphics_put_char()` - Draw character to buffer
- `vgraphics_put_string()` - Draw string to buffer
- `vgraphics_draw_box()` - Draw filled rectangle
- `vgraphics_repaint()` - Copy buffer to screen

**File Functions:**
- `fat16_file_load()` - Read file from disk
- `fat16_file_save()` - Write file to disk
- `fat16_create_file()` - Create new file
- `fat16_delete_file()` - Delete file
- `fat16_create_dir()` - Create directory
- `fat16_remove_dir()` - Remove directory

**Input Functions:**
- `keyboard_getchar()` - Non-blocking key check
- `keyboard_read()` - Blocking key read
- `mouse_handler()` - Process mouse interrupts

**Utility Functions:**
- `strlen()`, `strcpy()`, `strcmp()` - String operations
- `memcpy()`, `memset()` - Memory operations
- `simple_malloc()` - Dynamic memory allocation

## Project Structure

```
iykeos/
├── Makefile                    # Build system with targets for all components
├── build-hdd.sh               # Disk image creation and file copying script
├── linker.ld                  # Kernel linker script
├── linker2.ld                 # User program linker script
├── linker_os.ld               # OS binary linker script
│
├── lib/                       # OS Libraries (statically linked)
│   ├── graphics.c/h          # VGA graphics primitives and drawing functions
│   ├── screen.c/h            # Screen management and text output
│   ├── keyboard.c/h          # PS/2 keyboard driver with scancode handling
│   ├── file.c/h              # FAT16 filesystem implementation
│   ├── disk.c/h              # Low-level disk I/O operations
│   ├── strings.c/h           # String manipulation utilities
│   ├── memory_os.c/h         # Memory management and allocator
│   ├── print.c/h             # Printing utilities and formatting
│   ├── vgraphics.c/h         # Virtual graphics (double buffering)
│   ├── io.c/h                # Port I/O operations (inb/outb)
│   └── syscall.h             # System call definitions
│   └── out/                  # Compiled library object files
│
├── src/
│   ├── bootloader/
│   │   ├── boot.asm          # MBR - Master Boot Record (512 bytes)
│   │   └── stage2.asm        # VBR - Volume Boot Record, 32-bit mode setup
│   │
│   ├── kernel/
│   │   ├── kernel_entry_low.asm   # Kernel assembly entry point
│   │   ├── kernel_32_low.c        # Early kernel with boot menu
│   │   ├── os_entry.asm           # OS binary entry point
│   │   └── os_32_low.c            # Main OS with shell, GUI, games
│   │
│   ├── programs/
│   │   ├── code_entry.asm         # Program entry stub for user apps
│   │   ├── HELLO.c                # Hello world demo program
│   │   ├── EDITOR.c               # Text editor application
│   │   ├── BASIC.c                # BASIC interpreter
│   │   ├── gui.c                  # Basic GUI (legacy version)
│   │   ├── gui2.c                 # Enhanced GUI with full features
│   │   ├── SPACEINVADERS.c        # Space Invaders game
│   │   ├── TETRIS.C               # Tetris puzzle game
│   │   ├── SNAKE.C                # Classic Snake game
│   │   ├── SNAKE2.C               # Enhanced Snake game
│   │   ├── PONG.C                 # Pong paddle game
│   │   ├── BREAKOUT.c             # Breakout brick game
│   │   └── GAME.c                 # Game menu/launcher
│   │
│   └── tests/
│       ├── kernel_entry_high.asm  # Test kernel entry
│       ├── kernel_32_high.c       # Test kernel code
│       └── kernel_test.c          # Kernel unit tests
│
├── assets/
│   └── txt/                     # Sample text files for testing
│       ├── hello.txt
│       ├── obi.txt
│       └── ...
│
├── web/                        # Browser-based emulator
│   ├── index.html              # Emulator interface with controls
│   ├── js/
│   │   └── libv86.js          # V86 x86 emulator library
│   ├── assets/
│   │   ├── v86.wasm           # WebAssembly emulator core
│   │   └── harddisk.img       # OS disk image for web
│   └── bios/
│       ├── seabios.bin        # SeaBIOS for emulator
│       └── vgabios.bin        # VGA BIOS for emulator
│
├── build/                      # Build artifacts (created during build)
│   ├── harddisk.img           # Final bootable disk image
│   ├── kernel_low.bin         # Compiled kernel binary
│   ├── os.bin                 # OS binary
│   ├── *.bin                  # Compiled user programs
│   └── *.elf                  # ELF executables for debugging
│
└── .vscode/                    # VS Code configuration
    └── settings.json
```

## Building

### Prerequisites

- **GCC** - Cross-compiler for i686-elf target (or native GCC with 32-bit support)
- **NASM** - Netwide Assembler (version 2.14 or higher)
- **GNU Make** - Build automation
- **QEMU** - System emulator for testing (recommended)
- **Linux environment** with sudo access (for disk image creation)
- **mtools** - For manipulating FAT filesystems

### Build Commands

```bash
# Full build - compile everything and create disk image, then launch QEMU
sudo make

# Build specific components
make compile-lib        # Compile all libraries
make build-kernel       # Build kernel binaries only
make build-programs     # Build user programs (games, editor, etc.)
make build-games        # Build only the games
make clean              # Clean all build artifacts

# Build individual programs
make build/spaceinvaders.bin    # Build Space Invaders
make build/tetris.bin           # Build Tetris
make build/snake.bin            # Build Snake
make build/gui2.bin             # Build enhanced GUI
```

### Build Process Details

The build process follows these steps:

1. **Library Compilation** - Compiles `lib/*.c` → `lib/out/*.o`
2. **Bootloader Assembly** - Assembles `boot.asm` and `stage2.asm` to binaries
3. **Kernel Compilation** - Compiles `kernel_32_low.c` and `os_32_low.c`
4. **Kernel Linking** - Links kernel with libraries using `linker.ld`
5. **User Programs** - Compiles HELLO, EDITOR, BASIC, GUI, and all games
6. **Disk Image Creation** - Creates 32MB FAT16 disk image
7. **File Installation** - Copies all binaries and assets to disk image
8. **QEMU Launch** - Starts emulator for testing (when using `sudo make`)

## Usage

### Running in QEMU

```bash
# Build and run automatically
sudo make

# Or manually
make
qemu-system-i386 -drive file=build/harddisk.img,format=raw -m 32
```

### Web Emulator

IYKEOS can run directly in your browser without any installation:

1. Open `web/index.html` in a modern web browser
2. The OS will boot automatically using the V86 x86 emulator
3. Use the control buttons to pause, reset, or save/load state
4. Full keyboard and mouse support through the emulator

**Web Emulator Features:**
- **Pause/Resume** - Stop and continue emulation
- **Reset** - Restart the OS
- **Save State** - Download current machine state
- **Load State** - Restore from saved state
- **Fullscreen** - Expand to full screen
- **Screenshot** - Capture screen image
- **Scale** - Adjust display size

### Running on Real Hardware

**⚠️ Warning: This will overwrite your disk! Only use on dedicated media.**

```bash
# Write disk image to USB drive (replace /dev/sdX with your device)
sudo dd if=build/harddisk.img of=/dev/sdX bs=4M status=progress

# Or use Etcher/Rufus for a safer GUI approach
```

### Boot Options

The OS can boot in multiple modes:

1. **Shell Mode** (Option 1) - Text-based command line with full filesystem access
2. **GUI Mode** (Option 2) - Graphical desktop with window manager
3. **Game Mode** (Option 3) - Direct access to arcade games

Switch between modes anytime: From shell, type `window` to launch GUI. From GUI, select "Shell" from the Start Menu to return to command line.

## Technical Details

### Bootloader

**MBR (boot.asm):**
- 512 bytes loaded by BIOS at 0x7C00
- Reads partition table to find active partition
- Loads Stage 2 bootloader from disk
- Uses BIOS interrupt 0x13 for disk access

**VBR (stage2.asm):**
- Sets up Global Descriptor Table (GDT) for protected mode
- Enables A20 line for full 32-bit addressing
- Switches CPU from 16-bit real mode to 32-bit protected mode
- Loads kernel from disk to memory
- Jumps to kernel entry point

### Kernel

- Written in C with inline assembly for hardware access
- Freestanding environment (no standard library)
- Custom memory allocator with `simple_malloc()`
- Interrupt-driven keyboard and mouse handling
- Custom FAT16 filesystem implementation from scratch
- Virtual graphics system with double buffering

### Filesystem

- FAT16 with 512-byte sectors
- Supports 8.3 format filenames
- Full directory creation and navigation
- File read/write/delete operations
- Directory entry management
- Cluster allocation and deallocation

### Graphics System

**Text Mode (Mode 0x03):**
- 80 columns x 25 rows
- 16 colors for foreground/background
- Direct VGA memory access at 0xB8000

**Graphics Mode:**
- Custom rendering system
- Virtual graphics with double buffering
- Window manager with borders and titles
- Mouse cursor rendering

### Game Engine

Games are implemented as standalone programs that:
- Use the OS API for input and rendering
- Run at fixed screen coordinates
- Implement their own game loops
- Handle collision detection and scoring
- Support keyboard controls through OS keyboard API

## Development

### Adding New Programs

1. Create source file: `src/programs/YOURAPP.c`
2. Implement `main()` function using OS API
3. Add build rule to `Makefile`:
   ```makefile
   build/yourapp.bin: src/programs/YOURAPP.c lib/out/graphics.o ...
       $(CC) $(CFLAGS) -c $< -o build/yourapp.o
       $(LD) -T linker2.ld -o build/yourapp.elf build/yourapp.o lib/out/*.o
       $(OBJCOPY) -O binary build/yourapp.elf build/yourapp.bin
   ```
4. Add to `build-hdd.sh` to copy to disk image

### Adding System Calls

1. Add function pointer to `os_api_t` structure in `os_32_low.c`
2. Implement function in `os_32_low.c` or appropriate library
3. Assign function in `init_api()` function
4. Declare in `lib/syscall.h` or program headers
5. Document in README

### Debugging Tips

- Use `os_api->debug_print()` for serial output debugging
- Check QEMU monitor (Ctrl+Alt+2) for low-level info
- Enable verbose boot messages in `stage2.asm`
- Use `build/*.elf` files with GDB for source-level debugging
- Add `-d int` to QEMU for interrupt logging

## Known Limitations

- **No Multitasking** - Single-threaded execution only
- **No Virtual Memory** - Physical memory addressing only (no paging)
- **Limited Graphics** - 80x25 text mode maximum for GUI
- **No Networking** - TCP/IP stack not implemented
- **Basic Memory Management** - Fixed heap size, no expansion
- **PS/2 Devices Only** - No USB keyboard/mouse support
- **No Audio** - No sound driver or PC speaker support
- **Limited File Size** - FAT16 2GB maximum partition size

## Future Roadmap

### Version 0.4 (In Progress)
- [ ] Improved multitasking with process scheduling
- [ ] Better memory management with paging support
- [ ] Higher resolution graphics modes (VESA)

### Version 0.5 (Planned)
- [ ] Virtual memory implementation
- [ ] Preemptive multitasking
- [ ] ELF executable support

### Version 1.0 (Long Term)
- [ ] TCP/IP networking stack
- [ ] USB HID support (keyboards and mice)
- [ ] Audio driver (PC Speaker, AC97, or Intel HD Audio)
- [ ] Sophisticated GUI toolkit with widgets
- [ ] Package manager for programs
- [ ] POSIX compatibility layer

## Contributing

This is an educational open-source project. Contributions are welcome!

### How to Contribute

1. Fork the repository on GitHub
2. Create a feature branch: `git checkout -b feature-name`
3. Follow existing code style and conventions
4. Add comments for complex code sections
5. Test thoroughly in QEMU before submitting
6. Submit a pull request with clear description

### Coding Standards

- Use 4 spaces for indentation (no tabs)
- Function names: `lowercase_with_underscores()`
- Constants: `UPPERCASE_WITH_UNDERSCORES`
- Types: `lowercase_t` for typedefs
- Comment complex algorithms and hardware interactions
- Keep functions focused and under 50 lines when possible

## Acknowledgments

- **OSDev Wiki** (https://wiki.osdev.org) - Invaluable resource for OS development
- **NASM Documentation** - Assembly language reference
- **V86 Emulator** - Browser-based x86 emulation (https://github.com/copy/v86)
- **SeaBIOS** - Open source BIOS implementation
- **The open-source OS community** - For inspiration and examples

## Contact

**Author:** Chihurum Fortune  
**Project:** IYKEOS (Ike Operating System)  
**Version:** 0.3  
**Repository:** https://github.com/username/iykeos

---

*Built with passion for learning how operating systems work. Every line of code is a step toward understanding the machine.*

**Happy Hacking! 🖥️**
