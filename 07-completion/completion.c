#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/completion.h>

/* 许可证 */
MODULE_LICENSE("Dual BSD/GPL");

int completion_open(struct inode *inode, struct file *filp);
ssize_t completion_read(struct file *filp, char *buff, size_t count, loff_t *ppos);
ssize_t completion_write(struct file *filp, const char *buff, size_t count, loff_t *ppos);

struct completion_device {
    struct cdev cdev;
};

static struct file_operations f_ops = {
    .open = completion_open,
    .read = completion_read,
    .write = completion_write,
};

struct completion_device *device;

DECLARE_COMPLETION(comp);

int completion_open(struct inode *inode, struct file *filp)
{
    filp->private_data = device;
    return 0;
}

ssize_t completion_read(struct file *filp, char *buff, size_t count, loff_t *ppos)
{
    int cpu = get_cpu();
    printk(KERN_NOTICE "process %d(%s) ready to sleep on cpu %d\n", current->pid, current->comm, cpu);
    put_cpu();
    wait_for_completion(&comp);
    cpu = get_cpu();
    printk(KERN_NOTICE "process %d(%s) awake on cpu %d\n", current->pid, current->comm, cpu);
    put_cpu();
    return 0;
}

ssize_t completion_write(struct file *filp, const char *buff, size_t count, loff_t *ppos)
{
    int cpu = get_cpu();
    printk(KERN_NOTICE "process %d(%s) awake others on cpu %d\n", current->pid, current->comm, cpu);
    put_cpu();
    complete(&comp);
    /* 这个位置必须返回 count，想想为什么 */
    /* 可能会死循环 */
    return count;
}

static int __init completion_init(void)
{
    dev_t devno;
	printk(KERN_ALERT "completion init\n");
    device = kmalloc(sizeof(struct completion_device), GFP_KERNEL);

    devno = MKDEV(231, 0);

    if (!device)
        return -1;

    if (register_chrdev_region(devno, 1, "completion_device")) {
        kfree(device);
        return -1;
    }

    cdev_init(&device->cdev, &f_ops);

    if (cdev_add(&device->cdev, devno, 1)) {
        kfree(device);
        unregister_chrdev_region(devno, 1);
        return -1;
    }
	return 0;
}

/* 模块卸载时调用 */
static void __exit completion_exit(void)
{
    dev_t devno;
    if (device) {
        kfree(device);
        device = NULL;
    }

    devno = MKDEV(231, 0);
    unregister_chrdev_region(devno, 1);
	printk(KERN_ALERT "completion exit\n");
}

module_init(completion_init);
module_exit(completion_exit);
