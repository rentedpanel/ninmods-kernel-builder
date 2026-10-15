MODULE_NAME := ninmods_driver
obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-y := ninmods_driver.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
