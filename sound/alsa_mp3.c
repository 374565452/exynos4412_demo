/*
 * 嵌入式alsa+libmad实现mp3播放
 * libmad - MPEG audio decoder library
 * Copyright (C) 2000-2004 Underbit Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: minimad.c,v 1.4 2004/01/23 09:41:32 rob Exp $
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include "alsa/asoundlib.h"
#include "mad.h"
#include "player_ipc_interface.h"

/*
 * This is perhaps the simplest example use of the MAD high-level API.
 * Standard input is mapped into memory via mmap(), then the high-level API
 * is invoked with three callbacks: input, output, and error. The output
 * callback converts MAD's high-resolution PCM samples to 16 bits, then
 * writes them to standard output in little-endian, stereo-interleaved
 * format.
 */

#define DEBUG(x,y...)	//{printf("[ %s : %s : %d] ",__FILE__, __func__, __LINE__); printf(x,##y); printf("\n");}
#define ERROR(x,y...)	{printf("[ %s : %s : %d] ",__FILE__, __func__, __LINE__); printf(x,##y); printf("\n");}

typedef struct mad_decoder_s {
  struct mad_synth  synth;
  struct mad_stream stream;
  struct mad_frame  frame;
} mad_decoder_t;

extern player_shm_t *shm_buff;
static int to_stop_play = 0;

static inline void play_frame(mad_decoder_t *mad_decoder);
static inline signed int scale(mad_fixed_t sample);
static int set_pcm();
static snd_pcm_t *handle = NULL;        //PCI设备句柄
static snd_pcm_hw_params_t* params = NULL;//硬件信息和PCM流配置
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int libmad_play_mp3(char *mp3_file, res_type_t type)
{
	struct stat stat;
	int fd;
	int count;
	char *buff = NULL;
	mad_decoder_t *this;
	int buf_size = 1152;
	int buf_len = 0;

	this = calloc(1, sizeof(mad_decoder_t));
	buff = malloc(buf_size);
	if(buff)
		memset(buff,0,buf_size);

	mad_synth_init  (&this->synth);
	mad_stream_init (&this->stream);
	mad_frame_init  (&this->frame);

	if(NULL == mp3_file || strlen(mp3_file) == 0){
		ERROR("mp3_file is NULL of empty.");
		return -1;
	}

	if(shm_buff){
		strcpy(shm_buff->url, mp3_file);
		shm_buff->type = type;
		shm_buff->is_playing = 1;
	}else{
		ERROR("player share memory shm_buff is NULL\n");
		return -2;
	}

	fd = open(mp3_file, O_RDONLY);
	if(fd < 0){
		perror("open file failed:");
		return -2;
	}    

	if (fstat(fd, &stat) == -1 || stat.st_size == 0){
		ERROR("fstat failed:\n");
		return -3;
	}

	if(set_pcm(fd, this)!=0){                 //设置pcm 参数
		ERROR("set_pcm fialed:\n");
		return -4;   
	}

	while((count = read(fd, buff+buf_len, buf_size-buf_len)) > 0){
		buf_len += count;
		if(count < 0){
			ERROR("read mp3_file error:%s", strerror(errno));
			return -5;
		}
		if(to_stop_play){
			break;
		}
		while(1){
			int ret;
			mad_stream_buffer (&this->stream, (unsigned char *)buff, buf_len);
			ret=mad_frame_decode (&this->frame, &this->stream);
			if (this->stream.next_frame) {
				int num_bytes =
					(char*)buff+buf_len - (char*)this->stream.next_frame;
				memmove(buff, this->stream.next_frame, num_bytes);
				buf_len = num_bytes;
			}
			if (ret == 0)
				play_frame(this);
			// error! try to resync!
			if(this->stream.error==MAD_ERROR_BUFLEN) break;
		}
	}

	if(shm_buff){
		memset(shm_buff->url, 0, URL_SIZE);
		shm_buff->type = UNKNOWN_TYPE;
		shm_buff->is_playing = 0;
		pthread_mutex_lock(&mutex);
		to_stop_play = 0;
		pthread_mutex_unlock(&mutex);
	}
	mad_synth_finish (&this->synth);
	mad_frame_finish (&this->frame);
	mad_stream_finish(&this->stream);
	free(this);
	free(buff);
	close(fd);
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	return 0;
}

