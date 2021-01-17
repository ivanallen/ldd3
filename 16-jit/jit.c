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

extern struct file_operations jit_currenttime_fops;
extern struct file_operations jit_busy_fops;
extern struct file_operations jit_sched_fops;
extern struct file_operations jit_schedto_fops;
extern struct file_operations jit_timer_fops;

irqreturn_t button_handler(int irq, void *dev)
{
    int cpu = get_cpu();
    int is_in_interrupt = in_interrupt();

    printk("[%s] process %d(%s) interrupted on cpu %d, in_interrupt:%d\n", __func__, current->pid, current->comm, cpu, is_in_interrupt);
    put_cpu();
    gpio_set_value(LED_PIN, !gpio_get_value(LED_PIN));
    return IRQ_HANDLED;
}

static int __init jit_init(void)
{
    int err;
    unsigned int irqno;
    printk(KERN_ALERT "jit init, HZ=%d\n", HZ);
    proc_create("jit_currenttime", 0, NULL, &jit_currenttime_fops);
    proc_create("jit_busy", 0, NULL, &jit_busy_fops);
    proc_create("jit_sched", 0, NULL, &jit_sched_fops);
    proc_create("jit_schedto", 0, NULL, &jit_schedto_fops);
    proc_create("jit_timer", 0, NULL, &jit_timer_fops);

    err = gpio_request_one(BUTTON_PIN, GPIOF_IN, "button request");
    if (err) return err;

    err = gpio_request_one(LED_PIN, GPIOF_OUT_INIT_LOW, "led request");
    if (err) return err;

    irqno = gpio_to_irq(BUTTON_PIN);
    enable_irq(irqno);
    err = request_irq(irqno, button_handler, IRQF_TRIGGER_RISING, "led test", NULL);

    if (err < 0) {
        printk("irq_request failed!\n");
        return err;
    }

    return 0;
}

/* 模块卸载时调用 */
static void __exit jit_exit(void)
{
    printk(KERN_ALERT "jit exit\n");
    remove_proc_entry("jit_currenttime", NULL);
    remove_proc_entry("jit_busy", NULL);
    remove_proc_entry("jit_sched", NULL);
    remove_proc_entry("jit_schedto", NULL);
    remove_proc_entry("jit_timer", NULL);
    free_irq(gpio_to_irq(BUTTON_PIN), NULL);
    gpio_free(BUTTON_PIN);
    gpio_free(LED_PIN);
}

module_init(jit_init);
module_exit(jit_exit);
