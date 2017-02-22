/**
 * 简单上层应用，调用hello_ctl123驱动程序
 */
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main ()
{
    int fd;
    char *hello_node = "/dev/hello_led";
    if((fd = open(hello_node, O_RDWR | O_NDELAY)) < 0)
    {
        printf("APP open %s failed", hello_node);
    }
    else
    {
        printf("APP open %s success", hello_node);
        while(1)
        {
            ioctl(fd, 1, 1);
            sleep(3);
            ioctl(fd,0,1);
            sleep(3);
        }
        
        
    }

    close(fd);
    return 0;
}