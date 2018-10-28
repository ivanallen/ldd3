/* 包含大量的符号和函数定义 */
#include <linux/module.h>
/* 初始化和清除函数，但不包含也不出错 */
#include <linux/init.h>

/* 许可证 */
MODULE_LICENSE("Dual BSD/GPL");

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
static int __init hello_init(void)
{
	/*
	 * KERN_ALERT 表示消息的优先级，
	 * 定义参考最底部的注释
	 */
	printk(KERN_ALERT "hello world:%s\n", "allen");
	return 0;
}

/* 模块卸载时调用 */
static void __exit hello_exit(void)
{
	printk(KERN_ALERT "hello world:%s\n", "exit");
}

/*
 * 这两个是特殊的“内核宏”，
 * 用来指示上面两个函数的角色。
 * 初始化和退出函数是非必须的，可以没有。
 */
module_init(hello_init);
module_exit(hello_exit);

/*
 * #define	KERN_EMERG	"<0>"
 * #define	KERN_ALERT	"<1>"
 * #define	KERN_CRIT	"<2>"
 * #define	KERN_ERR	"<3>"
 * #define	KERN_WARNING	"<4>"
 * #define	KERN_NOTICE	"<5>"
 * #define	KERN_INFO	"<6>"
 * #define	KERN_DEBUG	"<7>"
 */
