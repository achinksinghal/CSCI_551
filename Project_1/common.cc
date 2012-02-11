#include "common.h"

//pack the one byte data into the buffer
void PACK_1_BYTE(ONE_BYTE **buff, ONE_BYTE val)
{
	(*buff)[0] = val;
	*buff = *buff + 1;
}

//pack the two byte data into the buffer
void PACK_2_BYTE(ONE_BYTE **buff, TWO_BYTE val)
{
	TWO_BYTE abc = htons(val);
	ONE_BYTE *a = (ONE_BYTE *)&abc;
	(*buff)[0] = a[0];
	(*buff)[1] = a[1];
	*buff = *buff + 2;
}

//pack the four byte data into the buffer
void PACK_4_BYTE(ONE_BYTE **buff, FOUR_BYTE val)
{
	FOUR_BYTE abc = htonl(val);
	ONE_BYTE *a = (ONE_BYTE *)&abc;
	(*buff)[0] = a[0];
	(*buff)[1] = a[1];
	(*buff)[2] = a[2];
	(*buff)[3] = a[3];
	*buff = *buff + 4;
}

//unpack the one byte data from the buffer and give it as value
void UNPACK_1_BYTE(ONE_BYTE **buff, ONE_BYTE *val)
{
	*val = (*buff)[0];
	*buff = *buff + 1;
}

//unpack the two byte data from the buffer and give it as value
void UNPACK_2_BYTE(ONE_BYTE **buff, TWO_BYTE *val)
{
	TWO_BYTE abc;
	ONE_BYTE *a = (ONE_BYTE *)&abc;
	a[0] = (*buff)[0];
	a[1] = (*buff)[1];
	*val = ntohs(abc);
	*buff = *buff + 2;
}

//unpack the four byte data from the buffer and give it as value
void UNPACK_4_BYTE(ONE_BYTE **buff, FOUR_BYTE *val)
{
	FOUR_BYTE abc;
	ONE_BYTE *a = (ONE_BYTE *)&abc;
	a[0] = (*buff)[0];
	a[1] = (*buff)[1];
	a[2] = (*buff)[2];
	a[3] = (*buff)[3];
	*val = ntohl(abc);
	*buff = *buff + 4;
}

//pack the complete request into the buffer
int PACK_REQUEST(Request *req, ONE_BYTE **buff, FOUR_BYTE *len)
{
	int i=0;

	if(buff == NULL)
		return 0;

	*len = HEADER_LEN + req->dataLength;

	*buff = (ONE_BYTE *)malloc((*len) * sizeof(ONE_BYTE));

	ONE_BYTE *tempBuff = *buff;
	if(*buff == NULL)
		return 0;
	
	PACK_2_BYTE(buff, req->msgType);
	PACK_4_BYTE(buff, req->offset);
	PACK_1_BYTE(buff, req->serverDelay);
	PACK_4_BYTE(buff, req->dataLength);

	if(req->dataLength > 0)
	{
		if(req->data != NULL)
		{
			for(i=0; i<req->dataLength; i++)
			{
				PACK_1_BYTE(buff, req->data[i]);
			}
		}
		else
			return 0;
	}

	*len = *buff - tempBuff;
	*buff = tempBuff;
	return 1;
}

//dellocates the packing buffer
int DEALLOC_PACKING_BUFFER(ONE_BYTE *buffer)
{
	free(buffer);
	return 1;
}

