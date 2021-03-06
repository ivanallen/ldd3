#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>


void *jit_schedto_seq_start(struct seq_file *s, loff_t *pos)
{
    if (*pos >= 10)
        return NULL;
    return (void *)1;
}

void *jit_schedto_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    (*pos)++;
    if (*pos >= 10)
        return NULL;
    return (void *)1;
}

void jit_schedto_seq_stop(struct seq_file *s, void *v)
{
    return;
}

int jit_schedto_seq_show(struct seq_file *s, void *v)
{
    unsigned long j0, j1; /* jiffies */
    int delay = 4*HZ;

    j0 = jiffies;
    j1 = j0 + delay;

    set_current_state(TASK_INTERRUPTIBLE);
    schedule_timeout(delay);

    j1 = jiffies;

    seq_printf(s, "%9li %9li\n", j0, j1);

    return 0;
}

static struct seq_operations jit_schedto_seq_ops = {
    .start = jit_schedto_seq_start,
    .next = jit_schedto_seq_next,
    .stop = jit_schedto_seq_stop,
    .show = jit_schedto_seq_show
};

int jit_schedto_proc_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &jit_schedto_seq_ops);
}

struct file_operations jit_schedto_fops = {
    .owner = THIS_MODULE,
    .open = jit_schedto_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release
};
