//生成设备节点信息
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
/**
 * 杂项设备结构体描述头文件，包括设备的注册、设备的卸载
 */
#include <linux/miscdevice.h>
/**
 * 注册设备文件节点的结构体头文件
 */
#include <linux/fs.h>

/**
 * Linux中申请gpio的头文件
 */
#include <linux/gpio.h>
/*三星平台的GPIO配置函数头文件*/
/*三星平台EXYNOS系列平台，GPIO配置参数宏定义头文件*/
#include <plat/gpio-cfg.h>
#include <mach/gpio.h>
/*三星平台4412平台，GPIO宏定义头文件*/
#include <mach/gpio-exynos4.h>

//使用ioremap函数头文件
#include <asm/io.h>  
#include <asm/uaccess.h>  

//驱动注册名称，此名称一定要与注册的设备名称对应
#define DRIVER_NAME "buzzer_ko" 
//生成的设备节点名称，此名称显示在/dev目录下，应用程序使用此文件名称来实现应用
#define DEVICE_NAME "buzzer_k"

MODULE_LICENSE("Dual BSD/GPL"); //声明是开源的，没有内核版本限制
MODULE_AUTHOR("KQZ"); //声明作者

#define R_GPD0CON  0x114000A0 //GPDO_0 控制输入输出设置寄存器
#define R_GPD0PUD  0x114000A8 //GPD0_0 控制上拉或下拉寄存器
#define R_TIMER0   0x139D0000 

#define  MAGIC_NUMBER    'k'  
#define  BEEP_ON    _IO(MAGIC_NUMBER    ,0)  
#define  BEEP_OFF   _IO(MAGIC_NUMBER    ,1)  
#define  BEEP_FREQ   _IO(MAGIC_NUMBER   ,2)  

struct PWM{  
    unsigned int    TCFG0;  
    unsigned int    TCFG1;  
    unsigned int    TCON;  
    unsigned int    TCNTB0;  
    unsigned int    TCMPB0;  
    unsigned int    TCNTO0;  
    unsigned int    TCNTB1;  
    unsigned int    TCMPB1;  
    unsigned int    TCNTO1;  
    unsigned int    TCNTB2;  
    unsigned int    TCMPB2;  
    unsigned int    TCNTO2;  
    unsigned int    TCNTB3;  
    unsigned int    TCMPB3;  
    unsigned int    TCNTO3;  
    unsigned int    TCNTB4;  
    unsigned int    TCNTO4;  
    unsigned int    TINT_CSTAT;  
};

struct PWM *pwm;
static unsigned int * gpd0con;
static unsigned int * gpd0pud;
static unsigned int * gdp0data;


//static unsigned int *gpd0con;  
static void *timer_base;  
#define GPD0CON       0x114000a0  
#define TIMER_BASE    0x139D0000             
#define TCFG0         0x0000                 
#define TCFG1         0x0004                              
#define TCON          0x0008               
#define TCNTB0        0x000C            
#define TCMPB0        0x0010    

#define BEPP_IN_FREQ 10000  
static void buzzer_freq(unsigned long arg)  
{  
    //writel(BEPP_IN_FREQ/arg, &pwm->TCNTB0  );  
    //writel(BEPP_IN_FREQ/(2*arg), &pwm->TCMPB0 );  
  	writel(BEPP_IN_FREQ/arg, timer_base +TCNTB0  );  
    writel(BEPP_IN_FREQ/(2*arg), timer_base +TCMPB0 );  
}  

static void buzzer_on(void)
{
	//设置TIMER0为自动装载 并且开启定时器
	//writel((readl(&pwm->TCON) & (~(0xf<<0))) | (0x9<<0),&pwm->TCON);
	writel ((readl(timer_base +TCON )&~(0xf<<0)) | (0x9 <<0),timer_base +TCON );  
}

static void buzzer_off(void)
{
	//writel((readl(&pwm->TCON)&(~(0xf<<0))) |(0x0<<0),&pwm->TCON);
	writel ((readl(timer_base +TCON )&~(0xf<<0)) | (0x0 <<0),timer_base +TCON ); 
}

/**********start generate devicenode*********/
static int buzzer_open  (struct inode * node, struct file * f)
{
	printk(KERN_EMERG"\t generate the device node in buzzer open function----------\n");
	return 0;
}
static int buzzer_release (struct inode * node , struct file * f)
{
	printk(KERN_EMERG"\t generate the device node in buzzer relese function ------\n");
	buzzer_off();
	return 0;
}
static long buzzer_ioctl(struct file * f, unsigned int cmd , unsigned long arg)
{
	printk(KERN_EMERG"\t generate buzzer device node the cmd is %d ,arg is %d",cmd,arg);

	switch(cmd)
	{
		case BEEP_ON:
		{
			buzzer_on();
			break;
		}
		case BEEP_OFF:
		{
			buzzer_off();
			break;
		}
		case BEEP_FREQ:
		{
			//bu
			buzzer_freq(arg);
			break;
		}
		default:
			return -EINVAL; 
	}
	return 0;
}
static struct file_operations buzzer_ops={
	.owner=THIS_MODULE,
	.open=buzzer_open,
	.release=buzzer_release,
	.unlocked_ioctl=buzzer_ioctl,
};

