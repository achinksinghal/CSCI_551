#ifndef __PROJ_1_COMMON_H__
#define __PROJ_1_COMMON_H__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h> 
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <sys/wait.h> 
#include <getopt.h>
#include <pthread.h>

// data types that are used
#define TWO_BYTE short int
#define ONE_BYTE char
#define FOUR_BYTE int
#define HEADER_LEN 11
#define MAX_SIZE 100

//requests types while sending requests and receiving response a client side
#define ADR_REQ  0xFE10
#define FSZ_REQ  0xFE20
#define GET_REQ  0xFE30

#define ADR_RPLY 0xFE11
#define FSZ_RPLY 0xFE21
#define GET_RPLY 0xFE31

#define ADR_FAIL 0xFE12
#define FSZ_FAIL 0xFE22
#define GET_FAIL 0xFE32

#define ALL_FAIL 0xFCFE

//request structure of request that is sent to server and server also sends response accordingly
typedef struct _Request
{
   TWO_BYTE msgType;
   FOUR_BYTE offset;
   ONE_BYTE serverDelay;
   FOUR_BYTE dataLength;
   ONE_BYTE *data;
}Request;


void PACK_1_BYTE(ONE_BYTE **buff, ONE_BYTE val);
void PACK_2_BYTE(ONE_BYTE **buff, TWO_BYTE val);
void PACK_4_BYTE(ONE_BYTE **buff, FOUR_BYTE val);
void UNPACK_1_BYTE(ONE_BYTE **buff, ONE_BYTE *val);
void UNPACK_2_BYTE(ONE_BYTE **buff, TWO_BYTE *val);
void UNPACK_4_BYTE(ONE_BYTE **buff, FOUR_BYTE *val);
int PACK_REQUEST(Request *req, ONE_BYTE **buff, FOUR_BYTE *len);
int DEALLOC_PACKING_BUFFER(ONE_BYTE *buff);
int UNPACK_REQUEST(Request *req, ONE_BYTE *buff, FOUR_BYTE *len);
void PRINT_REQUEST(Request *req);
int EXTRACT_DATALENGTH_FROM_BUFFER(ONE_BYTE *buff, FOUR_BYTE *dataLength, TWO_BYTE *msgType);
Request* alloc_request(int buffer_size);
void dealloc_request(Request *req);

#endif /*__PROJ_1_COMMON_H__*/
