# 这个 makefile 和你以往看到的不一样，这不是 makefile 的常见形式
# obj-m 定义了模块的名字
obj-m=helloworld.o

# /lib/modules/$(shell uname -r)/build 指定了内核源代码树
# -M 参数让 makefile 在构造 helloworld 之前返回到模块目录
all:
	make -C /lib/modules/$(shell uname -r)/build M=`pwd` modules

clean:
	make -C /lib/modules/$(shell uname -r)/build  M=`pwd` clean

.PHONY:clean
