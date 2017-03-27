#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>

#include <fcntl.h> 
#include <signal.h>  
#include <sys/types.h>  

#define BUFSIZE 512
#define SERV_PORT 7002
int sockfd;
void kill_signal_fun(int sign)
{
    printf("the kill signal fun sign is %d \n", sign);  
    //应先关闭此socket，然后再将进程退出
    if(sockfd){
    	close(sockfd);
    }
    exit(0);
}

int main(int argc,char * argv[])
{
	struct sockaddr_in servaddr;
	char buf[BUFSIZE];
	int n;
	int con=0;

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
//结束 
    //设置连接的超时时间
	struct timeval timeout={13,0};
	


	sockfd=socket(AF_INET,SOCK_STREAM,0);

setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));


	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	inet_pton(AF_INET,"192.168.0.34",&servaddr.sin_addr);
	servaddr.sin_port = htons(SERV_PORT);
/*连接服务端*/
    con=connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    printf("the con is %d \r\n",con);
    if(con<0)
    {
    	printf("\r\n");
    	exit(0);
    }
    while(fgets(buf, BUFSIZE, stdin) != NULL){
        /*通过sockfd给服务端发送数据*/
        write(sockfd, buf, strlen(buf));
        n = read(sockfd, buf, BUFSIZE);
        if(n == 0)
            printf("the other side has been closed.\n");
        else/*打印输出服务端传过来的数据*/
            write(STDOUT_FILENO, buf, n);
    }

    close(sockfd);

	return 0;
}