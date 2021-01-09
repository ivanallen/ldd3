#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/div64.h>
#include "scull_dev.h"

int scull_nr_devs = SCULL_NR_DEVS;
int scull_major = SCULL_MAJOR;
int scull_minor = SCULL_MINOR;
int scull_buffer_size = SCULL_BUFFER_SIZE;

struct scull_dev *scull_devices;

void scull_setup_cdev(struct scull_dev *dev, int index, const struct file_operations *scull_fops)
{
    int err;
    dev_t devno;

    cdev_init(&dev->cdev, scull_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->buffer_size = scull_buffer_size;
    dev->ring_buffer = kmalloc(scull_buffer_size, GFP_KERNEL);
    if (!dev->ring_buffer)
        printk(KERN_ERR "scull %d kmalloc failed\n", index);

    dev->end = dev->ring_buffer + scull_buffer_size;
    dev->nreaders = 0;
    dev->nwriters = 0;
    dev->rp = dev->wp = dev->ring_buffer;
    dev->size = 0;

    /*
     * 初始化等待队列
     */
    init_waitqueue_head(&dev->inq);
    init_waitqueue_head(&dev->outq);

    sema_init(&dev->sem, 1);

    devno = MKDEV(scull_major, scull_minor + index);
    err = cdev_add(&dev->cdev, devno, 1);
    if (err)
        printk(KERN_ERR "error %d adding scull%d\n", err, index);
}

void scull_cleanup_cdev(struct scull_dev *scull_device)
{
    cdev_del(&scull_device->cdev);
}

int scull_open(struct inode *inode, struct file *filp)
{
    struct scull_dev *dev;
    size_t minor = MINOR(filp->f_inode->i_rdev);

    printk(KERN_NOTICE "scull_open minor:%zu\n", minor);
    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filp->private_data = dev;

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    if (filp->f_mode & FMODE_READ)
        ++dev->nreaders;

    if (filp->f_mode & FMODE_WRITE)
        ++dev->nwriters;

    up(&dev->sem);
    return 0;
}

int scull_release(struct inode *inode, struct file *filp)
{
    struct scull_dev *dev = filp->private_data;
    size_t minor = MINOR(filp->f_inode->i_rdev);
    printk(KERN_NOTICE "scull_release minor:%zu\n", minor);

    down(&dev->sem);

    if (filp->f_mode & FMODE_READ)
        --dev->nreaders;

    if (filp->f_mode & FMODE_WRITE)
        --dev->nwriters;

    up(&dev->sem);
    return 0;
}

/*
 * 宏 __user 被定义为空，实际上就是一个标记,
 * 告诉你 buff 是用户空间地址，你需要小心
 */
ssize_t scull_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
    struct scull_dev *dev;

    dev = filp->private_data;

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    while (dev->rp == dev->wp) {
        up(&dev->sem);
        if (wait_event_interruptible(dev->inq, dev->rp != dev->wp))
            return -ERESTARTSYS;

        /* 被唤醒，重新加锁 */
        if (down_interruptible(&dev->sem))
            return -ERESTARTSYS;
    }

    if (dev->wp > dev->rp)
        count = min(count, (size_t)(dev->wp - dev->rp));
    else
        /* 指针回绕，就只读取到最后 */
        count = min(count, (size_t)(dev->end - dev->rp));

    if (copy_to_user(buff, dev->rp, count)) {
        up(&dev->sem);
        return -EFAULT;
    }

    dev->rp += count;

    /* 如果回绕就重置到起点 */
    if (dev->rp == dev->end)
        dev->rp = dev->ring_buffer;

    dev->size -= count;
    up(&dev->sem);

    wake_up_interruptible(&dev->outq);

    return count;
}

int spacefull(struct scull_dev *dev)
{
    return dev->wp + 1 == dev->rp ||
            (dev->wp + 1 == dev->end && dev->rp == dev->ring_buffer);
}

ssize_t scull_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
    struct scull_dev *dev;

    dev = filp->private_data;

    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    while (spacefull(dev)) {
        up(&dev->sem);
        if (wait_event_interruptible(dev->outq, !spacefull(dev)))
            return -ERESTARTSYS;

        if (down_interruptible(&dev->sem)) {
            return -ERESTARTSYS;
        }
    }

    if (dev->wp >= dev->rp) {
        if (dev->rp == dev->ring_buffer)
            count = min(count, (size_t)(dev->end - 1 - dev->wp));
        else
            count = min(count, (size_t)(dev->end - dev->wp));
    } else {
        count = min(count, (size_t)(dev->rp - 1 - dev->wp));
    }

    if (copy_from_user(dev->wp, buff, count)) {
        up(&dev->sem);
        return -EFAULT;
    }

    dev->wp += count;

    if (dev->wp == dev->end)
        dev->wp = dev->ring_buffer;

    dev->size += count;
    up(&dev->sem);
    wake_up_interruptible(&dev->inq);
    return count;
}
