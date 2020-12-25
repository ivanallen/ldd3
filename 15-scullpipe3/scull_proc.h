#pragma once

#include <linux/fs.h>
#include <linux/seq_file.h>

int scull_proc_open(struct inode *inode, struct file *file);
void *scull_seq_start(struct seq_file *s, loff_t *pos);
void *scull_seq_next(struct seq_file *s, void *v, loff_t *pos);
void scull_seq_stop(struct seq_file *s, void *v);
int scull_seq_show(struct seq_file *s, void *v);
