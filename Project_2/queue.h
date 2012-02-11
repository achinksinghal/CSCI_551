#ifndef __PROJ_2_QUEUE_H__
#define __PROJ_2_QUEUE_H__
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
#include <sys/time.h>
#include "time_551.h"

//structure of queue's 
//element
typedef struct _Element
{
	_Element *next;//pointer to next element
	int value;//its value
	time_in_ms time_arrived;//time when it arrived
	time_in_ms time_entered_q;//time when it arrived in the q
	time_in_ms time_left_q;//time when it left q
	time_in_ms time_begun_service;//time when begun service
	time_in_ms time_departed;//time when time system
	time_in_ms time_in_system;//total time spent in system
	time_in_ms service_time;//its service time
}
Element;

//structure of queue Q1
typedef struct _Queue
{
	Element *head;//head of q
	Element *tail;//tail of q
	int count;//total number of element present in q
	int size;//size of that is to be maintained
	int total_added;//total element added in q
	int total_tried;//total element tried in q
	int total_removed;//total element removed from 1
	int total_number;//total number of element
	float total_cust_with_time;//total cust time spent in q
	time_in_ms time_q_used;//total time q used
	pthread_mutex_t mutex;//mutex of q
	pthread_cond_t cond;//cv of q
}Queue;

//various q functions defined in 
//queue.cc
int queueInitialize(Queue *q);
int queueAddToTail(Queue *q, Element *element);
int queueRemoveFromHead(Queue *q, Element **element);
int queueEmpty(Queue *q, Element **element);
int queueGetHead(Queue *q, Element *element);
int queueGetTail(Queue *q, Element *element);
int queueElementCount(Queue *q);
int queueLock(Queue *q);
int queueUnlock(Queue *q);
int queueWait(Queue *q);
int queueSignal(Queue *q);
int queueBroadcast(Queue *q);
int printQueue(Queue *q);
Element* allocateElement();
void deallocateElement(Element *ele);
int queueFree(Queue *q);
int queue_main();
#endif /*__PROJ_2_QUEUE_H__*/
