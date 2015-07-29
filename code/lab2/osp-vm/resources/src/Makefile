KERNEL_VERSION	:= `uname -r`
KERNEL_DIR	:= /lib/modules/$(KERNEL_VERSION)/build
KERNEL_ARCH_DIR	:= /usr/src/kernel/linux-source-3.13.0/arch/x86/
INSTALL_MOD_DIR	:= .

PWD		:= $(shell pwd)

obj-m		:= sarlkm.o 
sarlkm-objs     := sar_main.o

all: sarlkm

sarlkm:
	@echo "Building SAR module..."
	@(cd $(KERNEL_DIR) && make -C $(KERNEL_DIR) SUBDIRS=$(PWD) CROSS_COMPILE=$(CROSS_COMPILE) modules)

install:

clean: toolsclean
	-rm -f *.o *.ko .*.cmd .*.flags *.mod.c Module.symvers version.h modules.order
	-rm -rf .tmp_versions

tools:
	@$(CC) -o test_syscall -I$(KERNEL_DIR)/include -I$(KERNEL_ARCH_DIR)/include test_syscall.c

toolsclean:
	-rm -f test_syscall
	
pack: clean
	tar cvf ../lab-2.tar ../src