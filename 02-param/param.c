#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");

static char *whom = "world";
static int howmany = 1;

/*
 * module_param 定义在 include/linux/moduleparam.h 中
 * module_param(name, type, perm)
 *
 * perm 参数用来控制谁能够访问 sysfs 中对模块参数的表述
 */
module_param(howmany, int, 0664);
module_param(whom, charp, 0664);

/*
 * 内核支持的模块参数类型如下：
 * bool
 * invbool
 *     反转 bool，如果你传 true，结果就变 false.
 * charp
 *     字符指针值，内核负责为用户提供的字符串分
 *     配内存，并相应设置指针
 * int
 * long
 * short
 * uint
 * ulong
 * ushort
 */

static int __init param_init(void)
{
	int i = 0;

	for (i = 0; i < howmany; ++i)
		printk(KERN_ALERT "hello %s\n", whom);
	return 0;
}

static void __exit param_exit(void)
{
	printk(KERN_ALERT "param exit\n");
}

module_init(param_init);
module_exit(param_exit);
