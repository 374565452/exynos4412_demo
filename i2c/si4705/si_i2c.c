/**
 * linux中对于si4705芯片驱动
 * 	硬件部分：
 * 		si4705复位管脚 CAM_MCLK 相对具体的GPIO管脚为GPJ1_3
 * 		I2C连接的是芯片中的I2C7总线（第7组I2C总线）
 */
/*该驱动针对已知总线编号的，总线上的多个设备*/
#include <linux/i2c.h>
#include <linux/list.h> 
#include <linux/delay.h>

/*包含初始化宏定义的头文件,代码中的module_init和module_exit在此文件中*/
#include <linux/init.h>
/*包含初始化加载模块的头文件,代码中的MODULE_LICENSE在此头文件中*/
#include <linux/module.h> 
/*定义module_param module_param_array的头文件*/
#include <linux/moduleparam.h>
/*定义module_param module_param_array中perm的头文件*/
#include <linux/stat.h>
/*三个字符设备函数*/
#include <linux/fs.h>
/*MKDEV转换设备号数据类型的宏定义*/
#include <linux/kdev_t.h>
/*定义字符设备的结构体*/
#include <linux/cdev.h>
/*分配内存空间函数头文件*/
#include <linux/slab.h>
/*包含函数device_create 结构体class等头文件*/
#include <linux/device.h>
/*包含copy_to_user和copy_from_user的头文件*/
#include <asm/uaccess.h>

/*设置GPIO状态，上下拉，输入输出就，复用等等相关函数*/
#include <plat/gpio-cfg.h>
/*包含GPIO端口的宏定义*/
#include <mach/gpio-exynos4.h>
/*linux系统提供的申请端口函数和设置端口状态的函数，通过这些函数可以避免端口使用的冲突，但是这些函数在某些系统里面不支持*/
#include <linux/gpio.h>

/*linux系统中的中断资源申请和释放函数，这是一个对于底层的接口，不包括具体实现*/
#include <linux/interrupt.h>
/*中断触发方式的定义*/
#include <linux/irq.h>

/*linux等待队列相关函数和宏*/
#include <linux/wait.h>
/*等待方式:TASK_INTERRUPTIBLE,TASK_UNINTERRUPTIBLE,TAST_KILLABLE等宏定义的头文件*/
#include <linux/sched.h>

/*包含定时器寄存器宏定义的头文件*///里面包含的是s3c2440的定义，在这里二者是公用的*/
#include <plat/regs-timer.h>
/*包含寄存器操作函数的头文件*/
#include <asm/io.h>
/*最终定位到map-base.h包含定时器寄存器虚拟地址和实际物理地址转换*/
#include <mach/regs-irq.h>
/*clk相关函数*/  
#include <linux/clk.h>

/*延迟函数*/
#include <linux/delay.h>

//#define MY_PRINTF_DEBUG_ON    //注销掉可以关闭myprintf
#ifdef MY_PRINTF_DEBUG_ON
/*#define myprintf(fmt,args...) do{printk(KERN_EMERG ""fmt,##args);}while(0)*/
#define myprintf(x...) do{printk(KERN_EMERG ""x);}while(0)								
#else
/*#define myprintf(fmt,args...)*/
#define myprintf(x...)
#endif

/*以下包含在i2c-core.h*/
struct i2c_devinfo {
	struct list_head	list;
	int			busnum;
	struct i2c_board_info	board_info;
};
extern struct rw_semaphore	__i2c_board_lock;
extern struct list_head	__i2c_board_list;  //i2c内核设备链表，通过该链表可以找到每条总线上的i2c设备信息，具体
										   //方法是，用list_entry得到每个设备对应的i2c_devinfo指针，然后由该结构体的
										   //busnum(i2c总线号码)定位到具体的总线，最后得到总线上的设备，注意，该链表只要有设备就会
										   //增加长度，而不管设备是否具有相同的i2c总线号，所以必须遍历整个链表才能定位到同一条总线上的
										   //所有设备。可以参考mach-itop4412.c里面的i2c_register_board_info函数										   
/*以上包含在i2c-core.h*/

#define MAJOR_NUM 	0   //默认主设备号
#define MINOR_NUM 	0   //默认次设备起始号
#define _P_SIZE 	64  //设备数据缓冲区大小，供dev_data使用,read和write的count不能超过这个值
///ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))   //数组元素个数  在某个头文件里面有

