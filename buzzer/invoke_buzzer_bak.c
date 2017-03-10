
#include <fcntl.h>  
#include <stdio.h>  
#include <poll.h>  
#include <signal.h>  
#include <sys/types.h>  
#include <unistd.h>  
#include <fcntl.h>  
#include <sys/ioctl.h>
#include <sys/stat.h>
int main(int argc, char **argv)  
{  
	int fd;
	fd = open("/dev/buzzer_ctl", O_RDWR);  
    if (fd < 0)  
    {  
        printf("can't open!\n");  
    }  
    else
    {
    	ioctl(fd, 1, 1);
    	sleep(3);
    	ioctl(fd, 0, 1);
    }
	return 0;
}