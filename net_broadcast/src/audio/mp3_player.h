#ifndef __MP3_PLAYER_H
#define __MP3_PLAYER_H

int player_audio(int argc, char * argv[]);

void init_audio();
void write_audio_vol(int vol);
int read_audio_vol();

void audio_play(char * src);

void close_audio();

/**
 * @2017-4-27 加入网络处理函数
 */

void set_net_audio_vol(int vol);

int insert_net_audio_datas(void * data,int len);

#endif