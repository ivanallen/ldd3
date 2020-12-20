#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

/* 许可证 */
MODULE_LICENSE("Dual BSD/GPL");

int sleepy_open(struct inode *inode, struct file *filp);
ssize_t sleepy_read(struct file *filp, char *buff, size_t count, loff_t *ppos);
ssize_t sleepy_write(struct file *filp, const char *buff, size_t count, loff_t *ppos);

struct sleepy_device {
    struct cdev cdev;
    /*
     * 或者你可以写成
     * struct wait_queue_head wq;
     */
    wait_queue_head_t wq;
    struct semaphore sem;
    int flag;
};

static struct file_operations f_ops = {
    .open = sleepy_open,
    .read = sleepy_read,
    .write = sleepy_write,
};

struct sleepy_device *device;

DECLARE_WAIT_QUEUE_HEAD(wq);

int sleepy_open(struct inode *inode, struct file *filp)
{
    filp->private_data = device;
    return 0;
}

ssize_t sleepy_read(struct file *filp, char *buff, size_t count, loff_t *ppos)
{
    struct sleepy_device *dev = filp->private_data;
    int cpu;

    down(&dev->sem);

    --dev->flag;


    while (dev->flag < 0) {
        /* struct wait_queue_entry wait */
        DEFINE_WAIT(wait);
        up(&dev->sem);

        cpu = get_cpu();
        printk(KERN_NOTICE "process %d(%s) prepare to sleep on cpu %d\n", current->pid, current->comm, cpu);
        put_cpu();
        /*
         * 如果有其它进程在这个时间点执行了唤醒
         * 会导致条件满足，但是状态却被设置成了 TASK_INTERRUPTIBLE
         * 所以 prepare 之后，还需要进行一次 if 条件的判断
         */
        prepare_to_wait(&wq, &wait, TASK_INTERRUPTIBLE);

        /*
         * 这个位置进程的状态要么是 TASK_INTERRUPTIBLE，要么是 TASK_RUNNING
         * 如果是 TASK_RUNNING，即使 schedule 也是安全的
         */
        if (dev->flag < 0)
            schedule();

        finish_wait(&wq, &wait);

        if (signal_pending(current))
            return -ERESTARTSYS;

        cpu = get_cpu();
        printk(KERN_NOTICE "process %d(%s) awake on cpu %d, flag:%d\n", current->pid, current->comm, cpu, dev->flag);
        put_cpu();

        down(&dev->sem);
    }


    up(&dev->sem);
    return 0;
}

ssize_t sleepy_write(struct file *filp, const char *buff, size_t count, loff_t *ppos)
{
    struct sleepy_device *dev = filp->private_data;
    int cpu = get_cpu();
    printk(KERN_NOTICE "process %d(%s) awake others on cpu %d\n", current->pid, current->comm, cpu);
    put_cpu();

    down(&dev->sem);
    ++dev->flag;
    up(&dev->sem);
    wake_up(&wq);
    /* 这个位置必须返回 count，想想为什么 */
    /* 可能会死循环 */
    return count;
}

static int __init sleepy_init(void)
{
    dev_t devno;
	printk(KERN_ALERT "sleepy init\n");
    device = kmalloc(sizeof(struct sleepy_device), GFP_KERNEL);
    device->flag = 0;
    sema_init(&device->sem, 1);
    init_waitqueue_head(&device->wq);

    devno = MKDEV(231, 0);

    if (!device)
        return -1;

    if (register_chrdev_region(devno, 1, "sleepy_device")) {
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
static void __exit sleepy_exit(void)
{
    dev_t devno;
    if (device) {
        kfree(device);
        device = NULL;
    }

    devno = MKDEV(231, 0);
    unregister_chrdev_region(devno, 1);
	printk(KERN_ALERT "sleepy exit\n");
}

module_init(sleepy_init);
module_exit(sleepy_exit);
