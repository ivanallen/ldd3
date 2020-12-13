#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/rcupdate.h>
#include <linux/delay.h>
#include <linux/slab.h>

/* 许可证 */
MODULE_LICENSE("Dual BSD/GPL");

struct seq_t {
    unsigned int write_count;
    struct rcu_head rcu;
};

ssize_t rculock_read(struct file *filp, char *buff, size_t count, loff_t *ppos);
ssize_t rculock_write(struct file *filp, const char *buff, size_t count, loff_t *ppos);

struct file_operations rculock_fops = {
    .read = rculock_read,
    .write = rculock_write,
};

struct seq_t *myseq;



ssize_t rculock_read(struct file *filp, char *buff, size_t count, loff_t *ppos)
{
    char buf[64];
    int len;
    int cpu;
    unsigned int seq;
    struct seq_t *p;

    cpu = get_cpu();
    printk("[%s] process %d(%s) read before on cpu %d\n", __func__, current->pid, current->comm, cpu);
    put_cpu();

    rcu_read_lock();
    p = rcu_dereference(myseq);
    if (p)
        seq = p->write_count;
    rcu_read_unlock();
    
    cpu = get_cpu();
    printk("[%s] process %d(%s) read after on cpu %d\n", __func__, current->pid, current->comm, cpu);
    put_cpu();

    len = sprintf(buf, "%u\n", seq);

    if (copy_to_user(buff, buf, len)) {
        return -EFAULT;
    }

    ssleep(1);

    return len;
}

static void release_oldseq(struct rcu_head *rh)
{
	struct seq_t *p = container_of(rh, struct seq_t, rcu);
	printk("[%s]: addr:%px, write_count = %d\n", __func__, p, p->write_count);
	kfree(p);
}

ssize_t rculock_write(struct file *filp, const char *buff, size_t count, loff_t *ppos)
{
    struct seq_t *newseq = kmalloc(sizeof(struct seq_t), GFP_KERNEL);
    struct seq_t *old = myseq;
    *newseq = *myseq;
    newseq->write_count++;
	printk("[%s]: malloc new seq, addr:%px, write_count = %d\n", __func__, newseq, newseq->write_count);
    rcu_assign_pointer(myseq, newseq);
    call_rcu(&old->rcu, release_oldseq);
    return count;
}

static int __init lock_init(void)
{
    myseq = kmalloc(sizeof(struct seq_t), GFP_KERNEL);
    myseq->write_count = 0;
	printk("[%s]: malloc new seq, addr:%px, write_count = %d\n", __func__, myseq, myseq->write_count);

    printk(KERN_ALERT "lock init\n");
    proc_create("myrculock", 0666, NULL, &rculock_fops);

    return 0;
}

/* 模块卸载时调用 */
static void __exit lock_exit(void)
{
    printk(KERN_ALERT "lock exit\n");
    remove_proc_entry("myrculock", NULL);
	printk("[%s]: addr:%px, write_count = %d\n", __func__, myseq, myseq->write_count);
    kfree(myseq);
}

module_init(lock_init);
module_exit(lock_exit);
