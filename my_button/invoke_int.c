#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <stdio.h>  
#include <poll.h>  
#include <signal.h>  
#include <sys/types.h>  
#include <unistd.h>  
#include <fcntl.h>  
  
  
/* fifthdrvtest  
  */  
int fd;  
  char flag =1;
//信号处理函数  
void my_signal_fun(int signum)  
{  
    char key_val;  
    read(fd, &key_val, 1);  
    printf("key_val: 0x%x\n", key_val);  
}  

void kill_signal_fun(int sign)
{
    printf("the kill signal fun sign is %d \n", sign);  
    //停止此进程
    ////应该将关闭资源代码程序运行放在这里
    if(fd>0)
    {
        close(fd);
    }
    flag=0;
}

int main(int argc, char **argv)  
{  
    unsigned char key_val;  
    int ret;  
    int Oflags;  
  //当程序用kill命令来杀掉后，会发送SIGTERM ,下面的处理就是要捕获此信号，然后进
  //按下CTRL+C会发送SIGTTIN
  //行停止此程序，进行资源的释放工作
    struct sigaction act;  
    act.sa_handler = kill_signal_fun;  
    sigemptyset(&act.sa_mask);  
    sigaddset(&act.sa_mask, SIGQUIT);  
      // act.sa_flags = SA_RESETHAND;  
      // act.sa_flags = SA_NODEFER;  
    act.sa_flags = 0;  
    sigaction(SIGTERM, &act, 0);
//结束 
    printf("testttttttttttttttttttttttttttttttttttt\n");
    //在应用程序中捕捉SIGIO信号（由驱动程序发送）  
    signal(SIGIO, my_signal_fun);  
      
    fd = open("/dev/k_buttons0", O_RDWR);  
    if (fd < 0)  
    {  
        printf("can't open!\n");  
    }  
  
    //将当前进程PID设置为fd文件所对应驱动程序将要发送SIGIO,SIGUSR信号进程PID  
    fcntl(fd, F_SETOWN, getpid());  
      
    //获取fd的打开方式  
    Oflags = fcntl(fd, F_GETFL);   
    printf("the oflags is %d -----\n", Oflags);
    //将fd的打开方式设置为FASYNC --- 即 支持异步通知  
    //该行代码执行会触发 驱动程序中 file_operations->fasync 函数 ------fasync函数调用fasync_helper初始化一个fasync_struct结构体，该结构体描述了将要发送信号的进程PID (fasync_struct->fa_file->f_owner->pid)  
    fcntl(fd, F_SETFL, Oflags | FASYNC);  
  
  
    while (flag)  
    {  
        sleep(1000);  
        //这里代码有问题，执行1S后，就会进行关闭操作，当按键按下后，就不会有键值传送回来
        //close(fd);
    }  
    
    return 0;  
}  