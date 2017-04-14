/**
 * 
 * 在进行此文件进行编译时，一直找不到 -lmad -lasound库文件，因为-L /usr/local/arm-alsa/lib 如果指定上这个目录，则会出现 mad库找不到
 * 情况，同理如果只是加上 /usr/local/libmad/lib，则会出现 asound库找不到现象，两个路径不能同进设置。
 * 为了解决以上问题，将Libmad编译完成的库文件一起拷贝到arm-alsa/lib文件夹下。
 *
 * 经测试编译时，加上-lasound后，不能加-static，因为会报鑵找不到-lasound
 * 经测试完整并且正确的编译命令为 
 * 	arm-none-linux-genueabi-gcc -lasound -lmad -L /usr/local/arm-alsa/lib/ -I/usr/loccal/arm-alsa/include/ -o mp3Playre mp3Player
 * *
**/
#include <stdio.h>
#include <stdlib.h> //使用atoi函数时添加此头文件
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "mad.h"
#include <alsa/asoundlib.h>

static int decode(unsigned char const *, unsigned long);
int set_pcm();
snd_pcm_t*             handle=NULL;        //PCI设备句柄
snd_pcm_hw_params_t*   params=NULL;//硬件信息和PCM流配置

/**
 * 添加音量设置代码
 * *
**/

snd_mixer_t * mixer;
snd_mixer_elem_t *pcm_element;

void init_mixer()
{
	snd_mixer_open(&mixer, 0);
	snd_mixer_attach(mixer, "default");
	snd_mixer_selem_register(mixer, NULL, NULL);
	snd_mixer_load(mixer);
	//找到Pcm对应的element,方法比较笨拙
	pcm_element = snd_mixer_first_elem(mixer);
	pcm_element = snd_mixer_elem_next(pcm_element);
	pcm_element = snd_mixer_elem_next(pcm_element);
	//
	long int a, b;
	long alsa_min_vol, alsa_max_vol;
	///处理alsa1.0之前的bug，之后的可略去该部分代码
	snd_mixer_selem_get_playback_volume(pcm_element,
	SND_MIXER_SCHN_FRONT_LEFT, &a);
	snd_mixer_selem_get_playback_volume(pcm_element,
	SND_MIXER_SCHN_FRONT_RIGHT, &b);

	snd_mixer_selem_get_playback_volume_range(pcm_element,
	&alsa_min_vol,
	&alsa_max_vol);
	printf("the alsa_min_vol is %d ,the alsa_max_vol is %d \r\n",alsa_min_vol,alsa_max_vol);
	///设定音量范围
	snd_mixer_selem_set_playback_volume_range(pcm_element, 0, 100);
}

void write_vol(int vol)
{
	//左音量
	snd_mixer_selem_set_playback_volume(pcm_element,SND_MIXER_SCHN_FRONT_LEFT,vol);
	//右音量
	snd_mixer_selem_set_playback_volume(pcm_element,SND_MIXER_SCHN_FRONT_RIGHT,vol);
}

int read_vol()
{
	long ll, lr;
	//处理事件
	snd_mixer_handle_events(mixer);
	//左声道
	snd_mixer_selem_get_playback_volume(pcm_element,SND_MIXER_SCHN_FRONT_LEFT, &ll);
	//右声道
	snd_mixer_selem_get_playback_volume(pcm_element,SND_MIXER_SCHN_FRONT_RIGHT, &lr);
	return (ll + lr) >> 1;
}

int main(int argc, char *argv[])
{
  struct stat stat;
  void *fdm;

  if (argc != 2)
    {
    printf("Usage: minimad + mp3 file name");
    return 1;
    }
  int fd;
  fd=open(argv[1],O_RDWR);
  if(fd<0)
  {
    perror("open file failed:");
    return 1;
  }    
 
  if (fstat(fd, &stat) == -1 ||stat.st_size == 0)
  {
    printf("fstat failed:\n");
    return 2;
  }
   //printf("stat.st_size=%d\n",stat.st_size);
  
  fdm = mmap(0, stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (fdm == MAP_FAILED)
    return 3;

  
  if(set_pcm()!=0)                 //设置pcm 参数
    {
        printf("set_pcm fialed:\n");
        return 1;   
    }
    init_mixer();
    int cur_vol=read_vol();
    printf("the default vol is  %d \r\n",cur_vol);
    int vol = 34;
    if((argc == 3))
    {
    	vol =atoi(argv[2]);
    	printf("the vol is %d \r\n",vol);
    }
    write_vol(34);
    cur_vol=read_vol();
    printf("the after config vol is %d \r\n",cur_vol);
  decode(fdm, stat.st_size);

  if (munmap(fdm, stat.st_size) == -1)
    return 4;

    snd_pcm_drain(handle);
    snd_pcm_close(handle);

  return 0;
}


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
                perror("\nopen PCM device failed:");
                exit(1);
        }
    snd_pcm_hw_params_alloca(&params); //分配params结构体
        
    rc=snd_pcm_hw_params_any(handle, params);//初始化params
        if(rc<0)
        {
                perror("\nsnd_pcm_hw_params_any:");
                exit(1);
        }
    rc=snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);                                 //初始化访问权限
        if(rc<0)
        {
                perror("\nsed_pcm_hw_set_access:");
                exit(1);

        }
        
    rc=snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);             //设置16位采样精度  
        if(rc<0)
       {
            perror("snd_pcm_hw_params_set_format failed:");
            exit(1);
        } 
        
    rc=snd_pcm_hw_params_set_channels(handle, params, channels);  //设置声道,1表示单声>道，2表示立体声
        if(rc<0)
        {
                perror("\nsnd_pcm_hw_params_set_channels:");
                exit(1);
        }    
        
     rc=snd_pcm_hw_params_set_rate_near(handle, params, &rate, &dir);  //设置>频率
        if(rc<0)
        {
                perror("\nsnd_pcm_hw_params_set_rate_near:");
                exit(1);
        }   
        
         
    rc = snd_pcm_hw_params(handle, params);
        if(rc<0)
        {
        perror("\nsnd_pcm_hw_params: ");
        exit(1);
        } 
   
    return 0;              
}

