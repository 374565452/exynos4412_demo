#include "alsa_pcm.h"
#include "mp3_audio.h"
#include "mp3_player.h"

#include <pthread.h>

#include <semaphore.h> //信号量机制头文件

FILE * fp;

pthread_t read_file_thread;
pthread_t player_thread;
static void init_thread();
static void * decoder_run(void * arg);
static void * read_file_run(void * arg);
static void close_thread(void);
unsigned char read_thread_flag=0;
unsigned char play_thread_flag=0;
sem_t empty_sem; //数据空信号量
sem_t full_sem;  //数据满信号量

int main(int argc, char * argv[])
{

 //if (argc != 2)
  	if(argc < 2)
    {
    	printf("Usage: minimad + mp3 file name");
    	return 1;
    }

    
  	fp=fopen(argv[1],"r"); //打开指定文件
  	if(fp==NULL)
  	{
    	perror("open file failed:");
    	return 1;
  	}    
	init_mp3_audio();//初始化循环缓冲区
	int rec=set_pcm();//初始化pcm数据
	//printf("--------------the rec-----is %d \r\n  ",rec);
	init_mixer(); //初始化声音

	init_thread();
	
	int vol = 34;
    if((argc == 3))
    {
    	vol =atoi(argv[2]);
    	//printf("the vol is %d \r\n",vol);
    }
    write_vol(vol);

    decode(&mp3_buf);

    close_pcm(); //关闭pcm设备
    //printf("close the app -------------------------------\r\n");
    close_thread();
	return 0;
}
/**
 * 缺少向循环缓冲区添加数据，同时从循环缓冲区中读取数据
 */
static void init_thread()
{
	int f_state,p_state;
	f_state=pthread_create(&read_file_thread,NULL,read_file_run,NULL);
	if(f_state != 0 )
	{
		printf("create the read_file_thread is failed ----------\r\n");
	}else
	{
		printf("create the read_file_thread is success ----------\r\n");
		read_thread_flag=1;
	}
	p_state=pthread_create(&player_thread,NULL,decoder_run,NULL);
	if(p_state != 0 )
	{
		printf("create the player_thread is failed ----------\r\n");
	}else
	{
		printf("create the player_thread is success ----------\r\n");
		play_thread_flag=1;
	}
	sem_init(&empty_sem, 0, 1);
	sem_init(&full_sem, 0, 0);
}

static void * decoder_run(void * arg)
{
	while(play_thread_flag)
	{
		//if( !get_data_size(&mp3_buf))
		{
			//printf("no data in the cycle buffer ,wait the full sem ------\r\n");
			//缓冲区中没有数据，这里需要等待
			sem_wait(&full_sem);  
		}
		//printf("----------------------decode the mp3 data ----------------------\r\n");
		player_run();//执行解码
		int data_size=get_data_size(&mp3_buf);
		//printf("the mp3 buf sdata size is %d ----\r\n",data_size);
		//if(get_data_size(&mp3_buf) == 0)
		{
			//printf("----post the empty_sem-----\r\n");
			sem_post(&empty_sem);
		}
	}
	return arg;
}
static void * read_file_run(void * arg)
{
	unsigned char read_buf[DECODER_LEN];
	while(read_thread_flag)
	{
		//if(get_extra_capacity(&mp3_buf)<DECODER_LEN) //缓冲区已经满拉
		{
			//printf("call empty_sem-----\r\n");
			sem_wait(&empty_sem);
		}
		if(fp)
		{
			int len=fread(read_buf,1,DECODER_LEN,fp);

			//printf("the read mp3 file len is %d -----------\r\n ",len);
			
			if(len > 0){
				//printf("write the mp3 data to the cycle buffer ----------------\r\n");
				write_audio_data(read_buf,len);
			}else{
				read_thread_flag=0;
			}
			int data_size=get_data_size(&mp3_buf);
			int extra_capacity=get_extra_capacity(&mp3_buf);
			//printf("the mp3 buf extra capacity is %d the data size is %d ----\r\n", extra_capacity,data_size);
			sleep(1);
			//if(get_extra_capacity(&mp3_buf)<DECODER_LEN)
			{
				//printf("post the full _sem-----\r\n");
				sem_post(&full_sem);
			}
		}

	}
	printf("the mp3 file is read complete --------\r\n");
	return arg;
}


static void close_thread(void)
{
	//read_thread_flag=0;
	//play_thread_flag=0;
	if(read_file_thread != 0 )
	{
		pthread_join(read_file_thread,NULL);
	}
	if(player_thread != 0)
	{
		pthread_join(player_thread,NULL);
	}
}