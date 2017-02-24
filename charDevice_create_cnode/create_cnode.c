//简单模块的加载与卸载 -------生成字符类设备节点
//需要做2件事情，
//1）创建设备节点类结构体对象
//2）生成设备节点
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

/*分配内存空间所用到的头文件*/
#include <linux/slab.h>
/*包含函数device_create 结构体class等头文件*/
#include <linux/device.h>



MODULE_LICENSE("Dual BSD/GPL"); //声明是开源的，没有内核版本限制
MODULE_AUTHOR("KQZ"); //声明作者


#define DEVICE_NAME "scdev"
#define CDEV_MAJOR 0   //主设备号
#define CDEV_MINOR 0   //次设备号
#define DEVICE_MINOR_NUM 2
#define REGDEV_SIZE 3000

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
 * 定义注册设备所需要的cdev结构体变量
 */
struct reg_dev
{
	char * data;
	unsigned long size;
	struct cdev reg_dev;
};
struct reg_dev * reg_devs;

//声明一个设备类结构体指针
static struct class * reg_class; 

/**
 * 定义注册设备所需要的file_operations结构体
 */
struct file_operations reg_fos=
{
	.owner=THIS_MODULE,
};

static int reg_init_cdev(struct reg_dev *dev,int index)
{
	int err;
	int deviceNode=MKDEV(dev_major,dev_minor+index);
	//cdev数据初始化
	cdev_init(&dev->reg_dev,&reg_fos);
	dev->reg_dev.owner=THIS_MODULE;

	//将设备添加到系统中
	err = cdev_add(&dev->reg_dev,deviceNode,1);
	if(err)
	{
		//如果cdev_add返回值为0 ，则说明向系统中加入设备失败
		printk(KERN_EMERG"cdev add the device is failed \n");
	}else
	{
		printk(KERN_EMERG"cdev add the device is success ,the dev_minor is %d \n",dev_minor+index);
	}
	return err;
}

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
		//动态申请设备号
		ret=alloc_chrdev_region(&num_dev,dev_minor,DEVICE_MINOR_NUM,DEVICE_NAME);
		dev_major=MAJOR(num_dev); //获取主设备号
		dev_minor=MINOR(num_dev); //获取次设备号
		printk(KERN_EMERG"dev_major is %d ,dev_minor is %d \n",dev_major,dev_minor);
	}
	if(ret<0)
	{
		printk(KERN_EMERG"request the num_dev number is failed\n");
	}
	//创建设备类结构体指针,将会在/sys/class下生成名称为DEVICE_NAME的文件，此案例中生成scdev
	reg_class=class_create(THIS_MODULE,DEVICE_NAME);
	/**
	 * 申请完设备号后，需进行向内核申请内存，然后进行内存的初始化操作
	 */
	reg_devs=kmalloc(DEVICE_MINOR_NUM*sizeof(struct reg_dev),GFP_KERNEL);
	if(!reg_devs)
	{
		ret=-ENOMEM;
		goto Failed;
	}
	//申请成功后，将所申请到的内存进行置0操作
	memset(reg_devs,0,DEVICE_MINOR_NUM*sizeof(struct reg_dev));
	//设备初始化操作，因为DEVICE_MINOR_NUM为2，进行两次设备申请
	for(i=0;i<DEVICE_MINOR_NUM;i++)
	{
		//进行DATA内存申请
		reg_devs[i].data=kmalloc(REGDEV_SIZE,GFP_KERNEL);
		memset(reg_devs[i].data,0,REGDEV_SIZE);
		reg_init_cdev(&reg_devs[i],i);
		//创建好cdev_t后，需要生成字符类设备节点
		//会在/dev/下生成两个设备节点信息，此案例中会生成/dev/scdev1 与/dev/scdev2
		device_create(reg_class,NULL,MKDEV(dev_major,dev_minor+i),NULL,DEVICE_NAME"%d",i);
	}

    /*
     打印信息，KERN_EMERG 表示紧急信息
     */
    printk(KERN_EMERG "Hello World enter !\n");
    return 0;
 Failed:
 	unregister_chrdev_region(MKDEV(dev_major,dev_minor),DEVICE_MINOR_NUM);
 	printk(KERN_EMERG"kmalloc is failed ,the ret is %d \n ",ret);
 	return ret;
}

/**
 * [驱动模块卸载程序执行的函数入口]
 */
static void char_device_exit(void)
{
	int i;
    printk(KERN_EMERG "Hello World exit !\n");
    //进行reg_devs释放前，先将data内存释放掉
    for(i=0;i<DEVICE_MINOR_NUM;i++)
	{
		//删除此设备节点信息
		device_destroy(reg_class,MKDEV(dev_major,dev_minor+i));
		printk(KERN_EMERG"cdev del the reg_dev the i is %d \n",i);
		//因为向系统中cdev_add操作，所以在模块卸载时，将device从系统中删除掉
		cdev_del(&(reg_devs[i].reg_dev)) ;
		printk(KERN_EMERG"del the reg_dev data memory the i is %d \n",i);
		kfree(reg_devs[i].data);
	}

	/*释放设备class*/
	class_destroy(reg_class);
	printk(KERN_EMERG"del the reg_dev memory \n");
    //进行内存的释放
    kfree(reg_devs);



    //释放掉设备号
    unregister_chrdev_region(MKDEV(dev_major,dev_minor),DEVICE_MINOR_NUM);
}

module_init(char_device_init); //模块入口
module_exit(char_device_exit); //模块卸载函数
