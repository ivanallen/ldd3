ifneq ($(KERNELRELEASE),)
	obj-m:=sleepy.o
else
	PWD:=$(shell pwd)
	LINUX_KERNEL:=$(shell uname -r)
	LINUX_KERNEL_PATH:=/usr/src/linux-headers-$(LINUX_KERNEL)

defalut:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(PWD) modules

clean:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(PWD) clean

ins:
	sudo insmod sleepy.ko

node:
	sudo mknod /dev/sleepy_device c 231 0
	sudo chmod o+rw /dev/sleepy_device

rm:
	sudo rm -rf /dev/sleepy_device
	sudo rmmod sleepy
endif
