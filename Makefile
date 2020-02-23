BUILD_DIR = ./build
OBJS = $(BUILD_DIR)/kernel/main.o \
	$(BUILD_DIR)/kernel/interrupt.o \
	$(BUILD_DIR)/kernel/bitmap.o \
	$(BUILD_DIR)/kernel/memory.o \
	$(BUILD_DIR)/kernel/intr_entry_list.o \
	$(BUILD_DIR)/device/timer.o \
	$(BUILD_DIR)/lib/kernel/print.o \
	$(BUILD_DIR)/lib/string.o
CFLAGS = -fno-builtin -Og

all: bochs/c.img

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/mbr.bin: $(BUILD_DIR) boot/mbr.s
	nasm -I boot/include/ -o $@ boot/mbr.s

$(BUILD_DIR)/loader.bin: $(BUILD_DIR) boot/loader.s
	nasm -I boot/include/ -o $@ boot/loader.s

$(BUILD_DIR)/lib/kernel/print.o: $(BUILD_DIR) lib/kernel/print.s
	nasm -f elf -o $@ lib/kernel/print.s

$(BUILD_DIR)/lib/string.o: $(BUILD_DIR) lib/string.c
	gcc -m32 -c $(CFLAGS) -o $@ lib/string.c

# kernel modules
$(BUILD_DIR)/kernel/intr_entry_list.o: $(BUILD_DIR) kernel/intr_entry_list.s
	nasm -f elf -o $@ kernel/intr_entry_list.s

$(BUILD_DIR)/kernel/interrupt.o: $(BUILD_DIR) kernel/interrupt.c
	gcc -m32 -c -fno-stack-protector $(CFLAGS) -o $@ kernel/interrupt.c

$(BUILD_DIR)/kernel/bitmap.o: $(BUILD_DIR) kernel/bitmap.c
	gcc -m32 -c $(CFLAGS) -o $@ kernel/bitmap.c

$(BUILD_DIR)/kernel/memory.o: $(BUILD_DIR) kernel/memory.c
	gcc -m32 -c $(CFLAGS) -o $@ kernel/memory.c

# device modules
$(BUILD_DIR)/device/timer.o: $(BUILD_DIR) device/timer.c
	gcc -m32 -c $(CFLAGS) -o $@ device/timer.c

# main
$(BUILD_DIR)/kernel/main.o: $(BUILD_DIR) kernel/main.c
	gcc -m32 -c $(CFLAGS) -o $@ kernel/main.c

$(BUILD_DIR)/kernel.bin: $(OBJS)
	ld -m elf_i386 -Ttext 0xc0001500 -e main -o $@ $(OBJS)

bochs/c.img: $(BUILD_DIR)/mbr.bin $(BUILD_DIR)/loader.bin $(BUILD_DIR)/kernel.bin
	dd if=$(BUILD_DIR)/mbr.bin of=$@ bs=512 count=1 conv=notrunc
	dd if=$(BUILD_DIR)/loader.bin of=$@ bs=512 count=4 seek=1 conv=notrunc
	dd if=$(BUILD_DIR)/kernel.bin of=$@ bs=512 count=200 seek=9 conv=notrunc

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/kernel
	mkdir -p $(BUILD_DIR)/device
	mkdir -p $(BUILD_DIR)/lib/kernel

.PHONY: all clean