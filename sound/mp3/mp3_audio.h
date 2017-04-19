/**
 * libmad alsa 集成测试demo ,
 */

#ifndef __MP3_AUDIO_H
#define __MP3_AUDIO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cycle_buffer.h"
#include "mad.h"

typedef enum mad_flow(*mp3_stream)(struct mad_stream *);
typedef struct mp3_t
{
	c_buffer * buf;
	enum mad_flow (*decoder)(struct mad_stream *stream);
}mp3_dec;

extern mp3_dec mp3_d;

void init_mp3_audio(mp3_stream fun);

int write_audio_data(unsigned char * data,int len);

int read_audio_data(unsigned char * data,int len);

int get_mp3_audio_len();

#endif