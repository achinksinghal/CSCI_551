#include "server.h"
#include <signal.h>

//checks if the given string is a number or not
int isNum(char *str)
{
	unsigned int i=0;
	for(i=0; i<strlen(str); i++)
	{
		if(str[i] < '0' || str[i] > '9')
		return 0;
	}
	return 1;
}


//checks if the given string is a right host or not, 
//if yes then return the ip address and port number
int checkHost(char *host, char *host_ip, int *port)
{
	int length = strlen(host);
	int i= length - 1;
	while(host[i] != ':' && i > 0)
	{
		i--;
	}
	if(isNum(&host[i+1]))
	{
		*port = atoi(&host[i+1]);
	}
	else
	{
		return 0;
	}
	host[i] = '\0';
	strcpy(host_ip, host);
	return 1;
}

//used to count running threads
int thread_count=0;
//used to check exit status
int exit_status_on=0;
//socket info variable
sock_info server_sock;
//set is for signal handling
sigset_t set;

//main function for server side
int main(int argc, char **argv)
{

	int option_index=0;
	int time=60;
	char time_char[100];
	char port_char[100];
	int port=-1;
	int retVal=-1;
	memset(time_char, '\0', 100);
	memset(port_char, '\0', 100);

	//taking command line arguments using getopt_long
	static struct option long_options[] = 
	{
		{(char *)"time", 1, 0, 't'},
		{(char *)"port", required_argument, 0, 'm'},
		{0, 0, 0, 0}
	};

	if(argc < 3)
	{
		printf("malformed command\n");
		exit(0);
	}

	while (1)
	{
		retVal = getopt_long(argc, argv, "m:t:d:", long_options, &option_index);

		if (retVal == -1)
		{
			break;
		}

		//different cases for command line arguments
		switch (retVal)
		{
			case 't':
				strcpy((char *)time_char, optarg);
				if(isNum(time_char))
				{
					time = atoi(time_char);
				}
				else
				{
					printf("malformed command\n");
					exit(0);
				}
				break;
			case 'm':
				strcpy((char *)port_char, optarg);
				if(isNum(port_char))
				{
					port = atoi(port_char);
				}
				else
				{
					printf("malformed command\n");
					exit(0);
				}
				break;
			default:
				printf("malformed command\n");
				exit(0);



		}
	}

	if(port < 0)
	{
		printf("malformed command\n");
		exit(0);
	}

	/*main logic start here*/
	//signal handling for blocking SIGPIPE interrupts
        sigemptyset(&set);
        sigaddset(&set, SIGPIPE);
        pthread_sigmask(SIG_BLOCK, &set, NULL);

	server_sock.port = port;
	if(create_server_sock(&server_sock) != 1)
	{
		printf("malformed command\n");
		exit(0);
	}


	exit_status_on = 0;
	thread_count=0;
	pthread_t exit_thread;
	//this thread is created for giving server the gracefull shutdown
	pthread_create(&exit_thread, NULL, exit_processing,(void *)time);

	fd_set rd_set;
	int select_ret=0;
	//fd set is made empty
	FD_ZERO(&rd_set);
	//entry is added in fd set
	FD_SET(server_sock.sock_fd, &rd_set);
	while(1)
	{
		if(exit_status_on == 0)
		{
			//if exit status is 0 fd set is made empty
			FD_ZERO(&rd_set);
			FD_SET(server_sock.sock_fd, &rd_set);
			select_ret = select( server_sock.sock_fd + 1, &rd_set, 0, 0, 0);
			if(select_ret < 0)
			{
				continue;
			}
			if(exit_status_on == 0)
			{
				//select selected the server sock fd to accept the new client connection request
				if(FD_ISSET(server_sock.sock_fd, &rd_set))
				{
					pthread_t tid;
					sock_info *client_info = new sock_info;
					accept_sock(&server_sock, client_info);
					//new thread is created that would accept data from new request and cater the response
					pthread_create(&tid, NULL, server_processing,(void *)client_info);
				}
			}
			else 
			{
				//frees the socket info structure in case of failure
				free_sock(&server_sock);
				if(thread_count == 0)
				{
					break;
				}
				else
				{
					//no busy wait
					usleep(1000);
				}
			}
		}
		else 
		{
			if(thread_count == 0)
			{
				break;
			}
			else
			{
				//no busy wait
				usleep(1000);
			}
		}


	}
	return 1;
}

//processing the exit control of server
void *exit_processing(void *arg)
{
	int time = (int)arg;
	
	sleep(time);
	free_sock(&server_sock);
	exit_status_on = 1;
	while(thread_count != 0)
	{
		usleep(1000);
	}
	exit(0);
}

