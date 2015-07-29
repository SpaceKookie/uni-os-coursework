all: clean

	gcc -m32 -c -g -Os -march=i686 -ffreestanding -Wall -Werror -I. -o os.o os.c
	ld -m elf_i386 -static -Tlinker.ld -nostdlib --nmagic -o os.elf os.o
	objcopy -O binary os.elf os.bin

	chmod -x os.elf os.bin


start: clean all
	qemu-system-x86_64 os.bin


clean:
	rm -f *.bin *.img *.elf *~ *.o *.s

pack: clean
	tar cvf ../lab-1.tar ../src

as: clean
	gcc -m32 -c -g -Os -march=i686 -ffreestanding -Wall -Werror -I. -S os.c -o os.s
	as os.s -o os.o
	ld -m elf_i386 -static -Tlinker.ld -nostdlib --nmagic -o os.elf os.o
	objcopy -O binary os.elf os.bin

	chmod -x os.elf os.bin
