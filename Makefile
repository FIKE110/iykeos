CC      := gcc
LD      := ld
NASM    := nasm
OBJCOPY := objcopy
QEMU    := qemu-system-x86_64

SRC_DIR      := .
BOOT_DIR     := $(SRC_DIR)/src/bootloader
KERNEL_DIR   := $(SRC_DIR)/src/kernel
PROGS_DIR    := $(SRC_DIR)/src/programs
LIB_DIR      := $(SRC_DIR)/lib
BUILD_DIR    := $(SRC_DIR)/build
LIB_OUT_DIR  := $(LIB_DIR)/out

CFLAGS := -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin \
          -fno-exceptions -fno-stack-protector -fno-pic -O2 -Wall -Wextra -I$(LIB_DIR)

NASMFLAGS := -f elf32
NASM_BIN  := -f bin

LDFLAGS  := -m elf_i386 -T linker.ld
LDFLAGS2 := -m elf_i386 -T linker2.ld
LDFLAGS3 := -m elf_i386 -T linker_os.ld
LDFLAGS4 := -m elf_i386 -T linker_gui.ld
LDFLAGS5 := -m elf_i386 -T linker_game.ld

LIB_SRCS := $(LIB_DIR)/memory_os.c $(LIB_DIR)/disk.c $(LIB_DIR)/print.c \
            $(LIB_DIR)/file.c $(LIB_DIR)/strings.c $(LIB_DIR)/keyboard.c \
            $(LIB_DIR)/screen.c $(LIB_DIR)/graphics.c $(LIB_DIR)/vgraphics.c

LIB_OBJS := $(patsubst $(LIB_DIR)/%.c,$(LIB_OUT_DIR)/%.o,$(LIB_SRCS))

BOOT_BIN     := $(BUILD_DIR)/boot.bin
STAGE2_BIN   := $(BUILD_DIR)/stage2.bin
KERNEL_ELF   := $(BUILD_DIR)/kernel_low.elf
KERNEL_BIN   := $(BUILD_DIR)/kernel_low.bin
DISK_IMG     := $(BUILD_DIR)/harddisk.img

.PHONY: all clean compile-lib build-kernel build-programs test-os-build dirs

all: test-os-build

dirs:
	@mkdir -p $(BUILD_DIR) $(LIB_OUT_DIR)

compile-lib: dirs $(LIB_OBJS)
	@echo "Library compilation complete"

$(LIB_OUT_DIR)/%.o: $(LIB_DIR)/%.c
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(LIB_OUT_DIR)/vgraphics.o: $(LIB_DIR)/vgraphics.c
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BOOT_BIN): $(BOOT_DIR)/boot.asm | dirs
	@echo "  NASM    $<"
	@$(NASM) $(NASM_BIN) $< -o $@

$(STAGE2_BIN): $(BOOT_DIR)/stage2.asm | dirs
	@echo "  NASM    $<"
	@$(NASM) $(NASM_BIN) $< -o $@

$(BUILD_DIR)/kernel_entry_low.o: $(KERNEL_DIR)/kernel_entry_low.asm | dirs
	@echo "  NASM    $<"
	@$(NASM) $(NASMFLAGS) $< -o $@

$(BUILD_DIR)/kernel_32_low.o: $(KERNEL_DIR)/kernel_32_low.c
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/os_32_low.o: $(KERNEL_DIR)/os_32_low.c
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/os_entry.o: $(KERNEL_DIR)/os_entry.asm | dirs
	@echo "  NASM    $<"
	@$(NASM) $(NASMFLAGS) $< -o $@

build-kernel: compile-lib $(BUILD_DIR)/kernel_entry_low.o $(BUILD_DIR)/kernel_32_low.o
	@echo "  LD      kernel_low.elf"
	@$(LD) $(LDFLAGS) $(BUILD_DIR)/kernel_entry_low.o \
		$(BUILD_DIR)/kernel_32_low.o \
		-o $(KERNEL_ELF)
	@echo "  OBJCOPY kernel_low.bin"
	@$(OBJCOPY) -O binary $(KERNEL_ELF) $(KERNEL_BIN)

