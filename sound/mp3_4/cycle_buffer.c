#include "cycle_buffer.h"

/**
 * 返回循环缓冲区中当前的数据长度
 */
int get_data_size(c_buffer *buf)
{
	int data_size=0;
	if(buf->wx.bit.index >=buf->rx.bit.index)
	{
		data_size=buf->wx.bit.index-buf->rx.bit.index;
	}else
	{
		data_size=buf->length-buf->rx.bit.index+buf->wx.bit.index;
	}
	return data_size;
}
/**
 * 返回指定循环缓冲区的剩余容量
 */
int get_extra_capacity(c_buffer * buf)
{
	int data_len=get_data_size(buf);
	return (buf->length - data_len);
}

void init_cycle_buffer(c_buffer * buf,unsigned char * data,unsigned int len)
{
	buf->data=data;
	buf->length=len;
	buf->wx.bit.index=0;
	buf->rx.bit.index=0;
}

/**
 * 向循环缓冲区中写入指定长度的数据，如果要写入的数据长度比缓冲区剩余的空间大，则返回为-1
 * 否则返回实际写入的数据长度
 */
int put_cycle(c_buffer * buf,unsigned char *data,int len)
{
	int i =0 ;
	int extra_capacity=get_extra_capacity(buf);
	if(len > extra_capacity)
	{
		return -1;
	}
	for(i=0;i<len;i++)
	{
		if( (buf->wx.bit.index + 1) != (buf->rx.bit.index))
		{
			buf->data[buf->wx.bit.index] = data[i];
			buf->wx.bit.index ++;
		}
	}
	return (i);
}

/**
 * 返回实际读取到的数据长度
 */
int get_cycle(c_buffer * buf,unsigned char * data,int len)
{
	int i =0;
	int cur_data_size=get_data_size(buf);
	if(len>=cur_data_size) //如果当前缓冲区中的数据小于需要获取的数据时
	{
		len=cur_data_size;
	}

	for(i=0;i<len;i++)
	{
		if(buf->rx.bit.index != buf->wx.bit.index)
		{
			data[i]=buf->data[buf->rx.bit.index];
			buf->rx.bit.index++;
		}
	}
	return (i);
}