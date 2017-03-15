#include <stdio.h>     	 //标准输入输出
#include <string.h>     	//字符操作
#include <sys/types.h>     //open和creat函数需要的头文件
#include <sys/stat.h>      //open和creat函数需要的头文件
#include <fcntl.h>		   //open和creat函数需要的头文件
#include <unistd.h>       //close,read,write函数需要的头文件
#include <termios.h>      //串口结构体和对应操作函数需要的头文件
#include <errno.h>        //错误诊断和输出需要的头文件
#include <malloc.h>  	  //内存分配

#define PATH  "/dev/i2c_7_dev-0x50"
  
int main(int argc,char *argv[])
{
	int fd_i2c_0=-1,ret = -1;
	int num = strlen(argv[2]);
	char *test = NULL;	 
	
	fd_i2c_0 = open(PATH,O_RDWR);
	usleep(1000);
	
	if(argv[1])
		if( lseek(fd_i2c_0,atoi(argv[1]), SEEK_CUR)<0 )  //操作之前先用lseek定位，要不然每次操作的数据地址都是0
			printf("lseek failed!\n");	
	
	ret = write(fd_i2c_0,argv[2],num);
	if(ret<0)
		printf("i2c_write wrong!\n");
	else printf("write data:%s\n",argv[2]);
	
	test = (char *)malloc(num);
	if(test == NULL)
		goto malloc_fail;
	memset(test,'\0',num);
	//printf("test = %s\n",test);
	
	ret = read(fd_i2c_0,test,num);
	if(ret<0)
		printf("i2c_read wrong!\n");
	else 
		printf("read data: %s\n",test);

	free(test);
	test = NULL;
	close(fd_i2c_0);	
	return 0;
	malloc_fail:
		printf("malloc failed!\n");	
}
