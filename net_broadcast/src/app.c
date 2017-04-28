#include "c_utils.h"
#include "c_net.h"
#include "mp3_player.h"

#define SERVER_NAME "192.168.0.34"
#define SERVER_PORT 7002
#define BUFSIZE 512


int main(int argc, char * argv[])
{
	 /*if (argc !=2){
	 	return 0;
	 }*/
	init_kill_signal();
	
	char buf[BUFSIZE];
	int n ;
	
	init_audio();
	write_audio_vol(90);
	//audio_play(argv[1]);
	net_play();
	//等到音频驱动初始化完成后，再启动网络连接
	init_c_net();

	int con=net_start(SERVER_NAME,SERVER_PORT);
	if(con <0)
	{
		_debug("connect to server is error!!!!");
		//此处不能进行直接退出程序操作
		//exit(0);
	}

	//player_audio(argc,argv);
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
    close_audio();
	return 0;	
}