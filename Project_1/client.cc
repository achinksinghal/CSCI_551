#include "sock.h"

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

//client main's program...
int main(int argc, char **argv)
{

	int option_index=0;
	int delay=0;
	char delay_char[100];
	char offset_char[100];
	char host[100];
	char host_ip[100];
	int offset=-1;
	int port=-1;
	int retVal=-1;
	char req[5];
	char print_on =0;
	TWO_BYTE msgType = ALL_FAIL;
	char str[1000];

	//memset is done for strings
	memset(delay_char, '\0', 100);
	memset(offset_char, '\0', 100);
	memset(host, '\0', 100);
	memset(host_ip, '\0', 100);
	memset(req, '\0', 5);
	memset(str, '\0', 1000);

	//getopt_long is used for command line arguments parsing
	static struct option long_options[] = 
	{
		{(char *)"delay", 1, 0, 'd'},
		{(char *)"offset", 1, 0, 'o'},
		{(char *)"host", no_argument, 0, 'm'},
		{0, 0, 0, 0}
	};

	if(argc <= 3)
	{
		printf("malformed command\n");
		exit(0);
	}

	//checked the request type
	strcpy(req, argv[1]);
	if(strcmp(req, "get") && strcmp(req, "fsz") && strcmp(req, "adr"))
	{
		printf("malformed command\n");
		exit(0);
	}
	
	if(!strcmp(req, "get"))
	{
		msgType = GET_REQ;
	}
	else if(!strcmp(req, "fsz"))
	{
		msgType = FSZ_REQ;
	}
	else if(!strcmp(req, "adr"))
	{
		msgType = ADR_REQ;
	}
	
	while (1)
	{
		retVal = getopt_long(argc-2, &argv[1], "d:o:m", long_options, &option_index);

		if (retVal == -1)
		{
			break;
		}

		switch (retVal)
		{//different cases for different command line arguments
			case 'd':
				strcpy((char *)delay_char, optarg);
				if(isNum(delay_char))
				{
					delay = atoi(delay_char);
				}
				else
				{
					printf("malformed command\n");
					exit(0);
				}
				break;

			case 'o':
				strcpy((char *)offset_char, optarg);
				if(isNum(offset_char))
				{
					offset = atoi(offset_char);
				}
				else
				{
					printf("malformed command\n");
					exit(0);
				}
				break;

			case 'm':
				print_on = 1;
				break;

			default:
				printf("malformed command\n");
				exit(0);
		}
	}

	strcpy((char *)host, argv[argc-2]);
	if(checkHost(host, host_ip, &port))
	{
		;
	}
	else
	{
		printf("malformed command\n");
		exit(0);
	}

	strcpy(str, argv[argc-1]);
	char prxy_ip[100];
	int prxy_port;
	//host is checked
	if(checkHost(str, prxy_ip, &prxy_port))
	{
		printf("malformed command\n");
		exit(0);
	}
	
	if(!strcmp(str, req) || !strcmp(str, host) || !strcmp(str, delay_char) || !strcmp(str, offset_char))
	{
		printf("malformed command\n");
		exit(0);
	}

	if(port < 0)
	{
		printf("malformed command.\n");
		exit(0);
	}

        struct hostent *hostip;
	//getting server ip address using gethostbyname()
        hostip = gethostbyname(host);

        char host_ip2[16];
        if(hostip != NULL)
        {
                strcpy(host_ip2, inet_ntoa(*((struct in_addr *)hostip->h_addr_list[0])));
        }

	/*main logic start here....*/

	sock_info client_sock;
	client_sock.port = port;
	if(create_client_sock(&client_sock, host) != 1)
	{
		printf("ERROR: Socket Creation: %s\n", strerror(errno));
		exit(0);
	}

	
        Request *request, *response;
        char *buffer;
        int len = 0;
	char addr[16];
	memset(addr, '\0', 16);

	/*sending request to server*/
        request = alloc_request(strlen(str));
        request->msgType = msgType;
	if(msgType == (TWO_BYTE)ADR_REQ)
	{
		request->offset = 0;
	}
	else if(msgType == (TWO_BYTE)FSZ_REQ)
	{
		request->offset = 0;
	}
	else if(msgType == (TWO_BYTE)GET_REQ)
	{
		if(offset < 0)
		{
			offset = 0;
		}
		request->offset = offset;
	}
	request->serverDelay = delay;
        request->dataLength = strlen(str);
        memcpy(request->data, str, request->dataLength);
	//packing request for sending it to server
        PACK_REQUEST(request, &buffer, &len);
	//sending request to server
	send_sock(&client_sock, buffer, len);
	DEALLOC_PACKING_BUFFER(buffer);
        dealloc_request(request);

	/*receiving response from server*/
        if(recv_sock(&client_sock, &buffer, &len) == -1)
	{
		printf("Server is down.\n");
		exit(0);
	}

        response = alloc_request(0);
	//unpacking request from server
        UNPACK_REQUEST(response, buffer, &len);
	if(print_on == 1)
	{
		printf("\tReceived %d bytes from %s.\n", len, host_ip2);
		printf("\t  MessageType: 0x%hx\n", response->msgType);
		printf("\t       Offset: 0x%08x\n", response->offset);
		printf("\t  ServerDelay: 0x%02x\n", 0);
		printf("\t   DataLength: 0x%08x\n", response->dataLength);
	}

	/*printing response from server in accordance with response types*/
	if(response->msgType == (TWO_BYTE)ADR_RPLY)
	{
		response->data[response->dataLength] = '\0';
		printf("\tADDR = %s\n", response->data);//address is printed
	}
	else if(response->msgType == (TWO_BYTE)ADR_FAIL)
	{
		printf("\tADDR request for \'%s\' failed.\n", str);//address request faliure
	}
	else if(response->msgType == (TWO_BYTE)FSZ_RPLY)
	{
		response->data[response->dataLength] = '\0';
		printf("\tFILESIZE = %d\n", *((int *)response->data));//file size is printed
	}
	else if(response->msgType == (TWO_BYTE)FSZ_FAIL)
	{
		printf("\tFILESIZE request for \'%s\' failed.\n", str);//file size failure
	}
	else if(response->msgType == (TWO_BYTE)GET_RPLY)
	{
		printf("\tFILESIZE = %d, MD5 = ", response->dataLength);
		int index=0;
		for(index=0; index<16; index++)
		{
			printf("%02x",client_sock.checksum[index]);//checksum is printed
		}
		printf("\n");
	}
	else if(response->msgType == (TWO_BYTE)GET_FAIL)
	{
		printf("\tGET request for \'%s\' failed.\n", str);//get request failure
	}
	else if(response->msgType == (TWO_BYTE)ALL_FAIL)
	{
		printf("\tALL FAIL is received.\n");//all fail failure
	}
	else
	{
		printf("\tResponse not determined.\n");//response not determined
	}
	
        dealloc_recv_buffer(buffer);
        dealloc_request(response);
}
