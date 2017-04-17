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

extern c_buffer mp3_buf;

void init_mp3_audio();

int write_audio_data(unsigned char * data,int len);

int read_audio_data(unsigned char * data,int len);

#endif