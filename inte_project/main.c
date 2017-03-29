#include "signal_pro.h"
#include "net_server.h"

#define SERVER_NAME "192.168.0.34"
#define SERVER_PORT 7002
#define BUFSIZE 512


int main(int argc, char * argv[])
{
	init_signal();
	init_net_server();
	char buf[BUFSIZE];
	int n ;
	int con=start_server(SERVER_NAME,SERVER_PORT);
	if(con <0)
	{
		_debug("connect to server is error!!!!");
		//此处不能进行直接退出程序操作
		//exit(0);
	}

	 while(fgets(buf, BUFSIZE, stdin) != NULL){
        /*通过sockfd给服务端发送数据*/
        net_send(buf, strlen(buf));
        //n = net_recv(buf, BUFSIZE);
        //if(n == 0){
          //  _debug("the other side has been closed");
        //}
        //else/*打印输出服务端传过来的数据*/{
          //  write(STDOUT_FILENO, buf, n);
        //}
    }

    close_con();
    close_thread();
	return 0;	
}