//unpacks the request from the buffer to request variable
int UNPACK_REQUEST(Request *req, ONE_BYTE *buff, FOUR_BYTE *len)
{
	int i=0;
	ONE_BYTE *tempBuff = buff;
	if(buff == NULL)
		return 0;

	UNPACK_2_BYTE(&buff, &req->msgType);
	UNPACK_4_BYTE(&buff, &req->offset);
	UNPACK_1_BYTE(&buff, &req->serverDelay);
	UNPACK_4_BYTE(&buff, &req->dataLength);
	
	if(req->msgType != (TWO_BYTE)GET_RPLY)
	{
		if(req->dataLength > 0)
		{
			req->data = (char *)malloc((req->dataLength + 1)* sizeof(ONE_BYTE));
		}

		if(req->data != NULL)
		{
			for(i=0; i<req->dataLength; i++)
			{
				UNPACK_1_BYTE(&buff, &req->data[i]);
			}
		}
		else
			return 0;
	}
	*len = buff - tempBuff;
	return 1;
}

//extract the data length from the buffer
int EXTRACT_DATALENGTH_FROM_BUFFER(ONE_BYTE *buff, FOUR_BYTE *dataLength, TWO_BYTE *msgType)
{
	if(buff == NULL)
		return 0;

	Request req;

	UNPACK_2_BYTE(&buff, &req.msgType);
	UNPACK_4_BYTE(&buff, &req.offset);
	UNPACK_1_BYTE(&buff, &req.serverDelay);
	UNPACK_4_BYTE(&buff, &req.dataLength);
	*dataLength = req.dataLength;
	*msgType = (TWO_BYTE)req.msgType;

	return 1;
}

//prints the request structure
void PRINT_REQUEST(Request *req)
{
	int i = 0;
	printf("    msgType=0x%hx\n", req->msgType);fflush(stdout);
	printf("     offset=%d\n", req->offset);fflush(stdout);
	printf("serverDelay=%d\n", req->serverDelay);fflush(stdout);
	printf(" dataLength=%d\n", req->dataLength);fflush(stdout);
	if(req->msgType == (TWO_BYTE)FSZ_RPLY)
	{
		printf("   filesize=%d\n", *((int *)req->data));fflush(stdout);
	}
	if(req->msgType == (TWO_BYTE)GET_RPLY)
	{
#ifdef DEBUG		
		printf("\ndata is ---");
		for( i=0; i<req->dataLength; i++ )
		{
			printf("%c", req->data[i]);fflush(stdout);
		}
		printf("--- data ends\n");
#endif	
	}
	else
	{
		for( i=0; i<req->dataLength; i++ )
		{
			printf("    data[%d]=%c\n", i, req->data[i]);fflush(stdout);
		}
	}
	fflush(stdout);
	printf("\n");
}

//allocate request structure with the specified buffer size
Request* alloc_request(int buffer_size)
{
	Request *req = new Request;
	req->msgType = -1;
	req->offset = -1;
	req->serverDelay = -1;
	if(buffer_size > 0)
	{
		req->dataLength = buffer_size;
	}
	else
	{
		req->dataLength = 0;
	}

	if(buffer_size > 0)
	{
		req->data = new ONE_BYTE[buffer_size];
	}
	else
	{
		req->data = NULL;
	}
	return req;
} 

//deallocate request structure
void dealloc_request(Request *req)
{
	if(req->data != NULL)
	{
		free(req->data);
	}
	
	free(req);
} 


//function made to test the functioning of packing and unpacking
int common_main()
{
	Request *req, *res;
	char *buffer;
	int len = 0, i=0;

	req = alloc_request(15);
	req->msgType = 2;
	req->offset = 14;
	req->serverDelay = 21;
	req->dataLength = 15;
	strncpy(req->data, "YOGESHCHHAYASINGHAL", 15);

	PRINT_REQUEST(req);

	PACK_REQUEST(req, &buffer, &len);
	printf("Length=%d \n\n",len);
	for(i=0; i<len; i++)
	{printf("buffer[%d]=%c\n\n",i, buffer[i]);fflush(stdout);}

	res = alloc_request(0);
	UNPACK_REQUEST(res, buffer, &len);
	printf("Length=%d\n\n",len);
	PRINT_REQUEST(res);
	DEALLOC_PACKING_BUFFER(buffer);	
	dealloc_request(req);
	dealloc_request(res);
	return 1;
}
