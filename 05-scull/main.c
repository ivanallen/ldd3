#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "scull_dev.h"

#define MAX_BUF_SIZE 16

struct scull_dev *scull_devices;


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Allen");

module_param(scull_quantum, int, S_IRUGO);
module_param(scull_qset, int, S_IRUGO);
module_param(scull_nr_devs, int, S_IRUGO);
module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);

static const struct file_operations scull_fops = {
    .owner = THIS_MODULE,
    .open = scull_open,
    .release = scull_release,
    .read = scull_read,
    .write = scull_write,
};

static int __init scull_init(void)
{
    int err = 0;
    int i = 0;
    dev_t devno = MKDEV(scull_major, scull_minor);

    printk(KERN_NOTICE "scull init scull_quantum:%d scull_qset:%d scull_nr_devs:%d scull_major:%d scull_minor:%d\n", scull_quantum, scull_qset, scull_nr_devs, scull_major, scull_minor);

    /* 占位设备号 */
    if ((err = register_chrdev_region(devno, scull_nr_devs, "scull"))) {
        printk(KERN_ERR "register_chrdev_region failed\n");
        return err;
    }

    scull_devices = kmalloc(scull_nr_devs * sizeof(struct scull_dev), GFP_KERNEL);

    if (!scull_devices) {
        unregister_chrdev_region(devno, scull_nr_devs);
        return -ENOMEM;
    }

    memset(scull_devices, 0, scull_nr_devs * sizeof(struct scull_dev));

    for (i = 0; i < scull_nr_devs; ++i)
        scull_setup_cdev(&scull_devices[i], i, &scull_fops);
    return 0;
}

static void __exit scull_exit(void)
{
    int i;
    dev_t devno;

    if (scull_devices) {
        for (i = 0; i < scull_nr_devs; ++i) {
            printk(KERN_NOTICE "release device %d\n", i);
            scull_cleanup_cdev(scull_devices + i);
        }
        kfree(scull_devices);
        scull_devices = NULL;
    }

    devno = MKDEV(scull_major, scull_minor);
    unregister_chrdev_region(devno, scull_nr_devs);
    printk(KERN_NOTICE "scull exit\n");
}


module_init(scull_init);
module_exit(scull_exit);
