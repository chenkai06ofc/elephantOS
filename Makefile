build_dir = build

all: bochs/c.img

clean:
	rm -rf $(build_dir)

$(build_dir)/mbr.bin: $(build_dir) boot/mbr.s
	nasm -I boot/include/ -o $@ boot/mbr.s

$(build_dir)/loader.bin: $(build_dir) boot/loader.s
	nasm -I boot/include/ -o $@ boot/loader.s

$(build_dir)/lib/kernel/print.o: $(build_dir) lib/kernel/print.s
	nasm -f elf -o $@ lib/kernel/print.s

$(build_dir)/kernel/main.o: $(build_dir) kernel/main.c
	gcc -m32 -c -I lib/kernel/ -o $@ kernel/main.c

$(build_dir)/kernel.bin: $(build_dir) $(build_dir)/kernel/main.o $(build_dir)/lib/kernel/print.o
	ld -m elf_i386 -Ttext 0xc0001500 -e main \
        -o $@ $(build_dir)/kernel/main.o $(build_dir)/lib/kernel/print.o

bochs/c.img: $(build_dir)/mbr.bin $(build_dir)/loader.bin $(build_dir)/kernel.bin
	dd if=$(build_dir)/mbr.bin of=$@ bs=512 count=1 conv=notrunc
	dd if=$(build_dir)/loader.bin of=$@ bs=512 count=4 seek=1 conv=notrunc
	dd if=$(build_dir)/kernel.bin of=$@ bs=512 count=200 seek=9 conv=notrunc

$(build_dir):
	mkdir -p $(build_dir)/kernel
	mkdir -p $(build_dir)/lib/kernel

.PHONY: all clean