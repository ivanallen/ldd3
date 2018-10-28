# 第一个驱动

## 1 编译

```shell
> make
```

## 2. 加载

```shell
> sudo insmod param.ko howmany=10 whom="Allen"
```

使用 dmesg 观察内核输出

```shell
> dmesg
```

## 3. 卸载

```shell
> sudo rmmod param
```
