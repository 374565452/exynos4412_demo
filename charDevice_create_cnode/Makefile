#!/bin/bash
obj-m += create_cnode.o

KDIR := /home/linux_source/iTop4412_Kernel_3.0

PWD ?= $(shell pwd)

all:
		make -C $(KDIR) M=$(PWD) modules

clean:
		rm -rf *.mod.c *.o *.order *.ko *.mod.o *.symvers