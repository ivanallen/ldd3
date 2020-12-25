#include "scull_proc.h"
#include "scull_dev.h"

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

    if (down_interruptible(&device->sem))
        return -ERESTARTSYS;

    seq_printf(s, "\nDevice %i: ring_buffer:%px end:%px buffer_size:%d rp:%px wp:%px nreaders:%d nwriters:%d size:%d\n",
           (int)(device - scull_devices),
           device->ring_buffer,
           device->end,
           device->buffer_size,
           device->rp,
           device->wp,
           device->nreaders,
           device->nwriters,
           device->size); 

    up(&device->sem);

    return 0;
}

