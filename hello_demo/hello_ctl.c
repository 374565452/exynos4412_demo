/**
 * 包含初始化宏定义的头文件，代码中的module_init与module_exit在此文件中
 */
#include <linux/init.h>
/**
 * 包含初始化加载模块的头文件，代码中的MODULE_LICENSE在此头文件中
 */
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL"); //声明是开源的，没有内核版本限制
MODULE_AUTHOR("KQZ"); //声明作者

/**
 * [驱动加载程序入口函数]
 * @return  [description]
 */
static int hello_init(void)
{
    /*
     打印信息，KERN_EMERG 表示紧急信息
     */
    printk(KERN_EMERG "Hello World enter !\n");
    return 0;
}

/**
 * [驱动模块卸载程序执行的函数入口]
 */
static void hello_exit(void)
{
    printk(KERN_EMERG "Hello World exit !\n");
}

module_init(hello_init); //模块入口
module_exit(hello_exit); //模块出口