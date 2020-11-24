/* 此代码仅适用于树莓派 4B */
/* 包含大量的符号和函数定义 */
#include <linux/module.h>
#include <linux/fs.h>
/* 初始化和清除函数，但不包含也不出错 */
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/io.h>

/* 许可证 */
MODULE_LICENSE("Dual BSD/GPL");

/*
 * 这里这个数字是从树莓派手册上查的，表示 GPIO 控制寄存器。
 */
#define GPIO_PUP_PDN_CNTRL_REG1 0xfe2000e8
#define GPFSEL1 0xfe200004
#define GPFSET0 0xfe20001c
#define GPFCLR0 0xfe200028
#define MAX_BUF_SIZE 16

struct led_dev *led_device;

static void led_setup_cdev(struct led_dev *dev, int index);
static int led_open(struct inode *inode, struct file *filp);
static int led_release(struct inode *inode, struct file *filp);
static ssize_t led_read(struct file *filp, char __user *buff, size_t count, loff_t *offp);
static ssize_t led_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp);
static void open_led(void);
static void close_led(void);

struct led_dev {
    int state;
    struct cdev cdev;
};

static const struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .release = led_release,
    .read = led_read,
    .write = led_write,
};

static void open_led(void) {
    iowrite32(1 << 18, ioremap(GPFCLR0, 4));
}

static void close_led(void) {
    iowrite32(1 << 18, ioremap(GPFSET0, 4));
}

static void led_setup_cdev(struct led_dev *dev, int index)
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
    register_chrdev_region(devno, 1, "myled");

    /*
     * 初始化 cdev，并填充 struct file_operations
     * 为什么不自己给成员赋值？因为 cdev_init 会
     * 初始化 list 以及 kobj 字段。
     */
    cdev_init(&dev->cdev, &led_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->state = 0;
    
    // 设置 GPIO0 为 output 模式
    iowrite32(0x001 << 24, ioremap(GPFSEL1, 4));
    // 设置 GPIO0 为下拉模式
    // iowrite32(0x10 << 4, ioremap(GPIO_PUP_PDN_CNTRL_REG1, 4));
    close_led();

    /*
     * 添加字段设备到系统。(主要是将设备加
     * 到内核中的 cdev_map 中，这是一个全局变量)
     *
     * 第 3 个参数表示连续的次设备号的个数
     */
    err = cdev_add(&dev->cdev, devno, 1);
    if (err)
        printk(KERN_ERR "error %d adding led%d\n", err, index);
}

static int led_open(struct inode *inode, struct file *filp)
{
    struct led_dev *dev;

    /*
     * 问题：如果知道某结构体成员字段地址，
     * 能否知道该结构体首地址呢？
     *
     * container_of 就是做这样一件事，它可以
     * 依据成员字段的地址来推算结构体的首地址。如：
     * 已经 struct led_dev 的成员 cdev 的地址
     * 是 p，则 struct led_dev 结构体的首地址
     * 是 container_of(p, struct led_dev, cdev)
     *
     * 其实现也相当巧妙，可参阅内核代码
     */
    dev = container_of(inode->i_cdev, struct led_dev, cdev);
    filp->private_data = dev;
    printk(KERN_ALERT "led open\n");
    return 0;
}

static int led_release(struct inode *inode, struct file *filp)
{
    return 0;
}

/*
 * 宏 __user 被定义为空，实际上就是一个标记,
 * 告诉你 buff 是用户空间地址，你需要小心
 */
static ssize_t led_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
    struct led_dev *dev;

    dev = filp->private_data;

    if (count == 0)
        return 0;

    if (count < 3) {
        return 0;
    }

    if (dev->state == 0) {
        copy_to_user(buff, "off", 3);
        count = 3;
    } else {
        copy_to_user(buff, "on", 2);
        count = 2;
    }

    return count;
}

static ssize_t led_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
    struct led_dev *dev;
    char buf[16] = { 0 };

    dev = filp->private_data;

    if (count > MAX_BUF_SIZE)
        return 0;

    if (count == 0)
        return 0;

    copy_from_user(buf, buff, count);

    if (strncmp("on", buf, MAX_BUF_SIZE) == 0) {
        open_led();
        dev->state = 1;
    } else if (strncmp("off", buf, MAX_BUF_SIZE) == 0) {
        close_led();
        dev->state = 0;
    }

    return count;
}

/*
 * 还有其它类似的 MODULE_ 系列的宏，
 * 比如 MODULE_AUTHOR 等，遇到再说。
 */

/*
 * 初始化，模块加载时调用
 * __init 也是一个宏，不同架构定义也不一样，
 * 比如 #define __init __section(.init.text)
 * 它提示内核，这段代码被执行完后，就可以将
 * 占用的内存释放掉。
 * 后面的 __exit 类似。
 */
static int __init led_init(void)
{
    /*
     * KERN_ALERT 表示消息的优先级，
     * 定义参考最底部的注释
     */
    printk(KERN_ALERT "led init\n");
    led_device = kmalloc(sizeof(struct led_dev), GFP_KERNEL);
    led_setup_cdev(led_device, 0);
    return 0;
}

/* 模块卸载时调用 */
static void __exit led_exit(void)
{
    dev_t devno;

    if (led_device != NULL) {
        kfree(led_device);
    }

    devno = MKDEV(231, 0);
    unregister_chrdev_region(devno, 1);
    printk(KERN_ALERT "led exit\n");
}

/*
 * 这两个是特殊的“内核宏”，
 * 用来指示上面两个函数的角色。
 * 初始化和退出函数是非必须的，可以没有。
 */
module_init(led_init);
module_exit(led_exit);

/*
 * #define  KERN_EMERG  "<0>"
 * #define  KERN_ALERT  "<1>"
 * #define  KERN_CRIT   "<2>"
 * #define  KERN_ERR    "<3>"
 * #define  KERN_WARNING    "<4>"
 * #define  KERN_NOTICE "<5>"
 * #define  KERN_INFO   "<6>"
 * #define  KERN_DEBUG  "<7>"
 */