/*
 * This is a private message structure. A generic pointer to this structure
 * is passed to each of the callback functions. Put here any data you need
 * to access from within the callbacks.
 */

struct buffer {
  unsigned char const *start;
  unsigned long length;
};

/*
 * This is the input callback. The purpose of this callback is to (re)fill
 * the stream buffer which is to be decoded. In this example, an entire file
 * has been mapped into memory, so we just call mad_stream_buffer() with the
 * address and length of the mapping. When this callback is called a second
 * time, we are finished decoding.
 */

static enum mad_flow input(void *data,
            struct mad_stream *stream)
{
  struct buffer *buffer = data;

 printf("this is input\n");
  if (!buffer->length)
    return MAD_FLOW_STOP;

  mad_stream_buffer(stream, buffer->start, buffer->length);

  buffer->length = 0;

  return MAD_FLOW_CONTINUE;
}

/*
 * The following utility routine performs simple rounding, clipping, and
 * scaling of MAD's high-resolution samples down to 16 bits. It does not
 * perform any dithering or noise shaping, which would be recommended to
 * obtain any exceptional audio quality. It is therefore not recommended to
 * use this routine if high-quality output is desired.
 */

static inline signed int scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

/*
 * This is the output callback function. It is called after each frame of
 * MPEG audio data has been completely decoded. The purpose of this callback
 * is to output (or play) the decoded PCM audio.
 */

static enum mad_flow output(void *data,
             struct mad_header const *header,
             struct mad_pcm *pcm)
{
  unsigned int nchannels, nsamples,n;
  mad_fixed_t const *left_ch, *right_ch;

  /* pcm->samplerate contains the sampling frequency */

  nchannels = pcm->channels;
  n=nsamples  = pcm->length;
  left_ch   = pcm->samples[0];
  right_ch  = pcm->samples[1];
  
  unsigned char Output[6912], *OutputPtr;  
  int fmt, wrote, speed, exact_rate, err, dir; 
  
  
//   printf("This is output\n");
   
   
 
   OutputPtr = Output;  
   
   while (nsamples--) 
   {
    signed int sample;

    /* output sample(s) in 16-bit signed little-endian PCM */
    
    sample = scale(*left_ch++);
   
    *(OutputPtr++) = sample >> 0;  
    *(OutputPtr++) = sample >> 8;  
    if (nchannels == 2)  
        {  
            sample = scale (*right_ch++);  
            *(OutputPtr++) = sample >> 0;  
            *(OutputPtr++) = sample >> 8;  
        }  
    
  
  }
 
    OutputPtr = Output;  
    snd_pcm_writei (handle, OutputPtr, n);  
    OutputPtr = Output;     

  return MAD_FLOW_CONTINUE;
}

/*
 * This is the error callback function. It is called whenever a decoding
 * error occurs. The error is indicated by stream->error; the list of
 * possible MAD_ERROR_* errors can be found in the mad.h (or stream.h)
 * header file.
 */

static enum mad_flow error(void *data,
            struct mad_stream *stream,
            struct mad_frame *frame)
{
  struct buffer *buffer = data;
  printf("this is mad_flow error\n");
  fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n",
      stream->error, mad_stream_errorstr(stream),
      stream->this_frame - buffer->start);

  /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

  return MAD_FLOW_CONTINUE;
}

/*
 * This is the function called by main() above to perform all the decoding.
 * It instantiates a decoder object and configures it with the input,
 * output, and error callback functions above. A single call to
 * mad_decoder_run() continues until a callback function returns
 * MAD_FLOW_STOP (to stop decoding) or MAD_FLOW_BREAK (to stop decoding and
 * signal an error).
 */

static int decode(unsigned char const *start, unsigned long length)
{
  struct buffer buffer;
  struct mad_decoder decoder;
  int result;

  /* initialize our private message structure */

  buffer.start  = start;
  buffer.length = length;

  /* configure input, output, and error functions */

  mad_decoder_init(&decoder, &buffer,
           input, 0 /* header */, 0 /* filter */, output,
           error, 0 /* message */);

  /* start decoding */

  result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

  /* release the decoder */

  mad_decoder_finish(&decoder);

  return result;
}