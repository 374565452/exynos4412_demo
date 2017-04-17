#include "mp3_audio.h"

unsigned char mp3_data[CYCLE_BUFFER_SIZE] ;

void init_mp3_audio()
{
	init_cycle_buffer(&mp3_buf,mp3_data,CYCLE_BUFFER_SIZE);
}

int write_audio_data(unsigned char * data,int len)
{
	return(put_cycle(&mp3_buf,data,len));
}

int read_audio_data(unsigned char * data,int len)
{
	return (get_cycle(&mp3_buf,data,len));
}

