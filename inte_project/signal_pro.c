#include "signal_pro.h"
#include "net_server.h"

void kill_signal_fun(int sign)
{
    _debug("the kill signal fun sign is %d \n", sign);  
   	close_con();
    exit(0);
}

void init_signal(void)
{
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
    sigaction(SIGTERM|SIGINT, &act, 0);
}