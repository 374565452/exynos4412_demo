#include <stdio.h>     	 //��׼�������
#include <string.h>     	//�ַ�����
#include <sys/types.h>     //open��creat������Ҫ��ͷ�ļ�
#include <sys/stat.h>      //open��creat������Ҫ��ͷ�ļ�
#include <fcntl.h>		   //open��creat������Ҫ��ͷ�ļ�
#include <unistd.h>       //close,read,write������Ҫ��ͷ�ļ�
#include <termios.h>      //���ڽṹ��Ͷ�Ӧ����������Ҫ��ͷ�ļ�
#include <errno.h>        //������Ϻ������Ҫ��ͷ�ļ�
#include <malloc.h>  	  //�ڴ����

#define PATH  "/dev/i2c_7_dev-0x50"
  
int main(int argc,char *argv[])
{
	int fd_i2c_0=-1,ret = -1;
	int num = strlen(argv[2]);
	char *test = NULL;	 
	
	fd_i2c_0 = open(PATH,O_RDWR);
	usleep(1000);
	
	if(argv[1])
		if( lseek(fd_i2c_0,atoi(argv[1]), SEEK_CUR)<0 )  //����֮ǰ����lseek��λ��Ҫ��Ȼÿ�β��������ݵ�ַ����0
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
