#ifndef __C_ERROR_H	
#define __C_ERROR_H

#define NET_CREATE_RECV_THREAD_ERROR             0x0101  //创建数据接收线程出错
#define NET_CREATE_RECON_THREAD_ERROR            0x0102  //创建重新创建socket连接出错
#define NET_CON_ERROR                            0x0103  //网络连接时出错
#define NET_SEND_ERROR                           0x0104  //发送时出错 ，现在只是表明没有进行连接

#endif