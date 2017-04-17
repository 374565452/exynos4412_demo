#include "alsa_pcm.h"
#include "mp3_audio.h"
#include "mp3_player.h"

#include <pthread.h>

int main(int argc, char * argv[])
{

 //if (argc != 2)
  	if(argc < 2)
    {
    	printf("Usage: minimad + mp3 file name");
    	return 1;
    }

    int fd; 
  	fd=open(argv[1],O_RDWR); //打开指定文件
  	if(fd<0)
  	{
    	perror("open file failed:");
    	return 1;
  	}    

	init_mp3_audio();//初始化循环缓冲区
	set_pcm();//初始化pcm数据
	init_mixer(); //初始化声音
	
	int vol = 34;
    if((argc == 3))
    {
    	vol =atoi(argv[2]);
    	printf("the vol is %d \r\n",vol);
    }
    write_vol(vol);

    decode(&mp3_buf);

    close_pcm(); //关闭pcm设备

	return 0;
}
/**
 * 缺少向循环缓冲区添加数据，同时从循环缓冲区中读取数据
 */