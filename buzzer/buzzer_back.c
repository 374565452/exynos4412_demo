#include <linux/init.h>
#include <linux/module.h>

#include <linux/kernel.h>
#include <linux/fs.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
//#include <mach/gpio-bank.h>
#include <mach/regs-gpio.h>
#include <asm/io.h>
#include <linux/regulator/consumer.h>
//#include "gps.h"
#include <linux/delay.h>

struct {  
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
}*PWM;
volatile unsigned long virt_addr,virt_addr_gpio,phys_addr,phys_addr_gpio;//用于存放虚拟地址和物理地址
volatile unsigned long *GPD0CON,*GPD0PUD;

void addr_init(void)
{	
	phys_addr = 0x139D0000;
	virt_addr = (unsigned long)ioremap(phys_addr,0x32);
	PWM = (unsigned long*)(virt_addr+0x00);
	
	phys_addr_gpio = 0x11400000+0xA0;
	virt_addr_gpio = (unsigned long)ioremap(phys_addr_gpio,0x10);
	GPD0CON = (unsigned long*)(virt_addr_gpio+0x00);
	GPD0PUD = (unsigned long*)(virt_addr_gpio+-0x00A0);
}

void pwm_init(void)
{
	addr_init();
	*GPD0CON =  *GPD0CON&(~(0xf))|0x2;
	*GPD0PUD =  *GPD0PUD&(~(0xf));
	
	//预分频1-254 + 1
	(*PWM).TCFG0 = (*PWM).TCFG0 &(~(0xff))|0xf9;
	//分频1.2.4.8.16
	(*PWM).TCFG1 = (*PWM).TCFG1 &(~(0xf))|0x2;
	//设置占空比
	(*PWM).TCMPB0 = 50;
	(*PWM).TCNTB0 = 100;
	//设置手动加载,开启定时器
	(*PWM).TCON = (*PWM).TCON & (~(0xf)) | 0x1 | 0x2;	
}
static void beep_on(void)
{
	//开启自动重载
	(*PWM).TCON = (*PWM).TCON & (~(0xf)) | 0x1 | 0x8;
}

static int iTop4412_PWM_init(void)
{
	pwm_init();
	beep_on();
	return 0;
}
static void beep_off(void)
{
	(*PWM).TCON = (*PWM).TCON & (~(0xf)) | 0x0;
	//定时器结束之后，输出的是高电平还是低电平?
	*GPD0CON =  *GPD0CON&(~(0xf))|0x0;
}
static void iTop4412_PWM_exit(void)
{
	beep_off();
}

module_init(iTop4412_PWM_init);
module_exit(iTop4412_PWM_exit);
MODULE_LICENSE("GPL");












