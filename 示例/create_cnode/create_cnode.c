/*������ʼ���궨���ͷ�ļ�,�����е�module_init��module_exit�ڴ��ļ���*/
#include <linux/init.h>
/*������ʼ������ģ���ͷ�ļ�,�����е�MODULE_LICENSE�ڴ�ͷ�ļ���*/
#include <linux/module.h>
/*����module_param module_param_array��ͷ�ļ�*/
#include <linux/moduleparam.h>
/*����module_param module_param_array��perm��ͷ�ļ�*/
#include <linux/stat.h>
/*�����ַ��豸����*/
#include <linux/fs.h>
/*MKDEVת���豸���������͵ĺ궨��*/
#include <linux/kdev_t.h>
/*�����ַ��豸�Ľṹ��*/
#include <linux/cdev.h>
/*�����ڴ�ռ亯��ͷ�ļ�*/
#include <linux/slab.h>

/*��������device_create �ṹ��class��ͷ�ļ�*/
#include <linux/device.h>

#define DEVICE_NAME "chardevnode"
#define DEVICE_MINOR_NUM 2
#define DEV_MAJOR 0
#define DEV_MINOR 0
#define REGDEV_SIZE 3000

MODULE_LICENSE("Dual BSD/GPL");
/*�����ǿ�Դ�ģ�û���ں˰汾����*/
MODULE_AUTHOR("iTOPEET_dz");
/*��������*/

int numdev_major = DEV_MAJOR;
int numdev_minor = DEV_MINOR;

/*�������豸��*/
module_param(numdev_major,int,S_IRUSR);
/*������豸��*/
module_param(numdev_minor,int,S_IRUSR);

static struct class *myclass;

struct reg_dev
{
	char *data;
	unsigned long size;
	
	struct cdev cdev;
};
struct reg_dev *my_devices;

struct file_operations my_fops = {
	.owner = THIS_MODULE,
};


/*�豸ע�ᵽϵͳ*/
static void reg_init_cdev(struct reg_dev *dev,int index){
	int err;
	int devno = MKDEV(numdev_major,numdev_minor+index);
	
	/*���ݳ�ʼ��*/
	cdev_init(&dev->cdev,&my_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &my_fops;
	
	/*ע�ᵽϵͳ*/
	err = cdev_add(&dev->cdev,devno,1);
	if(err){
		printk(KERN_EMERG "cdev_add %d is fail! %d\n",index,err);
	}
	else{
		printk(KERN_EMERG "cdev_add %d is success!\n",numdev_minor+index);
	}
}

static int scdev_init(void)
{
	int ret = 0,i;
	dev_t num_dev;
	
	
	printk(KERN_EMERG "numdev_major is %d!\n",numdev_major);
	printk(KERN_EMERG "numdev_minor is %d!\n",numdev_minor);
	
	if(numdev_major){
		num_dev = MKDEV(numdev_major,numdev_minor);
		ret = register_chrdev_region(num_dev,DEVICE_MINOR_NUM,DEVICE_NAME);
	}
	else{
		/*��̬ע���豸��*/
		ret = alloc_chrdev_region(&num_dev,numdev_minor,DEVICE_MINOR_NUM,DEVICE_NAME);
		/*������豸��*/
		numdev_major = MAJOR(num_dev);
		printk(KERN_EMERG "adev_region req %d !\n",numdev_major);
	}
	if(ret<0){
		printk(KERN_EMERG "register_chrdev_region req %d is failed!\n",numdev_major);		
	}
	myclass = class_create(THIS_MODULE,DEVICE_NAME);
	
	
	my_devices = kmalloc(DEVICE_MINOR_NUM * sizeof(struct reg_dev),GFP_KERNEL);
	if(!my_devices){
		ret = -ENOMEM;
		goto fail;
	}
	memset(my_devices,0,DEVICE_MINOR_NUM * sizeof(struct reg_dev));
	
	/*�豸��ʼ��*/
	for(i=0;i<DEVICE_MINOR_NUM;i++){
		my_devices[i].data = kmalloc(REGDEV_SIZE,GFP_KERNEL);
		memset(my_devices[i].data,0,REGDEV_SIZE);
		/*�豸ע�ᵽϵͳ*/
		reg_init_cdev(&my_devices[i],i);
		
		/*�����豸�ڵ�*/
		device_create(myclass,NULL,MKDEV(numdev_major,numdev_minor+i),NULL,DEVICE_NAME"%d",i);
	}
	
		
	printk(KERN_EMERG "scdev_init!\n");
	/*��ӡ��Ϣ��KERN_EMERG��ʾ������Ϣ*/
	return 0;

fail:
	/*ע���豸��*/
	unregister_chrdev_region(MKDEV(numdev_major,numdev_minor),DEVICE_MINOR_NUM);
	printk(KERN_EMERG "kmalloc is fail!\n");
	
	return ret;
}

static void scdev_exit(void)
{
	int i;
	printk(KERN_EMERG "scdev_exit!\n");
	
	/*��ȥ�ַ��豸*/
	for(i=0;i<DEVICE_MINOR_NUM;i++){
		cdev_del(&(my_devices[i].cdev));
		/*�ݻ��豸�ڵ㺯��d*/
		device_destroy(myclass,MKDEV(numdev_major,numdev_minor+i));
	}
	/*�ͷ��豸class*/
	class_destroy(myclass);
	/*�ͷ��ڴ�*/
	kfree(my_devices);
	
	unregister_chrdev_region(MKDEV(numdev_major,numdev_minor),DEVICE_MINOR_NUM);
}


module_init(scdev_init);
/*��ʼ������*/
module_exit(scdev_exit);
/*ж�غ���*/