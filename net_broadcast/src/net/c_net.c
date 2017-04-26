#include "c_net.h"
#include "net_protocol.h"

static void * recv_thread_func(void * pv);

static void * reconnect_thread_func(void * pv);

static int connect_server();

static void close_thread(void);

static void inner_close(void);

int sockfd;

unsigned char connect_flag=0;
unsigned char recv_thread_start=1;
unsigned char reconnect_thread_start=1;

char * ip ;
int port;

#define RECV_BUF_SIZE 1024*16
unsigned char recv_buf[RECV_BUF_SIZE];
//定义一个互斥锁
pthread_mutex_t con_mut;
pthread_t recv_thread;
pthread_t reconnect_thread;

int init_c_net(void)
{
	int recv_state;
	
	recv_thread=0;
	reconnect_thread=0;

	/*用默认属性初始化互斥锁*/
    pthread_mutex_init(&con_mut,NULL);
    //创建数据接收线程
    recv_state=pthread_create(&recv_thread,NULL,recv_thread_func,NULL);
    if(recv_state != 0)
    {
    	//_debug("the recv thread create error ! the recv_state is %d ",recv_state);
    	return NET_CREATE_RECV_THREAD_ERROR;
    }else
    {
		//_debug("the recv thread create success ! the recv_state is %d ",recv_state);
		recv_thread_start=1;
		return 0;
    }    
}

int net_start(char * server_ip,int server_port)
{
	int ret;
	int reconect_state;
	ip=server_ip;
	port=server_port;
	ret=connect_server();
	if(ret < 0 )
	{
		//_debug("connect to the server is error ------");
		return NET_CON_ERROR;
	}
	//放在这里进行重连操作，只有在程序运行，第一次结束掉连接服务器后才能进行重连操作
	reconect_state=pthread_create(&reconnect_thread,NULL,reconnect_thread_func,NULL);
    if(reconect_state != 0)
    {
    	//_debug("the reconect thread create error ! the reconect_state is %d ",reconect_state);
    	return NET_CREATE_RECON_THREAD_ERROR ;
    }else
    {
		//_debug("the reconect thread create success ! the reconect_state is %d ",reconect_state);
		 reconnect_thread_start=1;
    }
	return 0;
}
static int connect_server()
{
	struct sockaddr_in servaddr;
	int con;
	
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	_debug("the socket stream flag is [%d] ",sockfd);
	servaddr.sin_family=AF_INET;
	inet_pton(AF_INET,ip,&servaddr.sin_addr);
	servaddr.sin_port = htons(port);
/*连接服务端*/
    con=connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(con < 0)
    {
    	pthread_mutex_lock(&con_mut);
    	connect_flag=0;
    	pthread_mutex_unlock(&con_mut);
    	//_debug("connect the server is error ,the con is %d  " ,con);
    	close(sockfd);
    	return (-1);
    }
    else
    {
    	pthread_mutex_lock(&con_mut);
    	connect_flag=1;
    	pthread_mutex_unlock(&con_mut);
    	//_debug("connect the server is success,the con is %d ",con);
    	return 0;
    }
}
int net_send(void * buf,int length)
{
	int error=-1;
	if(get_connected_state())
	{
		/**
		 * ssize_t write(int fd,const void *buf,size_t nbytes) 
			write函数将buf中的nbytes字节内容写入文件描述符fd.成功时返回写的字节数.失败时返回-1.
			 并设置errno变量. 在网络程序中,当我们向套接字文件描述符写时有俩种可能.  
			1)write的返回值大于0,表示写了部分或者是全部的数据.  
			2)返回的值小于0,此时出现了错误.我们要根据错误类型来处理.  如果错误为EINTR表示在写的时候出现了中断错误.  
			如果为EPIPE表示网络连接出现了问题(对方已经关闭了连接).
		 */
		error=write(sockfd,buf,length);
	}else
	{
		return NET_SEND_ERROR; //发送数据失败
	}
	return error;
}

int net_recv(void * buf,int length)
{
	int error=-1;
	if(get_connected_state())
	{
		/**
		 * 读函数read  
ssize_t read(int fd,void *buf,size_t nbyte) 
read函数是负责从fd中读取内容.成功时,read返回实际所读的字节数,如果返回的值是0,表示已经读到文件的结束了.
小于0表示出现了错误.如果错误为EINTR说明读是由中断引起的, 如果是ECONNREST表示网络连接出了问题.
		 */
		error=read(sockfd,buf,length);
	}
	return error;
}
void set_connected_state(unsigned char s)
{
	pthread_mutex_lock(&con_mut);
	connect_flag=s;
	pthread_mutex_unlock(&con_mut);
}

unsigned char get_connected_state(void)
{
	return connect_flag;
}

void close_con()
{
	if(get_connected_state() || sockfd >0 )
	{
		close(sockfd);
	}
	close_thread();
}

void inner_close(void)
{
	if(get_connected_state() || sockfd >0 )
	{
		close(sockfd);
	}
}
static void * recv_thread_func(void * pv)
{
	int recv_len=-1;
	while(recv_thread_start)
	{
		if(get_connected_state())
		{
			_debug("---------------net recv---------------------");
			memset(recv_buf,0,RECV_BUF_SIZE);
			recv_len = read(sockfd,recv_buf,RECV_BUF_SIZE);
			//如里为0，表明已经读到文件的末尾，但在网络连接状态下为0表示，在这种状态下没有进行阻塞操作，直接返回
			//表明网络连接状态出现问题
			if(recv_len <= 0) //这里说明出错，需要重新进行连接操作
			{
				inner_close();
				_debug("the connection state is close -------------------");
				//改变连接状态
				set_connected_state(0);
			}
			else
			{
				//_debug("recv : the len is %d , the data is %s ",recv_len,recv_buf);
				if( (recv_buf[0]== NET_PROTOCOL_HEADER) && (recv_len > NET_PROTOCOL_MIN_LEN )) //如果为消息头,长度够长
				{
					int data_len=(int)((int)(recv_buf[3]<<8)+(int)recv_buf[4]);
					if((recv_len==(data_len+NET_PROTOCOL_MIN_LEN))&& (recv_buf[recv_len-1] == NET_PROTOCOL_TAIL))
					{
						process_protocol(recv_buf,recv_len); //进入到数据处理中
					}
				}
			}
		}
	}
}

static void * reconnect_thread_func(void * pv)
{
	while(reconnect_thread_start)
	{
		if(!get_connected_state())
		{
			_debug("------------reconnec to the server----------");
			connect_server();
		}
		sleep(30);
	}
}

void close_thread(void)
{
	recv_thread_start=0;
	reconnect_thread_start=0;
	if(recv_thread != 0 )
	{
		pthread_join(recv_thread,NULL);
	}
	if(reconnect_thread != 0)
	{
		pthread_join(reconnect_thread,NULL);
	}
}
