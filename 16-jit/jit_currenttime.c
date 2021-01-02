#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>


void *jit_currenttime_seq_start(struct seq_file *s, loff_t *pos)
{
    if (*pos >= 10)
        return NULL;
    return (void *)1;
}

void *jit_currenttime_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    (*pos)++;
    if (*pos >= 10)
        return NULL;
    return (void *)1;
}

void jit_currenttime_seq_stop(struct seq_file *s, void *v)
{
    return;
}

ssize_t jit_currenttime_seq_show(struct seq_file *s, void *v)
{
    struct timeval tv1;
    struct timespec tv2;
    unsigned long j1;
    u64 j2;

    printk("jit_currenttime\n");

    /* get them four */
    j1 = jiffies;
    j2 = get_jiffies_64();
    do_gettimeofday(&tv1);
    tv2 = current_kernel_time();

    seq_printf(s, "0x%08lx 0x%016Lx %10i.%06i\n"
               "%40i.%09i\n",
               j1, j2,
               (int) tv1.tv_sec, (int) tv1.tv_usec,
               (int) tv2.tv_sec, (int) tv2.tv_nsec);

    return 0;
}

static struct seq_operations jit_currenttime_seq_ops = {
    .start = jit_currenttime_seq_start,
    .next = jit_currenttime_seq_next,
    .stop = jit_currenttime_seq_stop,
    .show = jit_currenttime_seq_show
};

int jit_currenttime_proc_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &jit_currenttime_seq_ops);
}

struct file_operations jit_currenttime_fops = {
    .owner = THIS_MODULE,
    .open = jit_currenttime_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release
};
