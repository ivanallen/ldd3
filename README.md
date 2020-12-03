# LDD3 光速入门

## 1 运行环境

### 1.1 使用虚拟机

- 操作系统：ubuntu-16.04.5-desktop-i386 (不局限于此版本)
- 下载种子：[点我下载](./env/ubuntu-16.04.5-desktop-i386.iso.torrent)，如果无效，就自己去 env 目录下面把种子下载下来。如果种子失效，请去官网下载。


### 1.2 使用树莓派 4B

有一些实验只能在树莓派 4B 上完成，比如 02-led. 当然，为了追求趣味性，强烈建议使用树莓派。

## 2 有用的工具

- vim/tree/git

```shell
> sudo apt install vim
> sudo apt install tree
> sudo apt install git
```

- vim 模板

https://gitee.com/ivan_allen/myvim

- 自动生成驱动模板代码

在你的新文件夹下执行下面的命令，自动生成程序。

```
$ toos/initmod helloworld
```

## 3. 安装

```shell
# 这里设置你的工作目录
> export LDD3_WORK_DIR=~/work/ldd3
> ./INSTALL.sh
> source ~/.bashrc
```