static inline void play_frame(mad_decoder_t *this)
{
	mad_synth_frame (&this->synth, &this->frame);

	unsigned int nchannels, nsamples,n;
	mad_fixed_t const *left_ch, *right_ch;
	struct mad_pcm      *pcm = &this->synth.pcm;

	/* pcm->samplerate contains the sampling frequency */

	nchannels = pcm->channels;
	n=nsamples  = pcm->length;
	left_ch   = pcm->samples[0];
	right_ch  = pcm->samples[1];

	unsigned char Output[6912], *OutputPtr;  
	//int fmt, wrote, speed, exact_rate, err, dir; 
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
	snd_pcm_writei (handle, Output, n);  
}

void libmad_stop_play_mp3()
{
	DEBUG("url:%s, type:%d, is_playing:%d\n", shm_buff->url, shm_buff->type, shm_buff->is_playing);
	if(shm_buff && shm_buff->type == CAN_STOP && shm_buff->is_playing){
		memset(shm_buff->url, 0, URL_SIZE);
		shm_buff->type = UNKNOWN_TYPE;
		shm_buff->is_playing = 0;
		pthread_mutex_lock(&mutex);
		to_stop_play = 1;
		pthread_mutex_unlock(&mutex);
	}
}

static int set_pcm(int fd, mad_decoder_t *this)
{
	int rc;     
	int dir=0;
	unsigned int rate = 44100;;                /* 采样频率 44.1KHz*/
	//int format = SND_PCM_FORMAT_S16_LE; /*     量化位数 16      */
	int channels = 2;                 /*     声道数 2           */

	char *buff = NULL;
	int buf_size = 1152;
	int buf_len = 0;
	int count;
	int flag = 0;
	buff = malloc(buf_size);
	if(buff)
		memset(buff,0,buf_size);

	while((count = read(fd, buff+buf_len, buf_size-buf_len)) > 0){
		buf_len += count;
		if(count < 0){
			ERROR("read mp3_file error:%s", strerror(errno));
			return -5;
		}
		while(1){
			int ret;
			mad_stream_buffer (&this->stream, (unsigned char *)buff, buf_len);
			ret=mad_frame_decode (&this->frame, &this->stream);
			if (this->stream.next_frame) {
				int num_bytes =
					(char*)buff+buf_len - (char*)this->stream.next_frame;
				memmove(buff, this->stream.next_frame, num_bytes);
				buf_len = num_bytes;
			}
			if (ret == 0){
				channels = MAD_NCHANNELS(&(this->frame.header)); //动态设置通道数，采样率
				rate = this->frame.header.samplerate;
				flag = 1;
				break;
			}
		}
		if(flag)
			break;
	}
	lseek(fd, 0, SEEK_SET);
	free(buff);

	rc=snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if(rc<0){
		perror("\nopen PCM device failed:");
		exit(1);
	}
	snd_pcm_hw_params_alloca(¶ms); //分配params结构体

	rc=snd_pcm_hw_params_any(handle, params);//初始化params
	if(rc<0){
		perror("\nsnd_pcm_hw_params_any:");
		exit(1);
	}
	rc=snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);                                 //初始化访问权限
	if(rc<0){
		perror("\nsed_pcm_hw_set_access:");
		exit(1);

	}

	rc=snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);             //设置16位采样精度  
	if(rc<0){
		perror("snd_pcm_hw_params_set_format failed:");
		exit(1);
	} 

	rc=snd_pcm_hw_params_set_channels(handle, params, channels);  //设置声道,1表示单声>道，2表示立体声
	if(rc<0){
		perror("\nsnd_pcm_hw_params_set_channels:");
		exit(1);
	}    

	rc=snd_pcm_hw_params_set_rate_near(handle, params, &rate, &dir);  //设置>频率
	if(rc<0){
		perror("\nsnd_pcm_hw_params_set_rate_near:");
		exit(1);
	}   

	rc = snd_pcm_hw_params(handle, params);
	if(rc<0){
		perror("\nsnd_pcm_hw_params: ");
		exit(1);
	} 

	return 0;              
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
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}