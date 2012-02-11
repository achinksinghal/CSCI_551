#ifndef __PROJ_1_SOCK_H__
#define __PROJ_1_SOCK_H__
#include "common.h"

//structure for sockets
typedef struct _sock_info
{
	int port;//port number
	int sock_fd;//socket file descriptor
	unsigned char checksum[16];//checksum in case of get response used at client side while receiving request
	struct sockaddr_in sock_address;//sock address
}sock_info;


int create_server_sock(sock_info *info);
int create_client_sock(sock_info *info, char *server_name);
int accept_sock(sock_info *info, sock_info *res_sock);
int free_sock(sock_info *info);
int send_sock(sock_info *info, ONE_BYTE *data, int len);
int recv_sock(sock_info *info, ONE_BYTE **data, int *len);
void dealloc_recv_buffer(ONE_BYTE *buffer);
#endif /*__PROJ_1_SOCK_H__*/
