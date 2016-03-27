#########################
# Makefile for Orange'S #
#########################

# Entry point of Orange'S
# It must have the same value with 'KernelEntryPointPhyAddr' in load.inc!
ENTRYPOINT	= 0x1000

# Offset of entry point in kernel file
# It depends on ENTRYPOINT
ENTRYOFFSET	=   0x400

# Programs, flags, etc.
ASM		= nasm
DASM	= ndisasm
CC		= gcc
LD		= ld
ASMBFLAGS	= -I boot/include/
ASMKFLAGS	= -I include/ -f elf
CFLAGS		= -m32 -I include/ -c -fno-builtin -fno-stack-protector
LDFLAGS		= -s -Ttext $(ENTRYPOINT) -melf_i386
DASMFLAGS	= -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)

# This Program
ORANGESBOOT	= ../bin/boot_floppy.bin ../bin/loader_floppy.bin
OSCARSBOOT	= ../bin/boot_usb.bin ../bin/loader.bin
ORANGESKERNEL	= ../bin/kernel.bin
OBJS		= kernel/Kernel.o kernel/InitDscpt.o kernel/Init8259A.o kernel/Exception.o kernel/Interruption.o kernel/Exceptionc.o lib/Memory0.o   lib/Memory1.o kernel/Process.o lib/Delay.o lib/In_Out.o kernel/Interruptionc.o kernel/SystemCall.o kernel/Init8253.o lib/STI_CLI.o kernel/IPC.o lib/printf.o lib/Speaker.o lib/RTC.o lib/BootParams.o lib/fork.o mm/forkexit.o mm/main.o lib/misc.o lib/CPUID.o driver/video/graphics/Graphics.o driver/tty/Keyboard.o driver/tty/TTY.o lib/Graph.o driver/mouse/Mouse.o lib/SystimeShow.o driver/harddisk/HardDisk_IDE.o fs/FileSystem.o driver/pci/pci_io.o driver/pci/pci.o
DASMOUTPUT	= kernel.bin.asm

# All Phony Targets
.PHONY : everything final image clean realclean disasm all buildimg

# Default starting position
everything : $(ORANGESBOOT) $(OSCARSBOOT) $(ORANGESKERNEL) 

all : realclean everything

final : all clean

image : final buildimg

clean :
	rm -f $(OBJS)

realclean :
	rm -f $(OBJS) $(ORANGESBOOT) $(ORANGESKERNEL)
newdir:
	sudo mkdir /mnt/floppy/
	sudo mkdir /mnt/usb/
usb:
	sudo cp -fv boot/loader_usb.bin /media/oscar/"Oscar's OS"/loader.bin
	sudo cp -fv kernel.bin /media/oscar/"Oscar's OS"/
floppy:
	sudo mount -o loop TINIX.IMG /mnt/floppy/
	sudo cp -fv boot/loader_floppy.bin /mnt/floppy/loader.bin
	sudo cp -fv kernel.bin /mnt/floppy/
	sudo umount /mnt/floppy/
umall:
	sudo umount /mnt/floppy/
	sudo umount /mnt/usb/	
disasm :
	$(DASM) $(DASMFLAGS) $(ORANGESKERNEL) > $(DASMOUTPUT)

# We assume that "TINIX.IMG" exists in current folder
buildimg :
	dd if=boot/boot_floppy.bin of=../bin/TINIX.IMG bs=512 count=1 conv=notrunc
	sudo dd if=boot/boot_usb.bin of=/dev/sdb bs=512 count=1 seek=63 conv=notrunc
	sudo mount -o loop ../bin/TINIX.IMG /mnt/floppy/
	sudo mount -t vfat /dev/sdb /mnt/usb/
	sudo cp -fv boot/loader_floppy.bin /mnt/floppy/loader.bin
	sudo cp -fv boot/loader.bin /mnt/usb/loader.bin
	sudo cp -fv kernel.bin /mnt/floppy/
	sudo cp -fv kernel.bin /mnt/usb/kernel.bin
	sudo umount /mnt/floppy/
	sudo umount /mnt/usb/

../bin/boot_floppy.bin : boot/boot_floppy.asm boot/include/load.inc boot/include/fat12hdr.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<
	
