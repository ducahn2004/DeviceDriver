KDIR ?= $(HOME)/linux-5.15.163
PWD := $(shell pwd)

obj-m := stats_provider.o stats_consumer.o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean

