#ifndef __ALSAS_PCM_H
#define __ALSAS_PCM_H

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <alsa/asoundlib.h>
int set_pcm();

void write_pcm(unsigned char * data,unsigned int len);

void close_pcm();

/**
 * 声音混合器，可以用来控制音量
 */
void init_mixer();

/**
 * 修改音量操作
 */
void write_vol(int vol);

#endif