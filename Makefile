BUILD_DIR = ./build

C_OBJS = kernel/main.o \
		kernel/interrupt.o \
		kernel/syscall.o \
		kernel/debug.o \
		mm/bitmap.o \
		mm/addr_pool.o \
		mm/memory.o \
		userprog/tss.o \
		device/timer.o \
		device/console.o \
		device/keyboard.o \
		thread/thread.o \
		thread/sync.o \
		lib/kernel/list.o \
		lib/kernel/io.o \
		lib/stdio.o \
		lib/string.o

AS_OBJS = kernel/intr_entry_list.o \
		lib/kernel/print.o \
		thread/switch.o

CFLAGS = -m32 -fno-builtin -fno-stack-protector -Og

all: bochs/c.img

kernel/intr_entry_list.o: kernel/intr_entry_list.s
	nasm -f elf -o $@ kernel/intr_entry_list.s

lib/kernel/print.o: lib/kernel/print.s
	nasm -f elf -o $@ lib/kernel/print.s

thread/switch.o: thread/switch.s
	nasm -f elf -o $@ thread/switch.s

# bins
$(BUILD_DIR)/mbr.bin: $(BUILD_DIR) boot/mbr.s
	nasm -I boot/include/ -o $@ boot/mbr.s

$(BUILD_DIR)/loader.bin: $(BUILD_DIR) boot/loader.s
	nasm -I boot/include/ -o $@ boot/loader.s

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR) $(C_OBJS) $(AS_OBJS)
	ld -m elf_i386 -Ttext 0xc0001500 -e main -o $@ $(C_OBJS) $(AS_OBJS)

bochs/c.img: $(BUILD_DIR)/mbr.bin $(BUILD_DIR)/loader.bin $(BUILD_DIR)/kernel.bin
	dd if=$(BUILD_DIR)/mbr.bin of=$@ bs=512 count=1 conv=notrunc
	dd if=$(BUILD_DIR)/loader.bin of=$@ bs=512 count=4 seek=1 conv=notrunc
	dd if=$(BUILD_DIR)/kernel.bin of=$@ bs=512 count=200 seek=9 conv=notrunc

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	-rm $(C_OBJS) $(AS_OBJS)
	-rm -rf $(BUILD_DIR)

.PHONY: all clean