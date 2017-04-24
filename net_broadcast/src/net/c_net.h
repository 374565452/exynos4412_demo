#ifndef __C_NET_H
#define __C_NET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <pthread.h>


#include "c_error.h"
#include "c_utils.h"

int init_c_net(void);

int net_start(char * server_ip,int port);

int net_send(void * buf,int length);

int net_recv(void * buf,int length);

unsigned char get_connected_state(void);

void set_connected_state(unsigned char s);

void close_con(void);


#endif