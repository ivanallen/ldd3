#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seqlock.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

/* 许可证 */
MODULE_LICENSE("Dual BSD/GPL");

struct seq_t {
    unsigned int write_count;
    seqlock_t myseqlock;
};

ssize_t seqlock_read(struct file *filp, char *buff, size_t count, loff_t *ppos);
ssize_t seqlock_write(struct file *filp, const char *buff, size_t count, loff_t *ppos);

struct file_operations seqlock_fops = {
    .read = seqlock_read,
    .write = seqlock_write,
};

struct seq_t myseq;

ssize_t seqlock_read(struct file *filp, char *buff, size_t count, loff_t *ppos)
{
    char buf[64];
    int len;
    int cpu;
    unsigned start;
    unsigned int seq;

    cpu = get_cpu();
    printk("[%s] process %d(%s) read before on cpu %d\n", __func__, current->pid, current->comm, cpu);
    put_cpu();

    // 读免锁
    do {
        start = read_seqbegin(&myseq.myseqlock);
        // 拿到副本
        seq = myseq.write_count;
        // 假定读者计算需要消耗很久
        ssleep(1);
    } while (read_seqretry(&myseq.myseqlock, start));

    cpu = get_cpu();
    printk("[%s] process %d(%s) read after on cpu %d\n", __func__, current->pid, current->comm, cpu);

    len = sprintf(buf, "%u\n", seq);

    if (copy_to_user(buff, buf, len)) {
        return -EFAULT;
    }

    return len;
}

ssize_t seqlock_write(struct file *filp, const char *buff, size_t count, loff_t *ppos)
{
    // 写保护
    write_seqlock(&myseq.myseqlock);
    myseq.write_count++;
    write_sequnlock(&myseq.myseqlock);
    return count;
}

static int __init lock_init(void)
{
    seqlock_init(&myseq.myseqlock);
    myseq.write_count = 0;

    printk(KERN_ALERT "lock init\n");
    proc_create("myseqlock", 0666, NULL, &seqlock_fops);

    return 0;
}

/* 模块卸载时调用 */
static void __exit lock_exit(void)
{
    printk(KERN_ALERT "lock exit\n");
    remove_proc_entry("myseqlock", NULL);
}

module_init(lock_init);
module_exit(lock_exit);