#define CHAR_DRIVER_NAME "si4705_driver"
#define CLASS_NAME "si4705_class"
#define DEV_NAME "si4705"   //设备节点名称 
#define BUS_NUM  7  //挂载i2c设备的总线编号，看芯片手册

int DEV_NUM = 0; 	  //设备数量
int major = MAJOR_NUM;
int minor_begin = MINOR_NUM;

MODULE_AUTHOR("KQZ");
MODULE_DESCRIPTION("driver framework");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("V1.0");

static const struct i2c_device_id i2c_dev_id[] = {
	{ DEV_NAME,0},//用到哪些就声明哪些内容，比如driver_data用不到，所以这里就写0,这里的DEV_NAME要和注册的设备相同
	{}//总线上有两个或者多个设备的时候这里也只需要对一个设备进行说明就行，只要保证和总线上设备名字
	};
unsigned int CAP[] = {1024,512};   //i2c设备容量表（字节为单位）

struct _i2c_dev {
	struct cdev 		 i2c_cdev;   //字符型设备结构体，系统已经定义好
	char  				 *i2c_dev_data;	 //设备数据(自己定义)	
	struct i2c_client 	 *client;
	unsigned int 		 capacity;   //i2c设备容量（地址的最大值）
	char 				 chip_address;
	//锁机制
	//同步机制
	//其他
}*i2c_dev = NULL;     
struct class 		 *i2c_class; //设备按类区分，挂在同一条总线上的多个设备也只有一个类

/*打开操作*/
static int hardware_init(void)
{
	return 0;
}
static void hardware_free(void)
{
	;
}
/*i2c  读取n个数据*/
static int i2c_read_n_data(struct i2c_client *client, char *rxdata, int length, unsigned int data_addr,char chip_addr) 
{
	int ret;	
	unsigned char temp = data_addr%256;//16位地址，对应i2c器件最大存储空间为64k，注意这里不是i2c芯片的地址，而是用于读取存储数据的地址

	struct i2c_msg msg[2] = {
		{
			.addr	= chip_addr+data_addr/256,//基地址加存储阵列地址字地址
			.flags	= 0,
			.len	= 1,
			.buf	= &temp,
		},
		{
			.addr	= chip_addr+data_addr/256,//基地址加存储阵列地址字地址
			.flags	= I2C_M_RD,
			.len	= length,
			.buf	= rxdata,
		},
	};	
	
	myprintf(KERN_EMERG "chip address :%d  data_addr = %d \n",chip_addr,msg[0].buf[0]);
	myprintf(KERN_EMERG "adater num:%d\n",client->adapter->nr);
	
	udelay(1500);//在我的板子上如果不加这个延时，否掉myprintf以后执行read时就会报错
	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret < 0)
		printk(KERN_EMERG "%s: i2c read error: %d    %d line\n", __FILE__,ret,__LINE__);
	return ret;
}

/*i2c 写入n个数据*/
static int i2c_write_n_data(struct i2c_client *client, char *txdata, int length, unsigned int data_addr,char chip_addr)
{
	int ret;
	struct i2c_msg msg;	
	unsigned char temp = data_addr%256;
	
	msg.buf = kmalloc(1 + length,GFP_KERNEL); 

	if(!msg.buf)
		goto malloc_fail;
	
	msg.addr	= chip_addr+data_addr/256;//基地址加存储阵列地址字地址
	msg.flags	= 0;
	msg.len 	= 1 + length;	
	msg.buf[0]	= temp;	
	
	memcpy(&msg.buf[1], txdata, length);
	
	myprintf(KERN_EMERG "chip address num:%d\n",chip_addr);
	myprintf(KERN_EMERG "adater num:%d\n",client->adapter->nr);
	
	ret = i2c_transfer(client->adapter, &msg, 1);//基地址加存储阵列地址字地址
	
	kfree(msg.buf);	
	if (ret < 0)
		printk(KERN_EMERG "%s: i2c write error: %d   %d line\n", __FILE__,ret,__LINE__);	
	
	return ret;
	malloc_fail:
		return -1;
}

