

KBUILD_PATH := /lib/modules/$(shell uname -r)/build 
KO_NAME := simple_chardev

.PHONY: clean

obj-m += $(KO_NAME).o

$(KO_NAME)-objs += simple_chardev_drv.o simple_chardev_devfs_ops.o

modules:
	$(MAKE) -C $(KBUILD_PATH) M=$(PWD) modules

modules_clean: 
	@echo "clean all build";
	$(MAKE) -C $(KBUILD_PATH) M=$(PWD) clean

