/**
 * Linux中利用中断方式来完成按键的功能
 * 硬件为：开发板中的home、back按键，在原理图中对应的网络号为：UART_RING ,SIM_DET
 * 相对应的4412管脚为GPX1_1,GPX1_2 ，从datasheet中找到GPX1CON找到与之对应的KP_COL[1] KP_COL[2]
 * 然后找到相应的中断号为XEINT_9 XEINT_10
 *
 * 开发板中，SLEEP VOL+  VOL-相对应的管脚为GPX3_3 GPX2_1 GPX2_0 与之对应的 KP_ROW[11]  KP_ROW[1]  KP_ROW[0]
 * 与之相对应的中断号为  XEINT_27 XEINT_17  XEINT_16
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

/*中断处理函数所在头文件*/
#include <linux/interrupt.h>
#include <linux/irq.h>

#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
/*三星平台4412平台，GPIO宏定义头文件*/
#include <mach/gpio-exynos4.h>


//异步调用所用的头文件
#include <linux/wait.h>  
#include <linux/sched.h>  
#include <asm/uaccess.h> 
#include <linux/poll.h>

MODULE_LICENSE("Dual BSD/GPL"); //声明是开源的，没有内核版本限制
MODULE_AUTHOR("KQZ"); //声明作者


#define DEVICE_NAME "k_buttons"
#define CDEV_MAJOR 0   //主设备号
#define CDEV_MINOR 0   //次设备号
#define DEVICE_MINOR_NUM 1
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


static int key_gpios[] = {
	EXYNOS4_GPX1(1),
	EXYNOS4_GPX1(2),
};
#define KEY_NUM ARRAY_SIZE(key_gpios)
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

struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};

//当按键按下时为0x01,0x02
//当按键弹起时为0x81,0x82
struct pin_desc pins_desc[2]={
	{EXYNOS4_GPX1(1),0X01},
	{EXYNOS4_GPX1(2),0x02},

};
//按键CODE
static char key_code;

//申明一个等待队列
//static wait_queue_head_t wq;  
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);  
  
/* 中断事件标志, 中断服务程序将它置1，fifth_drv_read将它清0 */  
static volatile int ev_press = 0;  

static struct fasync_struct *button_async;  
/**
 * 只需要一个中断处理函数就可以拉
 * 这里可以设置一个按键值对应表，当按键按下时对应一个值，当按键弹起时对应一个值
 * @param  irq    [description]
 * @param  dev_id [description]
 * @return        [description]
 */
static irqreturn_t key_interrupt(int irq, void *dev_id)
{
	//printk(KERN_EMERG"the home key button is pressed \r\n");

	//进入中断后
	struct pin_desc * pin_src=(struct pin_desc *) dev_id;

	unsigned int key_val;
	//key_val=s3c_gpio_getpin(pin_src->pin);
	//利用下面的api函数来读取gpio管脚的值
	key_val=gpio_get_value(pin_src->pin);
	if(key_val==0)
	{
		key_code=pin_src->key_val;
	}else
	{
		key_code=pin_src->key_val | 0x80;
	}

	ev_press = 1;                  /* 表示中断发生了 */  
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */  
      
        //发送信号SIGIO信号给fasync_struct 结构体所描述的PID，触发应用程序的SIGIO信号处理函数  
    kill_fasync (&button_async, SIGIO, POLL_IN);  

	//s3c2410_gpio_getpin
	return IRQ_HANDLED;
}


/**
 * 打开操作
 * 打开驱动时，进行中断的申请
 * @param  inode [description]
 * @param  file  [description]
 * @return       [description]
 */
