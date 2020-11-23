ifneq ($(KERNELRELEASE),)
	obj-m:=led.o
else
	PWD:=$(shell pwd)
	LINUX_KERNEL:=$(shell uname -r)
	LINUX_KERNEL_PATH:=/usr/src/linux-headers-$(LINUX_KERNEL)

defalut:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(PWD) modules

clean:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(PWD) clean

endif
