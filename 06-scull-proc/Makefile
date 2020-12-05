ifneq ($(KERNELRELEASE),)
	scull-objs:=main.o scull_dev.o scull_proc.o
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
	sudo rm -rf /dev/myscull*
	sudo mknod /dev/myscull0 c 231 0
	sudo mknod /dev/myscull1 c 231 1
	sudo mknod /dev/myscull2 c 231 2
	sudo mknod /dev/myscull3 c 231 3
	sudo chmod o+rw /dev/myscull0
	sudo chmod o+rw /dev/myscull1
	sudo chmod o+rw /dev/myscull2
	sudo chmod o+rw /dev/myscull3

ins:
	sudo insmod scull.ko

rm:
	sudo rmmod scull

endif
