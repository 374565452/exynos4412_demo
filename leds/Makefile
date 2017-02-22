#!/bin/bash
obj-m += hello_leds.o

KDIR := /home/linux_source/iTop4412_Kernel_3.0

PWD ?= $(shell pwd)

all:
		make -C $(KDIR) M=$(PWD) modules

clean:
		rm -rf *.o