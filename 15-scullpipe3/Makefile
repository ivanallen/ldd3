EXTRA_CFLAGS += -g
ifneq ($(KERNELRELEASE),)
	scullp-objs:=main.o scull_dev.o scull_proc.o
	obj-m:=scullp.o
else
	PWD:=$(shell pwd)
	LINUX_KERNEL:=$(shell uname -r)
	LINUX_KERNEL_PATH:=/usr/src/linux-headers-$(LINUX_KERNEL)

defalut:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(PWD) modules

clean:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(PWD) clean

reader:reader.c
	$(CC) -o $@ $^

mp:multiplex.c
	$(CC) -o $@ $^

mp2:multiplex2.c
	$(CC) -o $@ $^

node:
	sudo mknod /dev/myscullpipe0 c 231 0
	sudo mknod /dev/myscullpipe1 c 231 1
	sudo mknod /dev/myscullpipe2 c 231 2
	sudo mknod /dev/myscullpipe3 c 231 3
	sudo chmod o+rw /dev/myscullpipe0
	sudo chmod o+rw /dev/myscullpipe1
	sudo chmod o+rw /dev/myscullpipe2
	sudo chmod o+rw /dev/myscullpipe3

ins:
	sudo insmod scullp.ko

rm:
	sudo rmmod scullp
	sudo rm -rf /dev/myscullpipe*

endif