//server processes the client requests
void *server_processing(void *arg)
{
	ONE_BYTE *buffer;
	Request *req;
	sock_info *client_info = (sock_info *)arg;
	int len;
	thread_count++;

	//receive data
	recv_sock(client_info, &buffer, &len);

	req = alloc_request(0);
	UNPACK_REQUEST(req, buffer, &len);
	dealloc_recv_buffer(buffer);

	char host_ip[16];
	strcpy(host_ip, inet_ntoa(client_info->sock_address.sin_addr));

	//prints the client data...
	printf("Received %d bytes from %s.\n", len, host_ip);
        printf("  MessageType: 0x%hx\n", req->msgType);
        printf("       Offset: 0x%08x\n", req->offset);
        printf("  ServerDelay: 0x%02x\n", req->serverDelay);
        printf("   DataLength: 0x%08x\n", req->dataLength);

	//case of Address request...
	if(req->msgType == (TWO_BYTE)ADR_REQ)
	{
		handleAdrReq(req, client_info);
	}
	//case of Get request...
	else if(req->msgType == (TWO_BYTE)GET_REQ)
	{
		handleGetReq(req, client_info);
	}
	//case of Filesize request...
	else if(req->msgType == (TWO_BYTE)FSZ_REQ)
	{	
		handleFszReq(req, client_info);
	}
	//case of Failure request...
	else if(req->msgType == (TWO_BYTE)ALL_FAIL)
	{	
		handleFailReq(req, client_info);
	}
	//any other corrupted case...
	else 
	{
		handleFailReq(req, client_info);
	}

	dealloc_request(req);
	thread_count--;
	return (void *)NULL;
}

//handling Address request
void handleAdrReq(Request *req, sock_info *client_sock)
{
	struct hostent *host;
	char *hostname = req->data;
	hostname[req->dataLength]='\0';
	//getting host name using gethostbyname...
	host = gethostbyname(hostname);

	char host_ip[16];
	if(host != NULL)
	{
		strcpy(host_ip, inet_ntoa(*((struct in_addr *)host->h_addr_list[0])));
	}
	Request *response;
        char *buffer;
        int len = 0;

        /*sending response to client*/
	if(host != NULL)
	{
		//filling response..
		response = alloc_request(strlen(host_ip));
		response->msgType = ADR_RPLY;
		response->offset = 0;
		response->serverDelay = 0;
		response->dataLength = strlen(host_ip);
		memcpy(response->data, host_ip, response->dataLength);
	}
	else
	{
		//sending failure response..
		response = alloc_request(0);
		response->msgType = ADR_FAIL;
		response->offset = 0;
		response->serverDelay = 0;
		response->dataLength = 0;
	}
	
	//response is packed and then sent..
        PACK_REQUEST(response, &buffer, &len);

	if(req->serverDelay > 0)
	{
		sleep(req->serverDelay);
	}
        send_sock(client_sock, buffer, len);
        DEALLOC_PACKING_BUFFER(buffer);
        dealloc_request(response);
}

//handling filesize requests
void handleFszReq(Request *req, sock_info *client_sock)
{

	struct stat fileStat;
	int ret=-1;
	Request *response;
        char *buffer;

	char *filename = req->data;
	filename[req->dataLength]='\0';
	//file size is calculated using stat()...
	ret = stat(filename, &fileStat);

        int len = 0;

        /*sending response to client*/
	if(ret == 0)
	{
		//filling response..
		response = alloc_request(0);
		response->msgType = FSZ_RPLY;
		response->offset = 0;
		response->serverDelay = 0;
		response->dataLength = sizeof(fileStat.st_size);
		response->data = (ONE_BYTE *)&fileStat.st_size;
	}
	else
	{
		//sending failure response..
		response = alloc_request(0);
		response->msgType = FSZ_FAIL;
		response->offset = 0;
		response->serverDelay = 0;
		response->dataLength = 0;
	}
	
	//response is packed and then sent..
        PACK_REQUEST(response, &buffer, &len);

	if(req->serverDelay > 0)
	{
		sleep(req->serverDelay);
	}
	//sending response to client
        send_sock(client_sock, buffer, len);
        DEALLOC_PACKING_BUFFER(buffer);
	response->dataLength = 0;
        dealloc_request(response);
}

