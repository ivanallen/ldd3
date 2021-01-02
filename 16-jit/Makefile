ifneq ($(KERNELRELEASE),)
	myjit-objs:=jit.o jit_currenttime.o jit_busy.o jit_sched.o jit_schedto.o
	obj-m:=myjit.o
else
	PWD:=$(shell pwd)
	LINUX_KERNEL:=$(shell uname -r)
	LINUX_KERNEL_PATH:=/usr/src/linux-headers-$(LINUX_KERNEL)

defalut:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(PWD) modules

clean:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(PWD) clean

ins:
	sudo insmod myjit.ko

rm:
	sudo rmmod myjit

endif
