#ifndef __MP3_PLAYER_H	
#define __MP3_PLAYER_H

# include <stdio.h>
# include <unistd.h>
# include <sys/stat.h>
# include <sys/mman.h>

# include "mad.h"

#include "cycle_buffer.h"
#include "mp3_audio.h"
#include "alsa_pcm.h"

#define DECODER_LEN 8192
int decode(mp3_dec * mp3);

void close_decode(); 

void player_run();

#endif