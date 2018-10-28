#include <linux/module.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Allen");

struct scull_dev {
	struct cdev cdev;
};

static const struct file_operations scull_fops = {
};

static int __init scull_init(void)
{
	printk(KERN_NOTICE "scull init\n");
	return 0;
}

static void __exit scull_exit(void)
{
	printk(KERN_NOTICE "scull exit\n");
}

static void scull_setup_cdev(struct scull_dev *dev, int index)
{
	/*
	 * struct cdev {
	 *     struct kobject kobj;
	 *     struct module *owner;
	 *     const struct file_operations *ops;
	 *     struct list_head list;
	 *     dev_t dev;
	 *     unsigned int count;
	 * };
	 */
	int err, devno;

	devno = MKDEV(231, index);

	/*
	 * 初始化 cdev，并填充 struct file_operations
	 * 为什么不自己给成员赋值？因为 cdev_init 会
	 * 初始化 list 以及 kobj 字段。
	 */
	cdev_init(&dev->cdev, &scull_fops);
	dev->cdev.owner = THIS_MODULE;

	/*
	 * 添加字段设备到系统。(主要是将设备加
	 * 到内核中的 cdev_map 中，这是一个全局变量)
	 *
	 * 第 3 个参数表示连续的次设备号的个数
	 */
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
		printk(KERN_NOTICE "error %d adding scull%d\n", err, index);
}

static int scull_open(struct inode *inode, struct file *filp)
{
	struct scull_dev *dev;

	/*
	 * 问题：如果知道某结构体成员字段地址，
	 * 能否知道该结构体首地址呢？
	 *
	 * container_of 就是做这样一件事，它可以
	 * 依据成员字段的地址来推算结构体的首地址。如：
	 * 已经 struct scull_dev 的成员 cdev 的地址
	 * 是 p，则 struct scull_dev 结构体的首地址
	 * 是 container_of(p, struct scull_dev, cdev)
	 *
	 * 其实现也相当巧妙，可参阅内核代码
	 */
	dev = container_of(inode->i_cdev, struct scull_dev, cdev);
	filp->private_data = dev;
	return 0;
}

static int scull_release(struct inode *inode, struct file *filp)
{
	return 0;
}

module_init(scull_init);
module_exit(scull_exit);
