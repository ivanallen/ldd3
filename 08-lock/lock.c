#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

/* 许可证 */
MODULE_LICENSE("Dual BSD/GPL");

#define BUTTON_PIN 24
#define LED_PIN 18

struct process {
    int is_read;
};

ssize_t spinlock_deadlock_read(struct file *filp, char *buff, size_t count, loff_t *ppos);
ssize_t spinlock_lock_read(struct file *filp, char *buff, size_t count, loff_t *ppos);
int spinlock_open(struct inode *inode, struct file *filp);
int spinlock_release(struct inode *inode, struct file *filp);

struct file_operations spinlock_deadlock_fops = {
    .open = spinlock_open,
    .release = spinlock_release,
    .read = spinlock_deadlock_read
};

struct file_operations spinlock_lock_fops = {
    .open = spinlock_open,
    .release = spinlock_release,
    .read = spinlock_lock_read
};

spinlock_t myspinlock;


irqreturn_t button_handler(int irq, void *dev)
{
    int cpu = get_cpu();
    printk("[%s] process %d(%s) interrupted on cpu %d\n", __func__, current->pid, current->comm, cpu);
    put_cpu();
    gpio_set_value(LED_PIN, !gpio_get_value(LED_PIN));
    return IRQ_HANDLED;
}

int spinlock_open(struct inode *inode, struct file *filp)
{
    struct process *p = kmalloc(sizeof(struct process), GFP_KERNEL);
    if (!p)
        return -EFAULT;

    p->is_read = 0;
    filp->private_data = p;
    return 0;
}

int spinlock_release(struct inode *inode, struct file *filp)
{
    if (filp->private_data)
        kfree(filp->private_data);
    filp->private_data = NULL;
    return 0;
}

ssize_t spinlock_deadlock_read(struct file *filp, char *buff, size_t count, loff_t *ppos)
{
    int cpu = get_cpu();
    printk("[%s] process %d(%s) lock before on cpu %d\n", __func__, current->pid, current->comm, cpu);
    put_cpu();

    spin_lock(&myspinlock);
    printk("[%s] process %d(%s) locking on cpu %d\n", __func__, current->pid, current->comm, smp_processor_id());
    // 此处会丢弃 cpu
    ssleep(20);
    spin_unlock(&myspinlock);

    cpu = get_cpu();
    printk("[%s] process %d(%s) unlock after on cpu %d\n", __func__, current->pid, current->comm, cpu);
    put_cpu();
    return 0;
}

ssize_t spinlock_lock_read(struct file *filp, char *buff, size_t count, loff_t *ppos)
{
    unsigned long j0, j1;
    char buf[64];
    int len;
    struct process *p = filp->private_data;
    int cpu;

    if (p->is_read)
        return 0;

    cpu = get_cpu();
    printk("[%s] process %d(%s) lock before on cpu %d\n", __func__, current->pid, current->comm, cpu);
    put_cpu();

    spin_lock(&myspinlock);
    printk("[%s] process %d(%s) locking on cpu %d\n", __func__, current->pid, current->comm, smp_processor_id());
    j0 = jiffies;
    j1 = j0 + 20 * HZ;
    while (time_before (jiffies, j1));
    // 更新 j1 为实际值
    j1 = jiffies;
    spin_unlock(&myspinlock);

    cpu = get_cpu();
    printk("[%s] process %d(%s) unlock after on cpu %d. j0:%lu j1:%lu\n", __func__, current->pid, current->comm, cpu, j0, j1);
    put_cpu();

    len = sprintf(buf, "%9lu %9lu\n", j0, j1);
    if (copy_to_user(buff, buf, len)) {
        return -EFAULT;
    }
    *ppos = len;
    p->is_read = 1;
    return len;
}

static int __init lock_init(void)
{
    int err;
    spin_lock_init(&myspinlock);
    printk(KERN_ALERT "lock init\n");
    proc_create("myspinlock_deadlock", 0, NULL, &spinlock_deadlock_fops);
    proc_create("myspinlock_lock", 0, NULL, &spinlock_lock_fops);

    err = gpio_request_one(BUTTON_PIN, GPIOF_IN, "button request");
    if (err) return err;

    err = gpio_request_one(LED_PIN, GPIOF_OUT_INIT_LOW, "led request");
    if (err) return err;

    enable_irq(gpio_to_irq(BUTTON_PIN));
    err = request_irq(gpio_to_irq(BUTTON_PIN), button_handler, IRQF_TRIGGER_RISING, "led Test", NULL);

    if (err < 0) {
        printk("irq_request failed!\n");
        return err;
    }

    return 0;
}

/* 模块卸载时调用 */
static void __exit lock_exit(void)
{
    printk(KERN_ALERT "lock exit\n");
    remove_proc_entry("myspinlock_deadlock", NULL);
    remove_proc_entry("myspinlock_lock", NULL);
    free_irq(gpio_to_irq(BUTTON_PIN), NULL);
    gpio_free(BUTTON_PIN);
    gpio_free(LED_PIN);
}

module_init(lock_init);
module_exit(lock_exit);
