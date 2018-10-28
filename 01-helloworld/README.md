# 第一个驱动

## 1 编译

```shell
> make
```

## 2. 加载

```shell
> sudo insmod helloworld.ko
```

使用 dmesg 观察内核输出

```shell
> dmesg
```

## 3. 卸载

```shell
> sudo rmmod helloworld
```

### 4. 其它

- 编码风格

tools 目录里有检查语法的 shell 脚本 `ck`。编写内核模块的编码风格不同以往，比如 if 后面的单行语句不应该加花括号等。

- 删除生成的文件

```shell
> make clean
```
-
