/**
 * 包含初始化宏定义的头文件，代码中的module_init与module_exit在此文件中
 */
#include <linux/init.h>
/**
 * 包含初始化加载模块的头文件，代码中的MODULE_LICENSE在此头文件中
 */
#include <linux/module.h>
/**
 * 驱动注册头文件，包括驱动注册结构体、注册与卸载函数
 */
#include <linux/platform_device.h>

//驱动注册名称，此名称一定要与注册的设备名称对应
#define DRIVER_NAME "hello_ctl" 

MODULE_LICENSE("Dual BSD/GPL"); //声明是开源的，没有内核版本限制
MODULE_AUTHOR("KQZ"); //声明作者


static int hello_probe(struct platform_device * pdv)
{
	printk(KERN_EMERG "\tthe hello driver probe init -----\n");
	return 0;
}

static int hello_remove(struct platform_device * pdv)
{
	return 0;
}

static void hello_shutdown(struct platform_device * pdv)
{
	//return 0;
}


static int hello_suspend(struct platform_device * pdv,pm_message_t state)
{
	return 0;
}


static int hello_resume(struct platform_device * pdv)
{
	return 0;
}

/**
 * 注册驱动结构体
 */
struct platform_driver hello_driver={
	.probe=hello_probe,
	.remove=hello_remove,
	.shutdown=hello_shutdown,
	.suspend=hello_suspend,
	.resume=hello_resume,
	.driver={
		.name=DRIVER_NAME,
		.owner=THIS_MODULE,
	}
};

/**
 * [驱动加载程序入口函数]
 * @return  [description]
 */
static int hello_init(void)
{
	int driver_state;

    /*
     打印信息，KERN_EMERG 表示紧急信息
     */
    printk(KERN_EMERG "Hello World enter !\n");
    //注册设备驱动
    driver_state=platform_driver_register(&hello_driver);
    printk(KERN_EMERG "the driver register state is %d \n",driver_state);
    return 0;
}

/**
 * [驱动模块卸载程序执行的函数入口]
 */
static void hello_exit(void)
{
	//卸载设备驱动
	platform_driver_unregister(&hello_driver);	
    printk(KERN_EMERG "Hello World exit !\n");
}

module_init(hello_init); //模块入口
module_exit(hello_exit); //模块出口