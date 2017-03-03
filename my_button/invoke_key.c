#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>


int main(int argc ,char ** args)
{
	int fd;
	char * hello_node0="/dev/k_buttons0";
	if((fd = open(hello_node0,O_RDWR|O_NDELAY))<0){
		printf("APP open %s failed! the fd is %d \n",hello_node0,fd);
	}
	else{
		printf("APP open %s success! the fd is %d \n",hello_node0,fd);
	}
	
	close(fd);
}