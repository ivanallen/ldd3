#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define MAX_BUF_SIZE 16

struct scull_dev *scull_device;

static void scull_setup_cdev(struct scull_dev *dev, int index);
static int scull_open(struct inode *inode, struct file *filp);
static int scull_release(struct inode *inode, struct file *filp);
static ssize_t scull_read(struct file *filp, char __user *buff, size_t count, loff_t *offp);
static ssize_t scull_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Allen");

struct scull_dev {
    char *buf;
    struct cdev cdev;
    dev_t devno;
    size_t size; /* 当前数据大小 */
};

static const struct file_operations scull_fops = {
    .owner = THIS_MODULE,
    .open = scull_open,
    .release = scull_release,
    .read = scull_read,
    .write = scull_write,
};

static int __init scull_init(void)
{
    printk(KERN_NOTICE "scull init\n");
    scull_device = kmalloc(sizeof(struct scull_dev), GFP_KERNEL);
    scull_setup_cdev(scull_device, 0);
    return 0;
}

static void __exit scull_exit(void)
{
    if (scull_device != NULL) {
        cdev_del(&scull_device->cdev);
        unregister_chrdev_region(scull_device->devno, 1);

        if (scull_device->buf != NULL) {
            kfree(scull_device->buf);
            scull_device->buf = NULL;
        }
        kfree(scull_device);
    }

    printk(KERN_NOTICE "scull exit\n");
}

static void scull_setup_cdev(struct scull_dev *dev, int index)
{
    /*
     * struct cdev {
     *     struct kobject kobj;
     *     struct module *owner;
     *     const struct file_operations *ops;
     *     struct list_head list;
     *     dev_t dev;
     *     unsigned int count;
     * };
     */
    int err, devno;

    devno = MKDEV(232, index);
    register_chrdev_region(devno, 1, "scull");

    /*
     * 初始化 cdev，并填充 struct file_operations
     * 为什么不自己给成员赋值？因为 cdev_init 会
     * 初始化 list 以及 kobj 字段。
     */
    cdev_init(&dev->cdev, &scull_fops);
    dev->cdev.owner = THIS_MODULE;

    /*
     * 第一个参数表示分配内存大小，
     * 第二个暂时写固定值
     */
    dev->buf = kmalloc(MAX_BUF_SIZE, GFP_KERNEL);
    dev->size = 0;
    dev->devno = devno;

    /*
     * 添加字段设备到系统。(主要是将设备加
     * 到内核中的 cdev_map 中，这是一个全局变量)
     *
     * 第 3 个参数表示连续的次设备号的个数
     */
    err = cdev_add(&dev->cdev, devno, 1);
    if (err)
        printk(KERN_ERR "error %d adding scull%d\n", err, index);
}

static int scull_open(struct inode *inode, struct file *filp)
{
    struct scull_dev *dev;

    printk(KERN_NOTICE "scull_open\n");
    /*
     * 问题：如果知道某结构体成员字段地址，
     * 能否知道该结构体首地址呢？
     *
     * container_of 就是做这样一件事，它可以
     * 依据成员字段的地址来推算结构体的首地址。如：
     * 已经 struct scull_dev 的成员 cdev 的地址
     * 是 p，则 struct scull_dev 结构体的首地址
     * 是 container_of(p, struct scull_dev, cdev)
     *
     * 其实现也相当巧妙，可参阅内核代码
     */
    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filp->private_data = dev;
    return 0;
}

static int scull_release(struct inode *inode, struct file *filp)
{
    printk(KERN_NOTICE "scull_release\n");
    return 0;
}

/*
 * 宏 __user 被定义为空，实际上就是一个标记,
 * 告诉你 buff 是用户空间地址，你需要小心
 */
static ssize_t scull_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
    struct scull_dev *dev;
    loff_t offset;

    dev = filp->private_data;
    offset = *offp;

    printk(KERN_NOTICE "[%s] user buff address:%p kernel buff address:%p, f_count:%d, f_pos:%llu, off:%llu count:%u, dev->size:%u\n",
            __func__, buff, dev->buf, filp->f_count.counter, filp->f_pos, offset, count, dev->size);

    if (offset + count >= dev->size)
        count = dev->size - offset;

    if (count == 0)
        return 0;

    if (copy_to_user(buff, dev->buf + offset, count)) {
        return -EFAULT;
    }

    *offp += count;

    return count;
}

static ssize_t scull_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
    struct scull_dev *dev;
    loff_t offset;

    dev = filp->private_data;
    /*
     * 更新 *off 到缓冲区的尾部
     */
    *offp = dev->size;
    offset = *offp;

    printk(KERN_NOTICE "[%s] user buff address:%p kernel buff address:%p, f_count:%d, f_pos:%llu, off:%llu count:%u, dev->size:%u\n",
            __func__, buff, dev->buf, filp->f_count.counter, filp->f_pos, offset, count, dev->size);

    if (offset + count > MAX_BUF_SIZE)
        count = MAX_BUF_SIZE - offset;

    if (count == 0) {
        printk(KERN_WARNING "[%s] buffer is full\n", __func__);
        return -EAGAIN;
    }

    if (copy_from_user(dev->buf + offset, buff, count)) {
        return -EFAULT;
    }
    *offp += count;

    dev->size += count;
    return count;
}

module_init(scull_init);
module_exit(scull_exit);
