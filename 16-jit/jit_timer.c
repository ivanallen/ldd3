#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/sched/signal.h>
#include <asm/hardirq.h>

static int s_delay = 100;

struct jit_timer_data {
    struct timer_list timer;
    wait_queue_head_t wait;
    struct seq_file *seq_file;
    unsigned long prev;
    int loops;
};

void *jit_timer_seq_start(struct seq_file *s, loff_t *pos)
{
    if (*pos > 0)
        return NULL;
    seq_printf(s, "   expires      time delta inirq    pid  cpu  command\n");
    return (void *)1;
}

void *jit_timer_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    (*pos)++;
    return NULL;
}

void jit_timer_seq_stop(struct seq_file *s, void *v)
{
    return;
}

void jit_timer_callback(struct timer_list *timer) {
    unsigned long j = jiffies;
    struct jit_timer_data *data;
    data = from_timer(data, timer, timer);

    seq_printf(data->seq_file, "%10lu %10lu   %3lu     %d   %4d   %d  %s\n",
           timer->expires, j, j - data->prev, in_interrupt() ? 1 : 0, current->pid, smp_processor_id(), current->comm);

    if (--data->loops) {
        data->prev = j;
        timer->expires = j + s_delay;
        add_timer(timer);
    } else {
        wake_up_interruptible(&data->wait);
    }
}

int jit_timer_seq_show(struct seq_file *s, void *v)
{
    int cpu;
    unsigned long j; /* jiffies */
    struct jit_timer_data *data = (struct jit_timer_data*)kmalloc(sizeof(struct jit_timer_data), GFP_KERNEL);

    if (!data)
        return 0;


    data->seq_file = s;
    data->loops = 5;

    init_waitqueue_head(&data->wait);
    timer_setup(&data->timer, jit_timer_callback, 0);


    cpu = get_cpu();
    put_cpu();

    j = jiffies;
    seq_printf(data->seq_file, "%10lu %10lu   %3lu     %d   %4d   %d  %s\n",
           j, j, 0lu, in_interrupt() ? 1 : 0, current->pid, cpu, current->comm);

    data->timer.expires = j + s_delay;
    data->prev = j;
    add_timer(&data->timer);

    wait_event_interruptible(data->wait, !data->loops);

    seq_printf(s, "I'm wake up, pid:%d\n", current->pid);
    kfree(data);

    if (signal_pending(current))
        return -ERESTARTSYS;

    return 0;
}

static struct seq_operations jit_timer_seq_ops = {
    .start = jit_timer_seq_start,
    .next = jit_timer_seq_next,
    .stop = jit_timer_seq_stop,
    .show = jit_timer_seq_show
};

int jit_timer_proc_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &jit_timer_seq_ops);
}

struct file_operations jit_timer_fops = {
    .owner = THIS_MODULE,
    .open = jit_timer_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release
};
