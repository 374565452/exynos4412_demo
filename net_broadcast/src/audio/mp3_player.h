#ifndef __MP3_PLAYER_H
#define __MP3_PLAYER_H

int player_audio(int argc, char * argv[]);

void init_audio();
void write_audio_vol(int vol);
int read_audio_vol();

void audio_play(char * src);

void close_audio();

#endif