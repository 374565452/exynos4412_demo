#include "mad.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <alsa/asoundlib.h>
#define BUFSIZE 8192
snd_pcm_t*             handle=NULL;        //PCI设备句柄
snd_pcm_hw_params_t*   params=NULL;//硬件信息和PCM流配置
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
/**
 * 网络上下载的，用于制作mp3程序所用-----主要是用来参考写程序
 */

/*
* This is a private message structure. A generic pointer to this structure
* is passed to each of the callback functions. Put here any data you need
* to access from within the callbacks.
*/  


snd_mixer_t * mixer;
snd_mixer_elem_t *pcm_element;

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

void write_vol(int vol)
{
    //snd_mixer_selem_set_playback_volume_all(pcm_element,vol);
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

struct buffer {
    FILE *fp; /*file pointer*/
    unsigned int flen; /*file length*/
    unsigned int fpos; /*current position*/
    unsigned char fbuf[BUFSIZE]; /*buffer*/
    unsigned int fbsize; /*indeed size of buffer*/
};
typedef struct buffer mp3_file;
int soundfd; /*soundcard file*/
unsigned int prerate = 0; /*the pre simple rate*/


/*
* This is perhaps the simplest example use of the MAD high-level API.
* Standard input is mapped into memory via mmap(), then the high-level API
* is invoked with three callbacks: input, output, and error. The output
* callback converts MAD's high-resolution PCM samples to 16 bits, then
* writes them to standard output in little-endian, stereo-interleaved
* format.
*/
static int decode(mp3_file *mp3fp);
int main(int argc, char *argv[])
{
    printf("------------------------------------------------------------");
    long flen, fsta, fend;
    int dlen;
    mp3_file *mp3fp;
    if (argc < 2)
        return 1;
    mp3fp = (mp3_file *)malloc(sizeof(mp3_file));
    if((mp3fp->fp = fopen(argv[1], "r")) == NULL)
    {
        printf("can't open source file.\n");
        return 2;
    }
    fsta = ftell(mp3fp->fp);
    fseek(mp3fp->fp, 0, SEEK_END);
    fend = ftell(mp3fp->fp);
    flen = fend - fsta;
    fseek(mp3fp->fp, 0, SEEK_SET);
    fread(mp3fp->fbuf, 1, BUFSIZE, mp3fp->fp);
    mp3fp->fbsize = BUFSIZE;
    mp3fp->fpos = BUFSIZE;
    mp3fp->flen = flen;
    //set_dsp();
    
     if(set_pcm()!=0)                 //设置pcm 参数
    {
        printf("set_pcm fialed:\n");
        return 1;   
    }
    init_mixer();
    int cur_vol=read_vol();
    printf("the default vol is  %d \r\n",cur_vol);
    int vol = 90;
    if((argc == 3))
    {
        vol =atoi(argv[2]);
        printf("the vol is %d \r\n",vol);
    }
    write_vol(vol);
    cur_vol=read_vol();
    printf("the after config vol is %d \r\n",cur_vol);

    decode(mp3fp);
    close(soundfd);
    fclose(mp3fp->fp);
    return 0;
}
/*
* This is the input callback. The purpose of this callback is to (re)fill
* the stream buffer which is to be decoded. In this example, an entire file
* has been mapped into memory, so we just call mad_stream_buffer() with the
* address and length of the mapping. When this callback is called a second
* time, we are finished decoding.
*/
static
enum mad_flow input(void *data,
struct mad_stream *stream)
{
    mp3_file *mp3fp;
    int ret_code;
    int unproc_data_size; /*the unprocessed data's size*/
    int copy_size;
    mp3fp = (mp3_file *)data;
    if(mp3fp->fpos <= mp3fp->flen)
    {
        unproc_data_size = stream->bufend - stream->next_frame;
    printf("%d------------------------\r\n",unproc_data_size);
        memcpy(mp3fp->fbuf, mp3fp->fbuf+mp3fp->fbsize-unproc_data_size, unproc_data_size);
        copy_size = BUFSIZE - unproc_data_size;
    printf("copy_size is %d ---------------------\r\n",copy_size);
        if(mp3fp->fpos + copy_size > mp3fp->flen)
        {
            copy_size = mp3fp->flen - mp3fp->fpos;
        }
        fread(mp3fp->fbuf+unproc_data_size, 1, copy_size, mp3fp->fp);
        mp3fp->fbsize = unproc_data_size + copy_size;
    printf("the fbsize is %d -------------------------\r\n",mp3fp->fbsize);
        mp3fp->fpos += copy_size;
        /*Hand off the buffer to the mp3 input stream*/
        mad_stream_buffer(stream, mp3fp->fbuf, mp3fp->fbsize);
        ret_code = MAD_FLOW_CONTINUE;
    }
    else
    {
        ret_code = MAD_FLOW_STOP;
    }
    return ret_code;
}
/*
* The following utility routine performs simple rounding, clipping, and
* scaling of MAD's high-resolution samples down to 16 bits. It does not
* perform any dithering or noise shaping, which would be recommended to
* obtain any exceptional audio quality. It is therefore not recommended to
* use this routine if high-quality output is desired.
*/
static inline
signed int scale(mad_fixed_t sample)
{
    /* round */
    sample += (1L <= MAD_F_FRACBITS - 16);
    if(sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if(sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}
/*
* This is the output callback function. It is called after each frame of
* MPEG audio data has been completely decoded. The purpose of this callback
* is to output (or play) the decoded PCM audio.
*/
static
enum mad_flow output(void *data,
struct mad_header const *header,
struct mad_pcm *pcm)
{
   unsigned int nchannels, nsamples,n;
  mad_fixed_t const *left_ch, *right_ch;

  /* pcm->samplerate contains the sampling frequency */

  nchannels = pcm->channels;
  n=nsamples  = pcm->length;
//printf("----the n is %d ,the nsamples is %d -----------------\r\n",n,nsamples);
  left_ch   = pcm->samples[0];
  right_ch  = pcm->samples[1];
  
  unsigned char Output[8196], *OutputPtr;  
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
    mp3_file *mp3fp = data;
    fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n",
    stream->error, mad_stream_errorstr(stream),
    stream->this_frame - mp3fp->fbuf);
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
static int decode(mp3_file *mp3fp)
{
    struct mad_decoder decoder;
    int result;
    /* configure input, output, and error functions */
    mad_decoder_init(&decoder, mp3fp,
    input, 0 /* header */, 0 /* filter */, output,
    error, 0 /* message */);
    /* start decoding */
    result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
    /* release the decoder */
    mad_decoder_finish(&decoder);
    return result;
}