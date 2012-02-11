#include "sock.h"
#include <sys/types.h>
#include <openssl/md5.h>

//server creates socket using this function
int create_server_sock(sock_info *info)
{
        int serversockfd = -1;
        struct addrinfo req, *res;
        memset(&req, 0, sizeof req);
        req.ai_family = AF_INET;
        req.ai_socktype = SOCK_STREAM;
        req.ai_flags = AI_PASSIVE;
	char port[5];
	memset(port, '\0', 5);
	sprintf(port, "%d", info->port);

	/* code reference: Beej's Tutorial*/	
	/*
	 * Begin code I did not write.
	 * The code is derived from http://beej.us/guide/bgnet/output/html/multipage/getaddrinfoman.html
	 * If the source code requires you to include copyright, put copyright here.
	 */

	getaddrinfo(NULL, port, &req, &res);

        /*creating a static tcp socket to send from supernode*/
        serversockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

        if(serversockfd <= -1)
        {
                //Error checking
                printf("Server Socket not created\n");
                return -1;
        }
	int yes=1;
        if (setsockopt(serversockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
                //Error checking
            printf("setsockopt failed.\n");
            return -1;
        }
                
        /*binding socket with the ip address*/
        if(bind(serversockfd, res->ai_addr, res->ai_addrlen))
        {
                //Error checking
                printf("Server not bind\n");
                return -1;
        }
        /*listening on socket */
        if(listen(serversockfd, 3)< 0)
        {
                //Error checking
                printf("Server not listening\n");
                return -1;
        }

	info->sock_fd = serversockfd;
	/*
	 * End code I did not write.
	 */
	return 1;
}

//client creates socket using this function
int create_client_sock(sock_info *info, char *server_name)
{
	int usersockfd;
	struct addrinfo req, *res;
	memset(&req, 0, sizeof req);
	req.ai_family = AF_INET;  
	req.ai_socktype = SOCK_STREAM;
	req.ai_flags = AI_PASSIVE;     
	char port[5];
	memset(port, '\0', 5);
	sprintf(port, "%d", info->port);
	/* code reference: Beej's Tutorial*/	

	/*
	 * Begin code I did not write.
	 * The code is derived from http://beej.us/guide/bgnet/output/html/multipage/getaddrinfoman.html
	 * If the source code requires you to include copyright, put copyright here.
	 */

	getaddrinfo(server_name, port, &req, &res);
	/*creating a static tcp socket to recieve from users*/
	usersockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(usersockfd <= -1)
	{
		//Error checking
		printf("User socket not created\n");
		return -1;
	}

	if( connect(usersockfd, (struct sockaddr *)res->ai_addr, res->ai_addrlen) < 0)
	{
		//Error checking
		printf("user not connecting with server\n");
		return -1;
	}
	/*
	 * End code I did not write.
	 */
	info->sock_fd = usersockfd;	
	return 1;
}

//server accepts clients connection requests using this function
int accept_sock(sock_info *info, sock_info *res_sock)
{

	socklen_t usersockaddresslen = sizeof(struct sockaddr_in);
	int usersockfd=-1;

	usersockaddresslen=sizeof(struct sockaddr);

	/*accepting socket request*/
	usersockfd = accept(info->sock_fd, (struct sockaddr *)&res_sock->sock_address, (socklen_t *)&usersockaddresslen);

        if(usersockfd< 0)
        {
                //Error checking
                printf("error while accepting socket from client\n");
                return -1;
        }

	res_sock->sock_fd = usersockfd;

	return 1;
}

//free and closes socket using this function
int free_sock(sock_info *info)
{

	/*closing socket request*/
	int ret = close(info->sock_fd);

        if(ret < 0)
        {
                //Error checking
                printf("error while closing\n");
                return -1;
        }

	return 1;
}

//used to send data to socket fd one byte at a time
int send_sock(sock_info *info, ONE_BYTE *data, int len)
{
	int i = 0;
	int ret=-1;

	for (i=0; i < len; i++)
	{
		ret= (int)write(info->sock_fd, &data[i], 1);
		if(ret < 0)
		{
			return -1;
		}
	}

	return 1;
}

//used to receive data to socket fd one byte at a time
//md5 checksum is calculated on the fly
int recv_sock(sock_info *info, ONE_BYTE **data, int *len)
{
	long int i = 0;
	int ret=-1;
	FOUR_BYTE dataLength;
	TWO_BYTE msgType;
	ONE_BYTE header[HEADER_LEN];

	if(data == NULL)
	{
		return -1;
	}

	for (i=0; i < HEADER_LEN; i++)
	{
		ret= (int)read(info->sock_fd, &header[i], 1);
		if(ret <= 0)
		{
			return -1;
		}
	}
	//datalength is extracted here..
	EXTRACT_DATALENGTH_FROM_BUFFER(header, &dataLength, &msgType);

	if(msgType != (TWO_BYTE)GET_RPLY)
	{

		*data = (ONE_BYTE *)malloc((dataLength + HEADER_LEN + 1) * sizeof(ONE_BYTE));

		memcpy(*data, header, HEADER_LEN);

		for (i = HEADER_LEN; i < (dataLength + HEADER_LEN); i++)
		{
			ret= (int)read(info->sock_fd, *data + i, 1);
			if(ret < 0)
			{
				return -1;
			}
		}

		*len = HEADER_LEN + dataLength;
	}
	else
	{	//in case of get request MD5 checksum is calculated...
		*data = (ONE_BYTE *)malloc((HEADER_LEN + 1) * sizeof(ONE_BYTE));
		char *act_data = (ONE_BYTE *)malloc(sizeof(ONE_BYTE));

		memcpy(*data, header, HEADER_LEN);

		MD5_CTX cs_ctx;

		MD5_Init(&cs_ctx);

		for (i = HEADER_LEN; i < (dataLength + HEADER_LEN); i++)
		{
			ret= (int)read(info->sock_fd, act_data, 1);
			if(ret < 0)
			{
				return -1;
			}
			MD5_Update(&cs_ctx, act_data, 1);
		}
		MD5_Final(info->checksum, &cs_ctx);

		*len = HEADER_LEN + dataLength;

	}
	return 1;
}

//used to dealloc receive buffer
void dealloc_recv_buffer(ONE_BYTE *buffer)
{
	free(buffer);
}
