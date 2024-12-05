TARGET_MODULE := My_Kernel

obj-m := $(TARGET_MODULE).o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
CC := gcc

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	@rm -f *.o *.ko *.mod.* *.symvers *.order *.mod.cmd *.mod .*.mod.* .*.*.cmd

Prog:
	@$(CC) -o 3_1.out 3_1.c
	@sudo ./3_1.out
	@rm -f 3_1.txt 3_1.out

load:
	@sudo insmod $(TARGET_MODULE).ko

unload:
	@sudo rmmod $(TARGET_MODULE).ko
