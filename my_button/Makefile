#!/bin/bash
obj-m += key_interrupt.o

KDIR := /home/linux_source/iTop4412_Kernel_3.0

PWD ?= $(shell pwd)

all:
		make -C $(KDIR) M=$(PWD) modules

clean:
		rm -rf *.mod.c *.o *.order *.ko *.mod.o *.symvers