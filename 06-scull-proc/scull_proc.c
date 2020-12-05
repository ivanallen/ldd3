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

