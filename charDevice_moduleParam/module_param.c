//简单模块的加载与卸载 -------模块加载中 向模块中传参数
/**
 * 包含初始化宏定义的头文件，代码中的module_init与module_exit在此文件中
 */
#include <linux/init.h>
/**
 * 包含初始化加载模块的头文件，代码中的MODULE_LICENSE在此头文件中
 */
#include <linux/module.h>
/**
 * 参数宏定义参数头文件
 */
#include <linux/moduleparam.h>
/**
 * 权限宏定义头文件
 */
#include <linux/stat.h>

MODULE_LICENSE("Dual BSD/GPL"); //声明是开源的，没有内核版本限制
MODULE_AUTHOR("KQZ"); //声明作者

static int module_arg1 ,module_arg2;
static int module_args[50];
static int module_args_nums;
/**
 * module_param参数说明
 * name 指的是 参数名
 * type  参数类型
 * perm  权限类型 S_IRUSR所有文件所有者可读
 */
module_param(module_arg1,int,S_IRUSR);
module_param(module_arg2,int,S_IRUSR);

/**
 * module_param_array参数说明
 * name 指的是 参数名
 * type 参数类型
 * nump 指的是 保存参数个数的地址
 * perm 权限类型 S_IRUSR所有文件所有者可读
 */
module_param_array(module_args,int ,&module_args_nums,S_IRUSR);
/**
 * [驱动加载程序入口函数]
 * @return  [description]
 */
static int char_device_init(void)
{
	int i ;
	printk(KERN_EMERG"the module_arg1 is %d \n",module_arg1);
	printk(KERN_EMERG"the module arg2 is %d \n",module_arg2);
	for(i=0;i<module_args_nums;i++)
	{
		printk(KERN_EMERG "int_array[%d] is %d!\n",i,module_args[i]);
	}
    /*
     打印信息，KERN_EMERG 表示紧急信息
     */
    printk(KERN_EMERG "Hello World enter !\n");
    return 0;
}

/**
 * [驱动模块卸载程序执行的函数入口]
 */
static void char_device_exit(void)
{
    printk(KERN_EMERG "Hello World exit !\n");
}

module_init(char_device_init); //模块入口
module_exit(char_device_exit); //模块卸载函数