../bin/boot_usb.bin : boot/boot_usb.asm boot/include/load.inc boot/include/Fat32BPB.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

../bin/loader_floppy.bin : boot/loader_floppy.asm boot/include/load.inc \
			boot/include/fat12hdr.inc boot/include/pm.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<
	
../bin/loader.bin : boot/loader_usb.asm boot/include/load.inc \
			boot/include/Fat32BPB.inc boot/include/pm.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

$(ORANGESKERNEL) : $(OBJS)
	$(LD) $(LDFLAGS) -o $(ORANGESKERNEL) $(OBJS)

kernel/Kernel.o : kernel/Kernel.asm include/sconst.inc
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/Exception.o : kernel/Exception.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/Exceptionc.o : kernel/Exception.c 
	$(CC) $(CFLAGS) -o $@ $<

kernel/Interruption.o : kernel/Interruption.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/Interruptionc.o : kernel/Interruption.c 
	$(CC) $(CFLAGS) -o $@ $<

kernel/InitDscpt.o : kernel/InitDscpt.c 
	$(CC) $(CFLAGS) -o $@ $<

kernel/Process.o : kernel/Process.c 
	$(CC) $(CFLAGS) -o $@ $<

kernel/Init8253.o : kernel/Init8253.c 
	$(CC) $(CFLAGS) -o $@ $<

kernel/IPC.o : kernel/IPC.c 
	$(CC) $(CFLAGS) -o $@ $<

kernel/Init8259A.o : kernel/Init8259A.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/SystemCall.o : kernel/SystemCall.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/Memory0.o : lib/Memory.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/In_Out.o : lib/In_Out.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/DispStr.o : lib/DispStr.c
	$(CC) $(CFLAGS) -o $@ $<

lib/Memory1.o : lib/Memory.c 
	$(CC) $(CFLAGS) -o $@ $<

lib/Delay.o : lib/Delay.c
	$(CC) $(CFLAGS) -o $@ $<

lib/STI_CLI.o : lib/STI_CLI.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/printf.o : lib/printf.c 
	$(CC) $(CFLAGS) -o $@ $<
	
lib/Speaker.o : lib/Speaker.c 
	$(CC) $(CFLAGS) -o $@ $<
	
lib/RTC.o : lib/RTC.c 
	$(CC) $(CFLAGS) -o $@ $<
	
lib/BootParams.o:	lib/BootParams.c
	$(CC) $(CFLAGS) -o $@ $<
	
lib/fork.o:	lib/fork.c
	$(CC) $(CFLAGS) -o $@ $<

lib/misc.o:	lib/misc.c
	$(CC) $(CFLAGS) -o $@ $<
	
lib/CPUID.o: lib/CPUID.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<
	
lib/SystimeShow.o: lib/SystimeShow.c
	$(CC) $(CFLAGS) -o $@ $<

mm/forkexit.o: mm/forkexit.c
	$(CC) $(CFLAGS) -o $@ $<
	
mm/main.o: mm/main.c
	$(CC) $(CFLAGS) -o $@ $<
	
driver/tty/Keyboard.o : driver/tty/Keyboard.c 
	$(CC) $(CFLAGS) -o $@ $<

driver/tty/TTY.o : driver/tty/TTY.c 
	$(CC) $(CFLAGS) -o $@ $<
	
driver/video/graphics/Graphics.o: driver/video/graphics/Graphics.c
	$(CC) $(CFLAGS) -o $@ $<
	
driver/mouse/Mouse.o: driver/mouse/Mouse.c
	$(CC) $(CFLAGS) -o $@ $<
	
driver/harddisk/HardDisk_IDE.o: driver/harddisk/HardDisk_IDE.c
	$(CC) $(CFLAGS) -o $@ $<
	
fs/FileSystem.o: fs/FileSystem.c
	$(CC) $(CFLAGS) -o $@ $<

lib/Graph.o:lib/Graph.c
	$(CC) $(CFLAGS) -o $@ $<
	
driver/pci/pci_io.o : driver/pci/pci_io.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<
	
driver/pci/pci.o : driver/pci/pci.c
	$(CC) $(CFLAGS) -o $@ $<




