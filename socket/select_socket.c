#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/time.h>  
#include <netinet/in.h>  
#include <netdb.h>  
#include <stdio.h>  
#include <string.h>  
#include <unistd.h>  
#include <stdlib.h>  
#include <fcntl.h>  
  
#include <signal.h> 

#include <errno.h> 
// this routine simply converts the address into an  
// internet ip  
unsigned long name_resolve(char *host_name)  
{  
struct in_addr addr;  
struct hostent *host_ent;  
  if((addr.s_addr=inet_addr(host_name))==(unsigned)-1) {  
    host_ent=gethostbyname(host_name);  
    if(host_ent==NULL) return(-1);  
    memcpy(host_ent->h_addr, (char *)&addr.s_addr, host_ent->h_length);  
    }  
  return (addr.s_addr);  
}  
int s;
struct sockaddr_in sinaddr;  
int asyncConnect(){
  int times = 0;  
  int ret=-1;
  while (times++ < 5)  
  {  
    fd_set rfds, wfds;  
    struct timeval tv;       
    printf("errno = %d\n", errno);  
    FD_ZERO(&rfds);  
    FD_ZERO(&wfds);  
    FD_SET(s, &rfds);  
    FD_SET(s, &wfds);    
                /* set select() time out */  
    tv.tv_sec = 10;   
    tv.tv_usec = 0;  
    int selres = select(s + 1, &rfds, &wfds, NULL, &tv);  
    switch (selres)  
    {  
     case -1:  
        printf("select error\n");  
        ret = -1;  
       break;  
      case 0:  
        printf("select time out\n");  
        ret = -1;  
        break;  
      default:  
        if (FD_ISSET(s, &rfds) || FD_ISSET(s, &wfds))  
        {  
                  
      
          connect(s, (struct sockaddr *)&sinaddr, sizeof(struct sockaddr_in));  
          int err = errno;  
          if  (err == EISCONN)  
           {  
              printf("connect finished 111.\n");  
              ret = 0;  
            }  else  {  
              printf("connect failed. errno = %d\n", errno);  
              printf("FD_ISSET(s, &rfds): %d\n FD_ISSET(s, &wfds): %d\n", FD_ISSET(s, &rfds) , FD_ISSET(s, &wfds));  
              ret = errno;  
            }  
          } else {  
               printf("haha\n");  
            }  
        }  
                  
      if (-1 != selres && (ret != 0))  
      {  
        printf("check connect result again... %d\n", times);  
        continue;  
      } else {  
        break;  
      }  
  } 
  return ret;

}

// The connect routine including the command to set  
// the socket non-blocking.  
int doconnect(char *address, int port)  
{  
  int x;  
 
  
  s=socket(AF_INET, SOCK_STREAM, 0);  
  x=fcntl(s,F_GETFL,0);              // Get socket flags  
  fcntl(s,F_SETFL,x | O_NONBLOCK);   // Add non-blocking flag  
  //bzero(&sin,sizeof(sin));
  memset(&sinaddr, 0, sizeof(struct sockaddr_in));  
  sinaddr.sin_family=AF_INET;  
  inet_pton(AF_INET,address,&sinaddr.sin_addr);
  sinaddr.sin_port=htons(port);  
  //sin.sin_addr.s_addr=name_resolve(address);  
 // if(sin.sin_addr.s_addr==NULL) return(-1);  
  printf("ip: %s ;the port is %d \n",inet_ntoa(sinaddr.sin_addr),port);  
  x=connect(s, (struct sockaddr *)&sinaddr, sizeof(sinaddr));  
  if(errno==EINPROGRESS)
  {
    //因为这时，已经将socket的编程模型更改为不阻塞模式，当进行connect的时候，会立即返回，并不会等待完成
    //所以如果这里判断返回的结果，肯定是错误
    //在这种情况下，连接的状态有可能为已经建立好连接，也有可能为正在处理连接
    //当服务端程序与客户端程序在同一机器上时，为建立好连接，不在同一机器时为正在处理连接
    printf("---------------------------\r\n");
    x=asyncConnect();
  }
  printf("the connect state is %d \r\n",x);
  if(x<0) 
  {
    return(-1); 
  } 
  return(s);  
}  
void kill_signal_fun(int sign)
{
    printf("the connect state is %d \r\n",s);
    //应先关闭此socket，然后再将进程退出
    if(s){
      close(s);
    }
    exit(0);
}
/**
 * linux下的select 编程模型，会一直在扫描select函数，CPU占用过高，并不适合于客户端使用
 * 代码中的while循环，一直处于执行状态，并没有阻塞
 * @return  [description]
 */
int main (void)  
{  
  fd_set read_flags,write_flags; // you know what these are  
  struct timeval waitd;            
  int thefd;             // The socket  
  char outbuff[512];     // Buffer to hold outgoing data  
  char inbuff[512];      // Buffer to read incoming data into  
  int err;           // holds return values  
  
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

  memset(&outbuff,0,sizeof(outbuff)); // memset used for portability  
  thefd=doconnect("192.168.0.34",7002); // Connect to the finger port  
  if(thefd==-1) {  
    printf("Could not connect to finger server/n");  
    exit(0);  
    }  
  strcat(outbuff,"jarjam/n"); //Add the string jarjam to the output  
                              //buffer  
  while(1) {  
    waitd.tv_sec = 1;     // Make select wait up to 1 second for data  
    waitd.tv_usec = 0;    // and 0 milliseconds.  
    FD_ZERO(&read_flags); // Zero the flags ready for using  
    FD_ZERO(&write_flags);  
    FD_SET(thefd, &read_flags);  
    if(strlen(outbuff)!=0) FD_SET(thefd, &write_flags);  
    err=select(thefd+1, &read_flags,&write_flags, (fd_set*)0,&waitd);  
    printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\r\n");
    if(err < 0) {
      continue; 
    } 
    if(FD_ISSET(thefd, &read_flags)) { //Socket ready for reading  
      FD_CLR(thefd, &read_flags);  
      memset(&inbuff,0,sizeof(inbuff));  
      if (read(thefd, inbuff, sizeof(inbuff)-1) <= 0) {  
        close(thefd);  
        break;  
        }  
      else printf("%s",inbuff);  
      }  
    if(FD_ISSET(thefd, &write_flags)) { //Socket ready for writing  
      FD_CLR(thefd, &write_flags);  
      write(thefd,outbuff,strlen(outbuff));  
      memset(&outbuff,0,sizeof(outbuff));  
      }  
    // now the loop repeats over again  
    }  
}  