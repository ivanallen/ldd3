obj-m=param.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=`pwd` modules

clean:
	find -maxdepth 1 ! \( \( -name '*.c' -and ! -name '*.mod.c' \) -or -iname 'README.md' -or -iname 'Makefile' \) -and ! -name '.' | xargs rm -r

.PHONY:clean
