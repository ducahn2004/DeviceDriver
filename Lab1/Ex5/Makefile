# Makefile for hello_multi (multi-file kernel module)

obj-m := hello_multi.o
hello_multi-objs := main.o helper.o

# Thư mục kernel source 5.15.163 (bạn đã build để chạy QEMU)
KDIR ?= $(HOME)/linux-5.15.163

# Thư mục hiện tại
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean


