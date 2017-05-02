#ifndef __NET_PROTOCOL_H
#define __NET_PROTOCOL_H

/**
 * 网络协议解析处理
 * 当前采用的协议格式为：
 * 	0xAA + 命令（2个字节，高字节在前，低字节在后）+数据长度（2个字节，高字节在前，低字节在后）+数据+CRC16（高在前，低在后）+0x55
 * 	其中CRC校验的数据为 0xAA ~ 数据结束（即crc数据之前的所有数据）
 */

#define NET_PROTOCOL_HEADER 0xAA
#define NET_PROTOCOL_TAIL   0x55
#define NET_PROTOCOL_MIN_LEN 8

#define NET_UPDATE_VOL 0x1501 //修改音量
#define NET_PLAY_MP3   0x1502 //播放mp3格式文件
#define NET_PLAY_WAV   0x1503 //播放wav格式文件

#define NET_PROTOCOL_PROCESS_MAX   3

typedef struct
{
	int command;
	void(*call_back)(unsigned char* ,int);
}protocol_action_t;

#define PROTOCOL_ACTION_MAX 256

void process_protocol(unsigned char * recv,int len);

void protocol_send(int command,void * data,int len);

#endif