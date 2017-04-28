#ifndef __MP3_PLAYER_H
#define __MP3_PLAYER_H

int player_audio(int argc, char * argv[]);

void init_audio();
void write_audio_vol(int vol);
int read_audio_vol();

void audio_play(char * src);

void net_play();

void close_audio();

/**
 * @2017-4-27 加入网络处理函数
 */
//由于此处函数的声明忘记最后个')'，导致出现storage class specified for parameter此异常错误
void set_net_audio_vol(unsigned char * data,int len);

void insert_net_audio_datas(unsigned char * data,int len);

#endif