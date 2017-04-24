#ifndef __C_UTILS_H
#define __C_UTILS_H

#define __DEBUG__ //打开调试
#ifdef __DEBUG__
	#define _debug(format,...)  do{printf("File: " __FILE__",Line:%05d,"format"\r\n",__LINE__,##__VA_ARGS__);}while(0);
#else
	#define _debug(format,...)
#endif


#define NET_RECON_DELAY 30 //网络断开后，重连间隔时间

void init_kill_signal(void);

#endif