//handling Get request
void handleGetReq(Request *req, sock_info *client_sock)
{
	int i = 0;
	int ret=0;
	int retVal=0;
	Request *response;
        char *buffer;
	FILE *file;
	long int size;

	char *filename = req->data;
	filename[req->dataLength]='\0';
	file = fopen(filename,"r");
       
	int len = 0;
	
        /*sending response to client*/
	do
	{
		if(file != NULL)
		{
			long int read_size = 0;
			if((retVal = fseek(file, req->offset, SEEK_SET)) != 0)
			{
				printf("ERROR: File handling: %s\n", strerror(errno));
				ret = -1;
				break;
			}

			size = ftell(file);
			if(size == -1)
			{
				printf("ERROR: File handling: %s\n", strerror(errno));
				ret = -1;
				break;
			}
			if(fseek(file, 0, SEEK_END))
			{
				printf("ERROR: File handling: %s\n", strerror(errno));
				ret = -1;
				break;
			}
			read_size = ftell(file);
			if(read_size == -1)
			{
				printf("ERROR: File handling: %s\n", strerror(errno));
				ret = -1;
				break;
			}
			//size is calculated after the consideration with offset...
			size = read_size - size;
			read_size = 0;
			//file indicator pointer is set...
			if(fseek(file, req->offset, SEEK_SET))
			{
				printf("ERROR: File handling: %s\n", strerror(errno));
				ret = -1;
				break;
			}
		}
		else
		{
			ret = -1;
		}
	}
	while(0);
	if(ret == -1)
	{
		//if any failure occured then failure message is sent to client
		response = alloc_request(0);
		response->msgType = GET_FAIL;
		response->offset = 0;
		response->serverDelay = 0;
		response->dataLength = 0;

		//packing response and then sending it to server
		PACK_REQUEST(response, &buffer, &len);

		if(req->serverDelay > 0)
		{
			sleep(req->serverDelay);
		}
		//sending response to server
		send_sock(client_sock, buffer, len);
		DEALLOC_PACKING_BUFFER(buffer);
		response->dataLength = 0;
		dealloc_request(response);

	}
	else
	{
		if(req->serverDelay > 0)
		{
			sleep(req->serverDelay);
		}
		//filling response
		response = alloc_request(0);
		response->msgType = GET_RPLY;
		response->offset = 0;
		response->serverDelay = 0;
		response->dataLength = size;

		//header is packed and then sent
		{
			len = HEADER_LEN;
			buffer = (ONE_BYTE *)malloc((len) * sizeof(ONE_BYTE));
			ONE_BYTE *tempBuff = buffer;
			if(buffer == NULL)
				return;

			PACK_2_BYTE(&buffer, response->msgType);
			PACK_4_BYTE(&buffer, response->offset);
			PACK_1_BYTE(&buffer, response->serverDelay);
			PACK_4_BYTE(&buffer, response->dataLength);

			buffer = tempBuff;
		}
		//response header is sent
		{
			int retVal=-1;

			for (i=0; i < len; i++)
			{
				retVal= (int)write(client_sock->sock_fd, &buffer[i], 1);
				if(retVal < 0)
				{
					free_sock(client_sock);
					return;
				}
			}
		}
		free(buffer);

		response->dataLength = 0;
		dealloc_request(response);

//malloc size is defined
#define MAX_ALLOC 512
		long int size_read=0, read_size=0;
		long int size_left=size;
		long int given_size;
		char *sending_buffer;
		if(size > MAX_ALLOC)
		{
			sending_buffer = (char *)malloc(MAX_ALLOC);	
		}
		else
		{
			sending_buffer = (char *)malloc(size);	
		}

		//response message data is first gathered then sent to client....
		while(size_read != size)
		{
			if(exit_status_on == 1)
			{
				free_sock(client_sock);
				return;
			}

			if(size_left < MAX_ALLOC)
			{
				given_size = size_left; 
			}
			else
			{
				given_size = MAX_ALLOC; 
			}
			//reading data from the file...
			read_size = fread(sending_buffer, given_size, 1, file);
			if(read_size != 1)
			{
				free_sock(client_sock);
				ret = -1;
				return;
			}
			//sending read data to client
			for (i=0; i < given_size; i++)
			{
				ret= (int)write(client_sock->sock_fd, &sending_buffer[i], 1);
				if(ret < 0)
				{
					return;
				}
			}
			size_left = size_left - given_size;
			size_read = size_read + given_size;

		}
		fclose(file);
#undef MAX_ALLOC
	}

}

//handling failure request
void handleFailReq(Request *req, sock_info *client_sock)
{

	Request *response;
	char *buffer;

	int len = 0;

	/*filling and sending faliure response to client*/
	response = alloc_request(0);
	response->msgType = ALL_FAIL;
	response->offset = 0;
	response->serverDelay = 0;
	response->dataLength = 0;

	PACK_REQUEST(response, &buffer, &len);

	if(req->serverDelay > 0)
	{
		sleep(req->serverDelay);
	}
	
	//sending response
	send_sock(client_sock, buffer, len);
	DEALLOC_PACKING_BUFFER(buffer);
	response->dataLength = 0;
	dealloc_request(response);
}