static struct miscdevice buzzer_dev={
	.minor = MISC_DYNAMIC_MINOR, //自动生成设备节点次设备号
	.name = DEVICE_NAME,
	.fops = &buzzer_ops,
};


/**
 * buzzer蜂鸣器，有GPD0_0管脚来控制，该IO口用TIMER0来控制
 * @param  pdv [description]
 * @return     [description]
 */
static int buzzer_probe(struct platform_device * pdv)
{
	//int gpio_request_state;
	int miscdevice_state;
	printk(KERN_EMERG "\tthe buzzer driver probe init -----\n");

	gpd0con = ioremap(GPD0CON,4);  
    timer_base = ioremap(TIMER_BASE,0x14);  
      
    writel ((readl(gpd0con)&~(0xf<<0)) | (0x2<<0),gpd0con);  
    writel ((readl(timer_base +TCFG0  )&~(0xff<<0)) | (0xff <<0),timer_base +TCFG0);   
    writel ((readl(timer_base +TCFG1 )&~(0xf<<0)) | (0x2 <<0),timer_base +TCFG1 );   
  
    writel (500, timer_base +TCNTB0  );  
    writel (250, timer_base +TCMPB0 );  
    writel ((readl(timer_base +TCON )&~(0xf<<0)) | (0x2 <<0),timer_base +TCON );   

	/*pwm=ioremap(R_TIMER0,0x14);
	gpd0con=ioremap(R_GPD0CON,0x10);
//gpd0pud=ioremap(R_GPD0PUD,4);
	gpd0pud=(unsigned int *)(gpd0con + 0x00A8);
	gdp0data=(unsigned int *)(gpd0con + 0x00A4);
	//设置GPD0_0为输出，同时支持不上拉也不下拉
	writel((readl(gpd0con)&(~(0xf<<0)))|(0x2<<0),gpd0con); //设置为输出
	writel(readl(gpd0pud)&(~(0xf<<0)),gpd0pud);//不上拉也不下拉
	writel(0,gdp0data);
	//预分频，支持1-254 + 1
	writel( (readl(&pwm->TCFG0)&(~(0xff<<0))) | (0xff<<0) ,&pwm->TCFG0); //预分频
	//分频 1，2，4，8
	writel((readl(&pwm->TCFG1) &(~(0xf<<0)))|(0x2<<0),&pwm->TCFG1); //分频
	//设置重载值
	writel(500,&pwm->TCNTB0);
	//设置占空比
	writel(250,&pwm->TCMPB0);

	writel((readl(&pwm->TCON)&(~(0xf<<0)))|(0x2<<0),&pwm->TCON) ;//更新TCNTB0与TCMPB0的值*/

	//进行杂项设备的注册
	miscdevice_state=misc_register(&buzzer_dev);
	printk(KERN_EMERG"\t the register miscdevice state is %d \n",miscdevice_state);
	return 0;
}

void buzzer_unmap(void)  
{  
	iounmap(gpd0con);  
    iounmap(timer_base);  
    //iounmap(gpd0con);  
    //iounmap(pwm);  
    //iounmap(gpd0pud);
} 

static int buzzer_remove(struct platform_device * pdv)
{
	int miscdevice_state;

	buzzer_unmap();
	//

	//进行杂项设备的卸载
	miscdevice_state=misc_deregister(&buzzer_dev);
	printk(KERN_EMERG"\t the deregister miscdevice state is %d \n",miscdevice_state);
	return 0;
}

static void buzzer_shutdown(struct platform_device * pdv)
{
	//return 0;
}


static int buzzer_suspend(struct platform_device * pdv,pm_message_t state)
{
	return 0;
}


static int buzzer_resume(struct platform_device * pdv)
{
	return 0;
}

/**
 * 注册驱动结构体
 */
struct platform_driver buzzer_driver={
	.probe=buzzer_probe,
	.remove=buzzer_remove,
	.shutdown=buzzer_shutdown,
	.suspend=buzzer_suspend,
	.resume=buzzer_resume,
	.driver={
		.name=DRIVER_NAME,
		.owner=THIS_MODULE,
	}
};

/**
 * [驱动加载程序入口函数]
 * @return  [description]
 */
static int buzzer_init(void)
{
	int driver_state;

    /*
     打印信息，KERN_EMERG 表示紧急信息
     */
    printk(KERN_EMERG "buzzer init enter!\n");
    //注册设备驱动
    driver_state=platform_driver_register(&buzzer_driver);
    printk(KERN_EMERG "the driver register state is %d \n",driver_state);
    return 0;
}

/**
 * [驱动模块卸载程序执行的函数入口]
 */
static void buzzer_exit(void)
{
	//卸载设备驱动
	platform_driver_unregister(&buzzer_driver);	
    printk(KERN_EMERG "buzzer ko  exit !\n");
}

module_init(buzzer_init); //模块入口
module_exit(buzzer_exit); //模块出口