#ifndef __MP3_MAD_H	
#define __MP3_MAD_H

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "mad.h"
#include "cycle_buffer.h"

#define DECODER_BUF_SIZE 8192

typedef enum mad_flow(*mp3_mad_stream)(struct mad_stream *);
typedef struct mp3_t
{
	c_buffer * buf;
	enum mad_flow (*mad_callback)(struct mad_stream *stream);
}mp3_mad_decoder;

struct buffer {
    FILE *fp; /*file pointer*/
    unsigned int flen; /*file length*/
    unsigned int fpos; /*current position*/
    unsigned char fbuf[DECODER_BUF_SIZE]; /*buffer*/
    unsigned int fbsize; /*indeed size of buffer*/
};
typedef struct buffer mp3_file;

int init_mad(mp3_mad_stream  callback);

int decode();

void close_decode(); 

void player_run();

int write_audio_data(unsigned char * data,int len);

int read_audio_data(unsigned char * data,int len);

int get_mp3_audio_len();
int get_mp3_capacity();

#endif