$(BUILD_DIR)/os_low.elf: $(BUILD_DIR)/os_entry.o $(BUILD_DIR)/os_32_low.o compile-lib
	@echo "  LD      os_low.elf"
	@$(LD) $(LDFLAGS3) $(BUILD_DIR)/os_entry.o \
		$(BUILD_DIR)/os_32_low.o \
		$(LIB_OBJS) -o $@

$(BUILD_DIR)/os.bin: $(BUILD_DIR)/os_low.elf
	@echo "  OBJCOPY os.bin"
	@$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/code_entry.o: $(PROGS_DIR)/code_entry.asm | dirs
	@echo "  NASM    $<"
	@$(NASM) $(NASMFLAGS) $< -o $@

$(BUILD_DIR)/hello_world_low.o: $(PROGS_DIR)/HELLO.c
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/editor_low.o: $(PROGS_DIR)/EDITOR.c
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/basic_low.o: $(PROGS_DIR)/BASIC.c
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/gui.o: $(PROGS_DIR)/gui.c
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/gui2.o: $(PROGS_DIR)/gui2.c
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/game_low.o: $(PROGS_DIR)/GAME.c
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@
	
$(BUILD_DIR)/snake.o: $(PROGS_DIR)/SNAKE.C
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/tetris.o: $(PROGS_DIR)/TETRIS.C
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/pong.o: $(PROGS_DIR)/PONG.C
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/breakout.o: $(PROGS_DIR)/BREAKOUT.c
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/snake2.o: $(PROGS_DIR)/SNAKE2.C
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/spaceinvaders.o: $(PROGS_DIR)/SPACEINVADERS.c
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/files.o: $(PROGS_DIR)/FILES.c
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/runner.o: $(PROGS_DIR)/RUNNER.c
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/edit.o: $(PROGS_DIR)/EDIT.c
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/hello.elf: $(BUILD_DIR)/code_entry.o $(BUILD_DIR)/hello_world_low.o
	@echo "  LD      hello.elf"
	@$(LD) $(LDFLAGS2) $^ -o $@

$(BUILD_DIR)/editor.elf: $(BUILD_DIR)/code_entry.o $(BUILD_DIR)/editor_low.o
	@echo "  LD      editor.elf"
	@$(LD) $(LDFLAGS2) $^ -o $@

$(BUILD_DIR)/basic.elf: $(BUILD_DIR)/code_entry.o $(BUILD_DIR)/basic_low.o
	@echo "  LD      basic.elf"
	@$(LD) $(LDFLAGS2) $^ -o $@

$(BUILD_DIR)/gui.elf: $(BUILD_DIR)/code_entry.o $(BUILD_DIR)/gui.o
	@echo "  LD      gui.elf"
	@$(LD) $(LDFLAGS4) $^ -o $@

$(BUILD_DIR)/gui2.elf: $(BUILD_DIR)/code_entry.o $(BUILD_DIR)/gui2.o
	@echo "  LD      gui2.elf"
	@$(LD) $(LDFLAGS4) $^ -o $@

$(BUILD_DIR)/game.elf: $(BUILD_DIR)/code_entry.o $(BUILD_DIR)/game_low.o
	@echo "  LD      game.elf"
	@$(LD) $(LDFLAGS5) $^ -o $@

$(BUILD_DIR)/snake.elf: $(BUILD_DIR)/code_entry.o $(BUILD_DIR)/snake.o
	@echo "  LD      snake.elf"
	@$(LD) $(LDFLAGS2) $^ -o $@

$(BUILD_DIR)/tetris.elf: $(BUILD_DIR)/code_entry.o $(BUILD_DIR)/tetris.o
	@echo "  LD      tetris.elf"
	@$(LD) $(LDFLAGS2) $^ -o $@

