#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <stdio.h>  
#include <sys/ioctl.h>  
  
#define  MAGIC_NUMBER    'k'  
#define   BEEP_ON    _IO(MAGIC_NUMBER    ,0)  
#define   BEEP_OFF   _IO(MAGIC_NUMBER    ,1)  
#define   BEEP_FREQ   _IO(MAGIC_NUMBER    ,2)  
  
main()  
{  
    int fd;  
  
    fd = open("/dev/buzzer_k",O_RDWR);  
    if(fd<0)  
    {  
        perror("open fail \n");  
        return ;  
    }  
  
    ioctl(fd,BEEP_ON);  
  
    sleep(6);  
    ioctl(fd,BEEP_OFF);   
  
    close(fd);  
}  