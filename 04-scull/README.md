# 字符设备驱动

## 1. 主设备和次设备号(major & minor)

### 1.1 设备号初始化与获取

- MAJOR(dev_t dev)
- MANOR(dev_t dev)
- MKDEV(int major, int minor)

### 1.2 设备啧分配与释放

- int register_chrdev_region(dev_t first, unsigned int count, char *name)
- int alloc_chrdev_region(dev_t *dev, unsigned int firstminor, unsigned int count, char *name)
- void unregister_chrdev_region(dev_t first, unsigned int count)

first 是要分配的设备号范围的起使值。count 是所请求的连续设备编号的个数。name 是和该编号范围关联的设备名称，它将出现在 /proc/devices 和 sysfs 中。

一般你可以自己指定一个设备号，然后告诉内核，但是这样一般不太好，因为你想用的设备号可能已经被别的驱动占用了。使用动态分配的方式往往会更好，因此推荐使用 alloc_* 函数，它会返回一个合适的设备号给你。

### 1.3 struct file_operations

```c
struct file_operations {
    struct module *owner;
    loff_t (*llseek) (struct file *, loff_t, int);
    ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
    ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
    ssize_t (*aio_read) (struct kiocb *, const struct iovec *, unsigned long, loff_t);
    ssize_t (*aio_write) (struct kiocb *, const struct iovec *, unsigned long, loff_t);
    int (*readdir) (struct file *, void *, filldir_t);
    unsigned int (*poll) (struct file *, struct poll_table_struct *);
    int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
    long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
    long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
    int (*mmap) (struct file *, struct vm_area_struct *);
    int (*open) (struct inode *, struct file *);
    int (*flush) (struct file *, fl_owner_t id);
    int (*release) (struct inode *, struct file *);
    int (*fsync) (struct file *, struct dentry *, int datasync);
    int (*aio_fsync) (struct kiocb *, int datasync);
    int (*fasync) (int, struct file *, int);
    int (*lock) (struct file *, int, struct file_lock *);
    ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
    unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
    int (*check_flags)(int);
    int (*flock) (struct file *, int, struct file_lock *);
    ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int);
    ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int);
    int (*setlease)(struct file *, long, struct file_lock **);
};
```

### 1.4 struct file

目前只保留一些我们关注的字段。

```c
struct file {
    const struct file_operations    *f_op;
    spinlock_t      f_lock;  /* f_ep_links, f_flags, no IRQ */
    atomic_long_t       f_count;
    unsigned int        f_flags;
    fmode_t         f_mode;
    loff_t          f_pos;
    struct fown_struct  f_owner;
    void            *private_data;
};
```

### 1.5 struct inode

目前只保留一些我们关注的字段。

```c
struct inode {
    atomic_t        i_count;
    dev_t           i_rdev;
    struct timespec     i_atime;
    struct timespec     i_mtime;
    struct timespec     i_ctime;
    union {
        struct pipe_inode_info  *i_pipe;
        struct block_device *i_bdev;
        struct cdev     *i_cdev;
    };
};
```

### 1.6 struct cdev

```c
struct cdev {
    struct kobject kobj;
    struct module *owner;
    const struct file_operations *ops;
    struct list_head list;
    dev_t dev;
    unsigned int count;
};
```
