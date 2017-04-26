#include "net_protocol.h"
#include "c_net.h"
#include "mp3_player.h"

static void net_update_vol(unsigned char * data,int len);

#define SEND_BUF_SIZE 1024
unsigned char send_buf[SEND_BUF_SIZE];

static void pre_net_send(int command);

protocol_action_t net_actions[PROTOCOL_ACTION_MAX]=
{
	{NET_UPDATE_VOL,net_update_vol},
	{0,NULL}
};

void process_protocol(unsigned char * recv,int len)
{
	int i=0;
	//这里的recv是完整的数据
	int command=(int)((int)(recv[1]<<8)+(int)recv[2]);
	int data_len=(int)((int)(recv[3]<<8)+(int)recv[4]);
	for(i=0;i<PROTOCOL_ACTION_MAX;i++)
	{
		protocol_action_t action_t=net_actions[i];
		if(action_t.command==0 && action_t.call_back == NULL)
		{
			printf("error command-------not found--------\r\n");
			break;
		}
		if(action_t.command == command)
		{
			if(action_t.call_back != NULL)
			{
				action_t.call_back(recv+5,data_len);
			}
		}
	}
}

void protocol_send(int command,void * data,int len)
{
	pre_net_send(command);
	send_buf[3]=(len>>8)&0xFF;
	send_buf[4]=(len & 0xFF);
	
	memcpy(send_buf+5,data,len);
	send_buf[5+len] = 0x13; //此处应该是计算CRC的
	send_buf[6+len] = 0x14;
	send_buf[7+len]=NET_PROTOCOL_TAIL;
	net_send(send_buf,len+NET_PROTOCOL_MIN_LEN);
}

void net_update_vol(unsigned char * data,int len)
{
	printf("------the data len is %d --------\r\n",len);
	int vol =(int)data[0];
	write_audio_vol(vol);
	unsigned char cur_vol=(unsigned char)read_audio_vol();
	protocol_send(NET_UPDATE_VOL,&cur_vol,1);
}

void pre_net_send(int command)
{
	memset(send_buf,0,SEND_BUF_SIZE);
	send_buf[0]=NET_PROTOCOL_HEADER;
	send_buf[1]=(command>>8)&0xFF;
	send_buf[2]=(command & 0xFF);
}