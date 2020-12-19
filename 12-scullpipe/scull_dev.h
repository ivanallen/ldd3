#pragma once

#include <linux/semaphore.h>
#include <linux/cdev.h>
#include <linux/wait.h>

#define SCULL_MAJOR 231
#define SCULL_MINOR 0
#define SCULL_NR_DEVS 4
#define SCULL_BUFFER_SIZE 16

extern int scull_nr_devs;
extern int scull_major;
extern int scull_minor;
extern int scull_buffer_size;

struct scull_dev {
    wait_queue_head_t inq;
    wait_queue_head_t outq;
    char* ring_buffer;
    char* end;
    char* rp;
    char* wp;
    int size;
    int buffer_size;
    int nreaders;
    int nwriters;
    struct semaphore sem;
    struct cdev cdev;
};

extern struct scull_dev *scull_devices;

void scull_setup_cdev(struct scull_dev *dev, int index, const struct file_operations *scull_fops);
void scull_cleanup_cdev(struct scull_dev *scull_device);
int scull_open(struct inode *inode, struct file *filp);
int scull_release(struct inode *inode, struct file *filp);
ssize_t scull_read(struct file *filp, char __user *buff, size_t count, loff_t *offp);
ssize_t scull_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp);
