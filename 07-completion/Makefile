ifneq ($(KERNELRELEASE),)
	obj-m:=completion.o
else
	PWD:=$(shell pwd)
	LINUX_KERNEL:=$(shell uname -r)
	LINUX_KERNEL_PATH:=/usr/src/linux-headers-$(LINUX_KERNEL)

defalut:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(PWD) modules

clean:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(PWD) clean

ins:
	sudo insmod completion.ko

node:
	sudo mknod /dev/completion_device c 231 0
	sudo chmod o+rw /dev/completion_device

rm:
	sudo rm -rf /dev/completion_device
	sudo rmmod completion
endif