static int i2c_open(struct inode *inode, struct file *file)
{
	myprintf(KERN_EMERG "i2c_open is called!\n");		
	file->private_data = &i2c_dev[MINOR(inode->i_rdev)];  //通过次设备号定位到不同的设备，这里的file->private_data每个次设备都不同，
														  //这是用一个read,write,ioctl函数能够完成所有设备操作的基础，每次进入这些函数所对应的file是不同的
	return 0;											  //通过file->f_dentry->d_inode->i_rdev也可以得到设备号
	
}
/*关闭操作*/
static int i2c_release(struct inode *inode, struct file *file)
{
	myprintf(KERN_EMERG "i2c_release is called!\n");	
	return 0;
}
/*ioctl操作,可以用这个函数改变adaper的一些参数，比如i2c速率等待*/

static long i2c_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	struct _i2c_dev *dev;
	myprintf(KERN_EMERG "i2c_ioctl is called!\n");
	dev = (struct _i2c_dev *)file->private_data;   //由于每次进入read函数的句柄不一样，因此对应的file->private_data也不一样	
	switch (cmd)
	{
		case 0: break;
		case 1: break;
		default :break;
	}
	return ret;
}
/*从固定的地方开始读取数据，如果想改变起始地址，用lseek*/
ssize_t i2c_read(struct file *file, char __user *buf, size_t count, loff_t *f_pos)
{
	struct _i2c_dev *dev;
	myprintf(KERN_EMERG "i2c_read is called!\n");
	dev = (struct _i2c_dev *)file->private_data;   //由于每次进入read函数的句柄不一样，因此对应的file->private_data也不一样	
	if( i2c_read_n_data(dev->client,dev->i2c_dev_data,count,*f_pos,dev->chip_address)<0 )
		return -EFAULT;
	return copy_to_user(buf,dev->i2c_dev_data,count);
}
/*从固定的地方开始写入数据，如果想改变起始地址，用lseek*/
ssize_t i2c_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct _i2c_dev *dev;
	myprintf(KERN_EMERG "i2c_write is called!\n");
	dev = (struct _i2c_dev *)file->private_data;   //由于每次进入write函数的句柄不一样，因此对应的file->private_data也不一样
    
	if ( copy_from_user(dev->i2c_dev_data,buf,count) )
		return - EFAULT;
	return  i2c_write_n_data(dev->client,dev->i2c_dev_data,count,*f_pos,dev->chip_address);
}
/*定位读取的首地址*/
loff_t i2c_lseek(struct file *file, loff_t offset, int orig)
{
	loff_t ret = 0;
	struct _i2c_dev *dev;
	myprintf(KERN_EMERG "i2c_lseek is called!\n");	
	dev = (struct _i2c_dev *)file->private_data;   //由于每次进入write函数的句柄不一样，因此对应的file->private_data也不一样
	switch (orig)
	{
		case 0: /*从文件开头开始偏移*///用户态用SEEK_SET
			if (offset < 0)
			{
				ret = - EINVAL;
				break;
			}
			if ((unsigned int)offset > dev->capacity) //偏移越界
			{
				ret = - EINVAL;				
			}
			file->f_pos = offset;
			ret = file->f_pos;
			break;
		case 1: /*从当前位置开始偏移*///用户态用SEEK_CUR			
			if ((file->f_pos + offset) > dev->capacity) //偏移越界
			{
				ret = - EINVAL;
				break;
			}
			if ((file->f_pos + offset) < 0)
			{
				ret = - EINVAL;
				break;
			}
			file->f_pos = file->f_pos + offset;
			ret = file->f_pos;
			break;
		case 2:/*从末尾位置开始偏移*///用户态用SEEK_END
			break;
		default:ret = - EINVAL;break;
	}	
			return ret;
}	

struct file_operations i2c_fops = {
		.owner	= THIS_MODULE,
		.read 	= i2c_read,
		.llseek = i2c_lseek,
		.write	= i2c_write,
		.open	= i2c_open,
.unlocked_ioctl = i2c_ioctl,
	   .release = i2c_release,
};