$(BUILD_DIR)/pong.elf: $(BUILD_DIR)/code_entry.o $(BUILD_DIR)/pong.o
	@echo "  LD      pong.elf"
	@$(LD) $(LDFLAGS2) $^ -o $@

$(BUILD_DIR)/breakout.elf: $(BUILD_DIR)/code_entry.o $(BUILD_DIR)/breakout.o
	@echo "  LD      breakout.elf"
	@$(LD) $(LDFLAGS2) $^ -o $@

$(BUILD_DIR)/snake2.elf: $(BUILD_DIR)/code_entry.o $(BUILD_DIR)/snake2.o
	@echo "  LD      snake2.elf"
	@$(LD) $(LDFLAGS2) $^ -o $@

$(BUILD_DIR)/spaceinvaders.elf: $(BUILD_DIR)/code_entry.o $(BUILD_DIR)/spaceinvaders.o
	@echo "  LD      spaceinvaders.elf"
	@$(LD) $(LDFLAGS2) $^ -o $@

$(BUILD_DIR)/files.elf: $(BUILD_DIR)/code_entry.o $(BUILD_DIR)/files.o
	@echo "  LD      files.elf"
	@$(LD) $(LDFLAGS2) $^ -o $@

$(BUILD_DIR)/runner.elf: $(BUILD_DIR)/code_entry.o $(BUILD_DIR)/runner.o
	@echo "  LD      runner.elf"
	@$(LD) $(LDFLAGS5) $^ -o $@

$(BUILD_DIR)/edit.elf: $(BUILD_DIR)/code_entry.o $(BUILD_DIR)/edit.o
	@echo "  LD      edit.elf"
	@$(LD) $(LDFLAGS2) $^ -o $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	@echo "  OBJCOPY $(notdir $@)"
	@$(OBJCOPY) -O binary $< $@

build-os: build-kernel $(BUILD_DIR)/os.bin
	@echo "OS binary build complete"

build-programs: build-os $(BUILD_DIR)/code_entry.o
	@$(MAKE) $(BUILD_DIR)/hello.bin $(BUILD_DIR)/editor.bin $(BUILD_DIR)/basic.bin $(BUILD_DIR)/gui.bin $(BUILD_DIR)/gui2.bin $(BUILD_DIR)/game.bin $(BUILD_DIR)/snake.bin $(BUILD_DIR)/tetris.bin $(BUILD_DIR)/pong.bin $(BUILD_DIR)/breakout.bin $(BUILD_DIR)/snake2.bin $(BUILD_DIR)/spaceinvaders.bin $(BUILD_DIR)/files.bin $(BUILD_DIR)/runner.bin $(BUILD_DIR)/edit.bin

test-os-build: build-programs $(BOOT_BIN) $(STAGE2_BIN) $(KERNEL_BIN)
	@bash ./build-hdd.sh
	@$(QEMU) -hda $(DISK_IMG)

flash: test-os-build
	@echo "  FLASH   $(DISK_IMG) -> /dev/sdb"
	@sudo dd if=$(DISK_IMG) of=/dev/sdb bs=4M status=progress
	@sync
	@echo "  Flash complete! You can now boot from /dev/sdb"

clean:
	@rm -rf $(BUILD_DIR)/*.o $(BUILD_DIR)/*.elf $(BUILD_DIR)/*.bin $(BUILD_DIR)/*.img
	@rm -rf $(LIB_OUT_DIR)/*.o

help:
	@echo "IYKEOS Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all              - Build and run OS (default)"
	@echo "  compile-lib      - Compile OS libraries"
	@echo "  build-kernel     - Build kernel"
	@echo "  build-os         - Build OS binary"
	@echo "  build-programs   - Build user programs"
	@echo "  test-os-build    - Build full OS and run in QEMU"
	@echo "  flash            - Write harddisk.img to /dev/sdb (flash drive)"
	@echo "  clean            - Remove build artifacts"
	@echo "  help             - Show this help"