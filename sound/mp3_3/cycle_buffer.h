#ifndef __CYCLE_BUFFER_H	
#define __CYCLE_BUFFER_H	

#include <stdio.h>
#include <stdlib.h>

#define CYCLE_BUFFER_SIZE 64*1024

typedef union{
	struct{
		unsigned index	:16; //最大为8192 2^3*2^10 数据
	}bit;
	unsigned int max;
}cycle_t;

typedef struct cycle_buffer{

	unsigned char * data;
	unsigned int length;
	cycle_t wx; //向缓冲区中写数据指针
	cycle_t rx; //从缓冲区中读数据指针
}c_buffer;

/**
 * 初始化指定的循环缓冲区
 * @param buf  [指定的循环缓冲区]
 * @param data [实际存放数据的位置]
 * @param len  [数据长度]
 */
void init_cycle_buffer(c_buffer * buf,unsigned char * data,unsigned int len); 

int put_cycle(c_buffer * buf,unsigned char *data,int len);

int get_cycle(c_buffer * buf,unsigned char * data,int len);

int get_data_size(c_buffer *buf);

int get_extra_capacity(c_buffer * buf);

#endif