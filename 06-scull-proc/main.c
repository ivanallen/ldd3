#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
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

int scull_read_procmem(char *buf, char **start, off_t offset, int count, int *eof, void *data);
int scull_proc_open(struct inode *inode, struct file *file);
void *scull_seq_start(struct seq_file *s, loff_t *pos);
void *scull_seq_next(struct seq_file *s, void *v, loff_t *pos);
void scull_seq_stop(struct seq_file *s, void *v);
int scull_seq_show(struct seq_file *s, void *v);


static const struct file_operations scull_fops = {
    .owner = THIS_MODULE,
    .open = scull_open,
    .release = scull_release,
    .read = scull_read,
    .write = scull_write,
};

static const struct file_operations scull_proc_ops = {
    .owner = THIS_MODULE,
    .open = scull_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release
};

static struct seq_operations scull_seq_ops = {
    .start = scull_seq_start,
    .next = scull_seq_next,
    .stop = scull_seq_stop,
    .show = scull_seq_show
};

int scull_proc_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &scull_seq_ops);
}

void *scull_seq_start(struct seq_file *s, loff_t *pos)
{
    if (*pos >= scull_nr_devs)
        return NULL;
    return scull_devices + *pos;
}

void *scull_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    (*pos)++;
    if (*pos >= scull_nr_devs)
        return NULL;
    return scull_devices + *pos;
}

void scull_seq_stop(struct seq_file *s, void *v)
{
    return;
}

int scull_seq_show(struct seq_file *s, void *v)
{
    struct scull_dev *device = (struct scull_dev *)v;
    struct scull_qset *q = NULL;
    struct scull_qset *next = NULL;
    int i = 0;


    if (down_interruptible(&device->sem))
        return -ERESTARTSYS;

    seq_printf(s, "\nDevice %i: qset %i, q %i, sz %li\n",
           (int)(device - scull_devices), device->qset, device->quantum, device->size); 

    for (q = device->data; q; q = next) {
        seq_printf(s, "  item at %p, qset at %p\n", device, device->data);
        if (q->data && !q->next)
            for (i = 0; i < device->qset; ++i) {
                if (q->data[i])
                    seq_printf(s, "    % 4i: %8p\n", i, q->data[i]);
            }
    }

    up(&device->sem);

    return 0;
}

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

    proc_create("scullmem", 0, NULL, &scull_proc_ops);
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
    remove_proc_entry("scullmem", NULL);
    printk(KERN_NOTICE "scull exit\n");
}


module_init(scull_init);
module_exit(scull_exit);
