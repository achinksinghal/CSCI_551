#ifndef __PROJ_1_SERVER_H__
#define __PROJ_1_SERVER_H__
#include "sock.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

//functions that are used at server side for handling client requests
void handleAdrReq(Request *req, sock_info *client_sock);
void handleGetReq(Request *req, sock_info *client_sock);
void handleFszReq(Request *req, sock_info *client_sock);
void handleFailReq(Request *req, sock_info *client_sock);
void *server_processing(void *arg);
void *exit_processing(void *arg);
#endif /*__PROJ_1_SERVER_H__*/
