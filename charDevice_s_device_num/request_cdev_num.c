//简单模块的加载与卸载 -------静态申请设备号，在模块加载过程中向其传入指定的主次设备号
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

/*三个字符设备函数*/
#include <linux/fs.h>
/*MKDEV转换设备号数据类型的宏定义*/
#include <linux/kdev_t.h>
/*定义字符设备的结构体*/
#include <linux/cdev.h>

MODULE_LICENSE("Dual BSD/GPL"); //声明是开源的，没有内核版本限制
MODULE_AUTHOR("KQZ"); //声明作者


#define DEVICE_NAME "scdev"
#define CDEV_MAJOR 0   //主设备号
#define CDEV_MINOR 0   //次设备号
#define DEVICE_MINOR_NUM 2

static int module_arg1 ,module_arg2;
static int module_args[50];
static int module_args_nums;

int dev_major=CDEV_MAJOR;
int dev_minor=CDEV_MINOR;

/**
 * module_param参数说明
 * name 指的是 参数名
 * type  参数类型
 * perm  权限类型 S_IRUSR所有文件所有者可读
 */
module_param(dev_major,int,S_IRUSR);
module_param(dev_minor,int,S_IRUSR);




/**
 * [驱动加载程序入口函数]
 * @return  [description]
 */
static int char_device_init(void)
{
	int i ;
	int ret;
	dev_t num_dev;
	printk(KERN_EMERG"the major is %d\n",dev_major);
	printk(KERN_EMERG"the minor is %d \n",dev_minor);
	//如果外部有传major参数过来，则就可以用register_chrdev_region来申请设备号
	//register_chrdev_region当已经知道设备的主次设备号，再去申请设备号
	if(dev_major)
	{
		num_dev=MKDEV(dev_major,dev_minor);
		//此函数第二个参数，代表需要申请多少个设备
		ret=register_chrdev_region(num_dev,DEVICE_MINOR_NUM,DEVICE_NAME);
	}
	else
	{
		printk(KERN_EMERG"dev_major is %d ,dev_minor is %d \n",dev_major,dev_minor);
	}
	if(ret<0)
	{
		printk(KERN_EMERG"request the num_dev number is failed\n");
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
    //释放掉设备号
    unregister_chrdev_region(MKDEV(dev_major,dev_minor),DEVICE_MINOR_NUM);
}

module_init(char_device_init); //模块入口
module_exit(char_device_exit); //模块卸载函数