static int key_open(struct inode *inode, struct file *file){
	int ret;
	printk(KERN_EMERG "key button open  is success!\n");
	//进行中断的申请操作
	//申请成功后，正常返回的值为0
	ret = request_irq(IRQ_EINT(9),key_interrupt,IRQ_TYPE_EDGE_FALLING,"HOME_INT",&pins_desc[0]);
	if(ret <0)
	{
		printk(KERN_EMERG"request the irq is failed the irq is %d,the ret is %d \r\n",IRQ_EINT(9),ret);
		return ret;
	}
	printk(KERN_EMERG"request the irq is success the irq is %d,the ret is %d \r\n",IRQ_EINT(9),ret);
	ret = request_irq(IRQ_EINT(10),key_interrupt,IRQ_TYPE_EDGE_FALLING,"BACK_INT",&pins_desc[1]);
	if(ret <0)
	{
		printk(KERN_EMERG"request the irq is failed the irq is %d,the ret is %d \r\n",IRQ_EINT(10),ret);
		return ret;
	}
	printk(KERN_EMERG"request the irq is success the irq is %d,the ret is %d \r\n",IRQ_EINT(10),ret);
	return 0;
}
/*关闭操作*/
static int key_close(struct inode *inode, struct file *file){
	printk(KERN_EMERG "key button close is success!\n");
	//将申请的中断释放掉
	//之前传入的参数为reg_devs，应该修改为&pins_desc[0]
	//因为申请的时候，传入的参数为&pin_desc[0]所以应该释放时传入参数与申请时传入参数一致
	free_irq(IRQ_EINT(9),&pins_desc[0]);
	free_irq(IRQ_EINT(10),&pins_desc[1]);
	return 0;
}
/*IO操作*/
static long key_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	printk(KERN_EMERG "key button ioctl  is success! cmd is %d,arg is %d \n",cmd,arg);
	
	return 0;
}

//上位机程序通过此函数来获取按下的键值
ssize_t key_read(struct file *file, char __user *buf, size_t count, loff_t *f_ops){
	if (count != 1)  
        return -EINVAL;  
  
    /* 如果没有按键动作, 休眠 */  
    //等待按键被按下时，执行动作，这里就是等待ev_press值为1
    wait_event_interruptible(button_waitq, ev_press);  
  
    /* 如果有按键动作, 返回键值 */  
    long l =copy_to_user(buf, &key_code, 1);
    ev_press = 0;  
    if(l>0){
    	return 1;
    }  else
    {
    	return 0;
    }
    
      
    
	//return 0;
}

ssize_t key_write(struct file *file, const char __user *buf, size_t count, loff_t *f_ops){
	return 0;
}

loff_t key_llseek(struct file *file, loff_t offset, int ence){
	return 0;
}
//设置select epoll是阻塞还是非阻塞
static unsigned key_poll(struct file *file, poll_table *wait)  
{  
    unsigned int mask = 0;  
    poll_wait(file, &button_waitq, wait); // 不会立即休眠  
  
    if (ev_press)  
        mask |= POLLIN | POLLRDNORM;  
  
    return mask;  
}  
  
static int key_fasync (int fd, struct file *filp, int on)  
{  
    printk(KERN_EMERG"key_fasync\n");  
    //初始化/释放 fasync_struct 结构体 (fasync_struct->fa_file->f_owner->pid)  
    return fasync_helper (fd, filp, on, &button_async);  
}  

/**
 * 定义注册设备所需要的file_operations结构体
 */
struct file_operations reg_fos=
{
	.owner=THIS_MODULE,
	.open=key_open,
	.release=key_close,
	.unlocked_ioctl=key_ioctl,
	.read=key_read,
	.write=key_write,
	.llseek=key_llseek,
	.poll    =  key_poll,  
    .fasync  =  key_fasync,  
};

static int request_key_gpio(void)
{
	int ret,i;
	for(i=0;i<KEY_NUM;i++)
	{
		ret= gpio_request(key_gpios[i],"KEY_BUTTONS");
		if(ret)
		{
			printk(KERN_EMERG"requet the key gpios failed  the ret is %d \r\n",ret);
			return ret;
		}
		else
		{
			printk(KERN_EMERG"request the key gpios %d success",key_gpios[i]);
		}
		//对key gpio进行配置
		//配置为外部中断模式
		s3c_gpio_cfgpin(key_gpios[i],S3C_GPIO_SFN(0xF));
		//配置为上拉
		s3c_gpio_setpull(key_gpios[i],S3C_GPIO_PULL_UP);
		gpio_free(key_gpios[i]);
	}
	return 0;
}

/**
 * 注册设备
 * @param  dev   [description]
 * @param  index [description]
 * @return       [description]
 */
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
		//如果cdev_add返回值不为0 ，则说明向系统中加入设备失败
		printk(KERN_EMERG"cdev add the device is failed \n");
	}else
	{
		printk(KERN_EMERG"cdev add the device is success the err is %d  ,the dev_minor is %d \n",err,dev_minor+index);
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
	printk(KERN_EMERG"alloc_chrdev_region the return value is %d \r\n",ret);
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
	//printk(KERN_EMERG"the kmalloc return value is %d \r\n",reg_devs);
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
	ret=request_key_gpio();
	if(!ret){
		return ret;
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
