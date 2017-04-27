#include "mp3_pcm.h"
snd_pcm_t * handle=NULL;        //PCI设备句柄
snd_pcm_hw_params_t* params=NULL;//硬件信息和PCM流配置


snd_mixer_t * mixer;
snd_mixer_elem_t *pcm_element;

int set_pcm()
{

	int    rc;     
    int  dir=0;
    int rate = 44100;;                /* 采样频率 44.1KHz*/
    int format = SND_PCM_FORMAT_S16_LE; /*     量化位数 16      */
    int channels = 2;                 /*     声道数 2           */
    rc=snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if(rc<0)
    {
        //perror("\nopen PCM device failed:");
        return -1;
    }
    snd_pcm_hw_params_alloca(&params); //分配params结构体
        
    rc=snd_pcm_hw_params_any(handle, params);//初始化params
    if(rc<0)
    {
       //perror("\nopen PCM device failed:");
        return -1;
    }
    rc=snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);                                 //初始化访问权限
    if(rc<0)
    {
        //perror("\nopen PCM device failed:");
        return -1;

    }
        
    rc=snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);             //设置16位采样精度  
    if(rc<0)
    {
        //perror("\nopen PCM device failed:");
        return -1;
    } 
        
    rc=snd_pcm_hw_params_set_channels(handle, params, channels);  //设置声道,1表示单声>道，2表示立体声
    if(rc<0)
    {
        //perror("\nopen PCM device failed:");
        return -1;
    }    
        
    rc=snd_pcm_hw_params_set_rate_near(handle, params, &rate, &dir);  //设置>频率
    if(rc<0)
    {
        //perror("\nopen PCM device failed:");
        return -1;
    }   
        
    rc = snd_pcm_hw_params(handle, params);
    if(rc<0)
    {
        //perror("\nopen PCM device failed:");
        return -1;
    } 
   
    return 0;            
}

void write_pcm(unsigned char * data,unsigned int len)
{
	if(handle)
	{
		//printf("------------------------write_pcm------------------------------\r\n");
		snd_pcm_writei (handle, data, len);  
	}
}

void close_pcm()
{
	if(handle)
	{
		snd_pcm_drain(handle);
    	snd_pcm_close(handle);
	}
}

/**
 * 声音混合器，可以用来控制音量
 */
void init_mixer()
{
	snd_mixer_open(&mixer, 0);
	snd_mixer_attach(mixer, "default");
	snd_mixer_selem_register(mixer, NULL, NULL);
	snd_mixer_load(mixer);
	//此处不能如此简单的遍历得到snd_mixer_elem_t 指针对象
	//找到Pcm对应的element,方法比较笨拙
	pcm_element = snd_mixer_first_elem(mixer);
	//pcm_element = snd_mixer_elem_next(pcm_element);
	//pcm_element = snd_mixer_elem_next(pcm_element);
	while(pcm_element){
		//strstr 判断给定的字符串是否在src字符串中出现过
		//strcmp 判断两个字符串是否相等
		if(snd_mixer_selem_is_active(pcm_element) && !strcmp(snd_mixer_selem_get_name(pcm_element),"Headphone")){
printf("the pcm_element name is %s \r\n",snd_mixer_selem_get_name(pcm_element));
			//printf("the pcm_element index is %d \r\n",snd_mixer_selem_get_index (pcm_element));
printf("the pcm_element enum items is %d \r\n",snd_mixer_selem_get_enum_items(pcm_element));
			break;
		}else
		{
			pcm_element=snd_mixer_elem_next(pcm_element);
		}
	}
	long int a, b;
	long alsa_min_vol, alsa_max_vol;
	///处理alsa1.0之前的bug，之后的可略去该部分代码
	snd_mixer_selem_get_playback_volume(pcm_element,
	SND_MIXER_SCHN_FRONT_LEFT, &a);
	snd_mixer_selem_get_playback_volume(pcm_element,
	SND_MIXER_SCHN_FRONT_RIGHT, &b);

printf("the a is %d ,the b is %d -----------\r\n",a,b);

	snd_mixer_selem_get_playback_volume_range(pcm_element,
	&alsa_min_vol,
	&alsa_max_vol);
printf("the alsa_min_vol is %d ,the alsa_max_vol is %d \r\n",alsa_min_vol,alsa_max_vol);
	///设定音量范围
	snd_mixer_selem_set_playback_volume_range(pcm_element, 0, 100);
}

/**
 * 修改音量操作 
 * @2017-4-27 10：28分钟测试时，只有当alsa初始化完成的，才能够进行音量的设置
 */
void write_vol(int vol)
{
	if(pcm_element)
	{
		//snd_mixer_selem_set_playback_volume_all(pcm_element,vol);
		//左音量
		snd_mixer_selem_set_playback_volume(pcm_element,SND_MIXER_SCHN_FRONT_LEFT,vol);
		//右音量
		snd_mixer_selem_set_playback_volume(pcm_element,SND_MIXER_SCHN_FRONT_RIGHT,vol);
	}
}

int read_vol()
{
	long ll, lr;
	if(pcm_element){
		//处理事件
		snd_mixer_handle_events(mixer);
		//左声道
		snd_mixer_selem_get_playback_volume(pcm_element,SND_MIXER_SCHN_FRONT_LEFT, &ll);
		//右声道
		snd_mixer_selem_get_playback_volume(pcm_element,SND_MIXER_SCHN_FRONT_RIGHT, &lr);
	}
	return (ll + lr) >> 1;
}