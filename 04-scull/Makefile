ifneq ($(KERNELRELEASE),)
	obj-m:=scull.o
else
	PWD:=$(shell pwd)
	LINUX_KERNEL:=$(shell uname -r)
	LINUX_KERNEL_PATH:=/usr/src/linux-headers-$(LINUX_KERNEL)

defalut:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(PWD) modules

clean:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(PWD) clean

node:
	sudo rm -rf /dev/myscull0
	sudo rm -rf /dev/myscull1
	sudo mknod /dev/myscull0 c 232 0
	sudo mknod /dev/myscull1 c 232 1
	sudo chmod o+rw /dev/myscull0
	sudo chmod o+rw /dev/myscull1

ins:
	sudo insmod scull.ko

rm:
	sudo rmmod scull

endif
