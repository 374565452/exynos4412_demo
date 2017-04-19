#include "mp3_audio.h"
mp3_dec mp3_d;
c_buffer mp3_buf;

unsigned char mp3_data[CYCLE_BUFFER_SIZE] ;

void init_mp3_audio(mp3_stream fun)
{
	mp3_d.buf=&mp3_buf;
	mp3_d.decoder=fun;
	init_cycle_buffer(&mp3_buf,mp3_data,CYCLE_BUFFER_SIZE);
}

int write_audio_data(unsigned char * data,int len)
{
	return(put_cycle(mp3_d.buf,data,len));
}

int read_audio_data(unsigned char * data,int len)
{
	return (get_cycle(mp3_d.buf,data,len));
}

int get_mp3_audio_len()
{
	return (get_data_size(mp3_d.buf));
}