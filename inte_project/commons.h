#ifndef __COMMONS_H
#define __COMMONS_H


//#define MY_PRINTF_DEBUG_ON    //注销掉可以关闭myprintf
//#ifdef MY_PRINTF_DEBUG_ON
/*#define myprintf(fmt,args...) do{printk(KERN_EMERG ""fmt,##args);}while(0)*/
//#define myprintf(x...) do{printk(KERN_EMERG ""x);}while(0)								
//#else
/*#define myprintf(fmt,args...)*/
//#define myprintf(x...)
//#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>

#include <fcntl.h> 
#include <signal.h>  
#include <sys/types.h>  

#include <errno.h> 
/**
 * 调试，自定义printf
 */
#define __DEBUG__ //打开调试
#ifdef __DEBUG__
	#define _debug(format,...)  do{printf("File: " __FILE__",Line:%05d,"format"\r\n",__LINE__,##__VA_ARGS__);}while(0);
#else
	#define _debug(format,...)
#endif

#endif