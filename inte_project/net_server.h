#ifndef __NET_SERVER_H
#define __NET_SERVER_H

#include "commons.h"

void init_net_server(void);

int start_server(char * server_ip,int port);

int net_send(void * buf,int length);

int net_recv(void * buf,int length);

unsigned char get_connected_state(void);

void set_connected_state(unsigned char s);

void close_con(void);

void close_thread(void);

#endif