static int i2c_remove(struct i2c_client *client);
static void i2c_shutdown(struct i2c_client *client);
static int i2c_suspend(struct i2c_client *client, pm_message_t mesg);
static int i2c_resume(struct i2c_client *client);
static int i2c_probe(struct i2c_client *client, const struct i2c_device_id *device_id);
static struct of_device_id mpu6050_dt_match[] = {  
    {.compatible = "invense,si4705" },  
    {/*northing to be done*/},  
};  
struct i2c_driver i2c_dev_driver = {
	.driver 	= {
					.name = DEV_NAME,
					.owner = THIS_MODULE,	
					//.of_match_table=mpu6050_dt_match,	
				  },
	.probe 		= i2c_probe,
	.remove 	= i2c_remove,	
	.shutdown 	= i2c_shutdown,
	//.suspend 	= i2c_suspend,
	//.resume 	= i2c_resume,
	.id_table   = i2c_dev_id,
};

static int i2c_probe(struct i2c_client *client, const struct i2c_device_id *device_id)
{
printk(KERN_EMERG "i2c_probe is called!\n");
	int ret = -1,i = 0;
	int j = 0;
	int num_of_dev = 0/*设备的数量*/;
	char *addr_tab = NULL;
	struct list_head *detect_list;
	struct i2c_devinfo *devinfo;  
printk(KERN_EMERG "i2c_probe is called!\n");
	addr_tab =(char *)kmalloc(127,GFP_KERNEL); //一条总线上最多挂127个设备
	if(!addr_tab)  
		goto fail;
	
	printk(KERN_EMERG "Bus num:%d\n",client->adapter->nr);
	dev_t dev_num = MKDEV(major,minor_begin);  //因为存在静态申请设备号的可能，因此首先构造设备号
	/*寻找对应总线上的设备*///这里可以得到i2c器件的地址，因此对于挂载在同一条i2c总线上的多个设备的访问，可以在这里做文章
	down_read(&__i2c_board_lock);
	list_for_each(detect_list, &__i2c_board_list)
	{
		devinfo = list_entry(detect_list, struct i2c_devinfo, list);
		if( devinfo->busnum == BUS_NUM )		
		{
			printk(KERN_EMERG"Device %d:%s\n",num_of_dev,devinfo->board_info.type);
			addr_tab[i] = devinfo->board_info.addr;
			num_of_dev++;
			i++;
		}		
	}
	i = 0;
	up_read(&__i2c_board_lock);
	DEV_NUM = num_of_dev;
	if(DEV_NUM > 1)
		printk(KERN_EMERG "There are %d devices on i2c-bus %d!\n",DEV_NUM,BUS_NUM);
	else if(DEV_NUM == 1)
			printk(KERN_EMERG "There is %d device on i2c-bus %d!\n",DEV_NUM,BUS_NUM);
		 else
		 {
			printk(KERN_EMERG "No device on i2c_bus,exit!\n");
			goto fail;	 
		 }
	/*寻找对应总线上的设备*/
	
	if(num_of_dev == 0)
	{
		printk(KERN_EMERG"there is no device on i2c-bus,exit!\r\n");
		goto fail;	
	}	
	/*hardware initial*/
	if(hardware_init()<0)
	{
		printk(KERN_EMERG "Hardware initial goes wrong! exit! %s\n",__FILE__);
		goto fail;
	}	
	if(major)
		ret = register_chrdev_region(dev_num,DEV_NUM,CHAR_DRIVER_NAME);   //静态申请设备号
	else
	{
		ret = alloc_chrdev_region(&dev_num,minor_begin,DEV_NUM,CHAR_DRIVER_NAME);
		major = MAJOR(dev_num);		//动态申请后需要取出主设备号，由于次设备基号(第一个次设备号)以及设备数已知，因此不需要取出次设备号
	}
	if(ret < 0)
	{
		printk(KERN_EMERG "Failed to regist! %s\n",__FILE__);
		return -1;
	}
	i2c_dev = kmalloc(DEV_NUM*sizeof(struct _i2c_dev),GFP_KERNEL);  //上面直接定义了设备指针，所以这里需要分配内存,
	if(i2c_dev==NULL)											//这样做在驱动卸载以后可以释放内存,达到节约内存的目的
	{
		printk(KERN_EMERG "Failed to malloc-1! %s\n",__FILE__);
		goto malloc_fail;
	}
	else
		memset(i2c_dev,'\0',DEV_NUM*sizeof(struct _i2c_dev));
	i2c_class = class_create(THIS_MODULE,CLASS_NAME);       //创建设备类,多个设备也只有一个类
	 
	for(i=0;i<DEV_NUM;i++)	//i2c_dev++由于指针进行了DEV_NUM次自加，并且后面程序还会用到，因此需要进行减操作，
	{						//推荐用下标的方法，这样可以避免指针加减出现错误 比如i2c_dev[i]
		i2c_dev[i].i2c_dev_data = kmalloc(_P_SIZE,GFP_KERNEL);    //设备结构体里面还有指针，还需要分配内存
		if(i2c_dev[i].i2c_dev_data==NULL)
		{
			printk(KERN_EMERG "Failed to malloc-2! %s\n",__FILE__);
			goto malloc_fail_1;
		}	
		memset(i2c_dev[i].i2c_dev_data,'0',_P_SIZE);		
		i2c_dev[i].client = client;//这里用client仅仅是为了后面使用adapter,实际上每个设备对应的i2c_client结构体指针时可以得到的，只是现在我还没有找到得到指针的办法
		i2c_dev[i].capacity = CAP[i];
		i2c_dev[i].chip_address = addr_tab[i];
		
		cdev_init(&i2c_dev[i].i2c_cdev,&i2c_fops);//设备注册三部曲:建立cdev与fops联系，初始化cdev.owner，调用cdev_add注册设备
		i2c_dev[i].i2c_cdev.owner = THIS_MODULE;
		if(cdev_add(&i2c_dev[i].i2c_cdev,MKDEV(major,minor_begin+i),DEV_NUM))
			printk(KERN_EMERG "Failed to do cdev_add %d!\n",i);
		device_create(i2c_class,NULL,MKDEV(major,minor_begin+i),NULL,DEV_NAME"-0x%x",i2c_dev[i].chip_address); //生成设备节点，必须在class_create后进行		
	}
	kfree(addr_tab);
	return 0;	
	fail:		
		return -1;
	malloc_fail:		
		unregister_chrdev_region(MKDEV(major,minor_begin),DEV_NUM);
		kfree(addr_tab);
		return -1;
	malloc_fail_1:		
		hardware_free();
		if(i>=1)
			for(j=0;j<i-1;j++)//注意注销函数的调用顺序，遵循的原则为:先调用的后释放，后调用的先释放，比如:内存一定要后分配的先释放
			{
				device_destroy(i2c_class,MKDEV(major,minor_begin+j));
				cdev_del(&(i2c_dev[j].i2c_cdev));		
				kfree(i2c_dev[j].i2c_dev_data);		
			}
		class_destroy(i2c_class);
		kfree(i2c_dev);			
		unregister_chrdev_region(MKDEV(major,minor_begin),DEV_NUM);
		kfree(addr_tab);
		return -1;
}
static int i2c_remove(struct i2c_client *client)
{
	printk(KERN_EMERG "i2c_remove is called!\n");
	return 0;
}
static void i2c_shutdown(struct i2c_client *client)
{
	myprintf(KERN_EMERG "i2c_shutdown is called!\n");
}
static int i2c_suspend(struct i2c_client *client, pm_message_t mesg)
{
	myprintf(KERN_EMERG "i2c_suspend is called!\n");
	return 0;
}
static int i2c_resume(struct i2c_client *client)
{
	myprintf(KERN_EMERG "i2c_resume is called!\n");
	return 0;
}

static int __init i2c_init(void)
{	
	printk(KERN_EMERG "i2c enter!\n");
	
	i2c_add_driver(&i2c_dev_driver);
	return 0; 
}
static void __exit i2c_exit(void)
{
	int i = 0;
		//hardware free
	hardware_free();
	for(i=0;i<DEV_NUM;i++)//注意注销函数的调用顺序，遵循的原则为:先调用的后释放，后调用的先释放，比如:内存一定要后分配的先释放
	{
		device_destroy(i2c_class,MKDEV(major,minor_begin+i));
		cdev_del(&(i2c_dev[i].i2c_cdev));		
		kfree(i2c_dev[i].i2c_dev_data);		
	}
	class_destroy(i2c_class);
	kfree(i2c_dev);	
	unregister_chrdev_region(MKDEV(major,minor_begin),DEV_NUM);
	i2c_del_driver(&i2c_dev_driver);
	printk(KERN_EMERG "i2c exit!\n");
}
module_init(i2c_init);
module_exit(i2c_exit);
//module_i2c_driver(i2c_dev_driver);
