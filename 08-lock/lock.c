#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

/* 许可证 */
MODULE_LICENSE("Dual BSD/GPL");

#define BUTTON_PIN 24
#define LED_PIN 18

ssize_t spinlock_deadlock_read(struct file *filp, char *buff, size_t count, loff_t *ppos);
ssize_t spinlock_lock_read(struct file *filp, char *buff, size_t count, loff_t *ppos);

struct file_operations spinlock_deadlock_fops = {
    .read = spinlock_deadlock_read
};

struct file_operations spinlock_lock_fops = {
    .read = spinlock_lock_read
};

spinlock_t myspinlock;

irqreturn_t button_handler(int irq, void *dev)
{
    printk("[%s] process %d(%s) interrupted on cpu %d\n", __func__, current->pid, current->comm, get_cpu());
    gpio_set_value(LED_PIN, !gpio_get_value(LED_PIN));
    return IRQ_HANDLED;
}

ssize_t spinlock_deadlock_read(struct file *filp, char *buff, size_t count, loff_t *ppos)
{
    printk("[%s] process %d(%s) lock before on cpu %d\n", __func__, current->pid, current->comm, get_cpu());
    spin_lock(&myspinlock);
    printk("[%s] process %d(%s) locking on cpu %d\n", __func__, current->pid, current->comm, get_cpu());
    // 此处会丢弃 cpu
    ssleep(20);
    spin_unlock(&myspinlock);
    printk("[%s] process %d(%s) lock after on cpu %d\n", __func__, current->pid, current->comm, get_cpu());
    return 0;
}

ssize_t spinlock_lock_read(struct file *filp, char *buff, size_t count, loff_t *ppos)
{
    unsigned long j;
    printk("[%s] process %d(%s) lock before on cpu %d\n", __func__, current->pid, current->comm, get_cpu());
    spin_lock(&myspinlock);
    printk("[%s] process %d(%s) locking on cpu %d\n", __func__, current->pid, current->comm, get_cpu());
    j = jiffies + 20 * HZ;
    while (time_before (jiffies, j));
    spin_unlock(&myspinlock);
    printk("[%s] process %d(%s) lock after on cpu %d\n", __func__, current->pid, current->comm, get_cpu());
    return 0;
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
    err = request_irq(gpio_to_irq(BUTTON_PIN), button_handler, IRQF_TRIGGER_RISING, "LED Test", NULL);

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
