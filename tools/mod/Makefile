ifneq ($(KERNELRELEASE),)
	obj-m:={{ MOD_NAME }}.o
else
	PWD:=$(shell pwd)
	LINUX_KERNEL:=$(shell uname -r)
	LINUX_KERNEL_PATH:=/usr/src/linux-headers-$(LINUX_KERNEL)

defalut:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(PWD) modules

clean:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(PWD) clean

ins:
	sudo insmod {{ MOD_NAME }}.ko

rm:
	sudo rmmod {{ MOD_NAME }}

node:
	sudo rm -rf /dev/{{ MOD_NAME }}_device
	sudo mknod /dev/{{ MOD_NAME }}_device c 231 0
	sudo chmod o+rw /dev/{{ MOD_NAME }}_device

endif
