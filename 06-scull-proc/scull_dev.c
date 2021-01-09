#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/div64.h>
#include "scull_dev.h"

int scull_quantum = SCULL_QUANTUM;
int scull_qset = SCULL_QSET;
int scull_nr_devs = SCULL_NR_DEVS;
int scull_major = SCULL_MAJOR;
int scull_minor = SCULL_MINOR;

struct scull_dev *scull_devices;

int scull_trim(struct scull_dev *dev)
{
    struct scull_qset *q = NULL;
    struct scull_qset *next = NULL;
    int qset = dev->qset;
    int i, j;
    
    printk(KERN_NOTICE "scull_trim");

    j = 0;
    for (q = dev->data; q; q = next, ++j) {
        printk(KERN_NOTICE "trim qset:%px", q);
        next = q->next;

        if (q->data) {
            for (i = 0; i < qset; ++i) {
                if (q->data[i]) {
                    printk(KERN_NOTICE "free quantum:%d address:%px\n", i, q->data[i]);
                    kfree(q->data[i]);
                }
            }
            printk(KERN_NOTICE "free qset data address:%px\n", q->data);
            kfree(q->data);
        }

        printk(KERN_NOTICE "free qset:%d address:%px\n", j, q);
        kfree(q);
    }

    dev->size = 0;
    dev->quantum = scull_quantum;
    dev->qset = scull_qset;
    dev->data = NULL;
    return 0;
}

void scull_setup_cdev(struct scull_dev *dev, int index, const struct file_operations *scull_fops)
{
    int err;
    dev_t devno;

    cdev_init(&dev->cdev, scull_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->data = NULL;
    dev->quantum = scull_quantum;
    dev->qset = scull_qset;
    dev->size = 0;
    sema_init(&dev->sem, 1);

    devno = MKDEV(scull_major, scull_minor + index);
    err = cdev_add(&dev->cdev, devno, 1);
    if (err)
        printk(KERN_ERR "error %d adding scull%d\n", err, index);
}

void scull_cleanup_cdev(struct scull_dev *scull_device)
{
    scull_trim(scull_device);
    cdev_del(&scull_device->cdev);
}

int scull_open(struct inode *inode, struct file *filp)
{
    struct scull_dev *dev;
    size_t minor = MINOR(filp->f_inode->i_rdev);

    printk(KERN_NOTICE "scull_open minor:%zu\n", minor);
    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filp->private_data = dev;

    // O_ACCMODE = O_RDWR | O_WRONLY;
    if ((filp->f_flags & O_ACCMODE) == O_WRONLY) {
        scull_trim(dev);
    }
    return 0;
}

int scull_release(struct inode *inode, struct file *filp)
{
    size_t minor = MINOR(filp->f_inode->i_rdev);
    printk(KERN_NOTICE "scull_release minor:%zu\n", minor);
    return 0;
}

/*
 * 宏 __user 被定义为空，实际上就是一个标记,
 * 告诉你 buff 是用户空间地址，你需要小心
 */
ssize_t scull_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
    struct scull_dev *dev;
    struct scull_qset *q;
    loff_t offset;
    uint64_t qset_size, quantum, qset;
    ssize_t ret = 0;
    size_t minor = MINOR(filp->f_inode->i_rdev);
    /*
     * i 表示第几个 qset
     * j 表示 qset 中的第几个 quantum
     * k 表示 quantum 中的第几个字节
     */
    uint64_t i, j, k;

    dev = filp->private_data;
    q = dev->data;
    quantum = dev->quantum;
    qset = dev->qset;

    qset_size = quantum * qset;

    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    offset = *offp;

    printk(KERN_NOTICE "[%s] user buff address:%px f_count:%lld, f_pos:%llu, off:%llu, count:%zu, minor:%zu\n",
            __func__, buff, (long long)filp->f_count.counter, filp->f_pos, offset, count, minor);

    do {
        if (offset >= dev->size) {
            break;
        }

        if (offset + count > dev->size) {
            count = dev->size - offset;
        }

        /*
         * 这里无法直接使用 / %，会报错。
         * i = offset / qset_size;
         * j = offset % qset_size / quantum;
         * k = offset % qset_size % quantum;
         */

        /*
         * do_div 是一个宏
         * 第一个参数是被除数，同时接收商
         * 第二个参数是除数
         * 返回值接收余数
         */
        i = offset;
        j = do_div(i, qset_size);
        k = do_div(j, quantum);
        k = do_div(k, quantum);

        q = scull_follow(dev, i);

        if (!q || !q->data || !q->data[j]) {
            break;
        }

        if (count > quantum - k) {
            count = quantum - k;
        }

        if (copy_to_user(buff, q->data[j] + k, count)) {
            ret = -EFAULT;
            break;
        }

        printk(KERN_NOTICE "copy_to_user %zd bytes\n", count);

        *offp += count;
        ret = count;
    } while (0);

    up(&dev->sem);

    return ret;
}

ssize_t scull_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
    struct scull_dev *dev;
    struct scull_qset *q;
    loff_t offset;
    uint64_t qset_size, quantum, qset;
    ssize_t ret = 0;
    size_t minor = MINOR(filp->f_inode->i_rdev);
    /*
     * i 表示第几个 qset
     * j 表示 qset 中的第几个 quantum
     * k 表示 quantum 中的第几个字节
     */
    uint64_t i, j, k;

    dev = filp->private_data;
    q = dev->data;
    quantum = dev->quantum;
    qset = dev->qset;

    qset_size = quantum * qset;

    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    offset = *offp;

    printk(KERN_NOTICE "[%s] user buff address:%px, f_count:%lld, f_pos:%llu, off:%llu count:%zu, minor:%zu\n",
            __func__, buff, (long long)filp->f_count.counter, filp->f_pos, offset, count, minor);

    do {
        i = offset;
        j = do_div(i, qset_size);
        k = do_div(j, quantum);
        k = do_div(k, quantum);

        printk(KERN_NOTICE "qset:%llu quantum:%llu offset:%llu\n", i, j, k);

        q = scull_follow(dev, i);

        if (!q)
            break;

        if (!q->data) {
            q->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
            if (!q->data)
                break;
            printk(KERN_NOTICE "malloc qset data:%px\n", q->data);
            memset(q->data, 0, qset * sizeof(char *));
        }

        if (!q->data[j]) {
            q->data[j] = kmalloc(quantum, GFP_KERNEL);
            if (!q->data[j])
                break;
            printk(KERN_NOTICE "malloc quantum:%px\n", q->data[j]);
            memset(q->data[j], 0, quantum);
        }

        if (count > quantum - k) {
            count = quantum - k;
        }

        if (copy_from_user(q->data[j] + k, buff, count)) {
            ret = -EFAULT;
            break;
        }

        printk(KERN_NOTICE "copy_from_user %zd bytes\n", count);

        *offp += count;
        dev->size = *offp;
        ret = count;
    } while (0);

    up(&dev->sem);
    return ret;
}

struct scull_qset *scull_follow(struct scull_dev *dev, int index)
{
    struct scull_qset *q = dev->data;

    if (!q) {
        q = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
        if (!q)
            return NULL;
        dev->data = q;
        memset(q, 0, sizeof(struct scull_qset));
        printk(KERN_NOTICE "malloc first qset:%px qset->next:%px", q, q->next);
    }

    while (index--) {
        if (!q->next) {
            q->next = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
            printk(KERN_NOTICE "malloc new qset:%px", q->next);
            if (!q->next)
                return NULL;
            memset(q->next, 0, sizeof(struct scull_qset));
        }
        q = q->next;
    }

    return q;
}
