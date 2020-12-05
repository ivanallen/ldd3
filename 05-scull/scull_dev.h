#pragma once

#include <linux/semaphore.h>
#include <linux/cdev.h>

#define SCULL_QUANTUM 4000
#define SCULL_QSET 1000
#define SCULL_MAJOR 231
#define SCULL_MINOR 0
#define SCULL_NR_DEVS 4

extern int scull_quantum;
extern int scull_qset;
extern int scull_nr_devs;
extern int scull_major;
extern int scull_minor;

/*
 * 量子集合
 * 量子：一个固定大小的内存单元
 */
struct scull_qset {
    void **data; /* 量子数组 */
    struct scull_qset *next; /* 下一个量子集 */
};

struct scull_dev {
    struct scull_qset *data; /* 量子集链表头 */
    int quantum; /* 量子大小 */
    int qset; /* 量子集大小 */
    unsigned long size; /* 保存在内存中的总数据量 */
    unsigned long access_key; /* 暂且忽略 */
    struct semaphore sem; /* 互斥信号量，暂且忽略 */
    struct cdev cdev; /* 设备对象，你已经很熟悉了 */
};

int scull_trim(struct scull_dev *dev);
void scull_setup_cdev(struct scull_dev *dev, int index, const struct file_operations *scull_fops);
void scull_cleanup_cdev(struct scull_dev *scull_device);
int scull_open(struct inode *inode, struct file *filp);
int scull_release(struct inode *inode, struct file *filp);
ssize_t scull_read(struct file *filp, char __user *buff, size_t count, loff_t *offp);
ssize_t scull_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp);
struct scull_qset *scull_follow(struct scull_dev *dev, int index);
