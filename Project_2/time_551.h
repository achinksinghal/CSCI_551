#ifndef __PROJ_2_TIME_551_H__
#define __PROJ_2_TIME_551_H__
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

//structure of time
typedef struct _time_in_ms
{
	long part_1;//defines the left part before decimal
	long part_2;//defines the right part after decimal
}time_in_ms;

//various functions for 
//getting random or deterministic time
//interval.
long ExponentialInterval(float dval, float rate);
void InitRandom(long l_seed);
int GetInterval(int exponential, double rate);

//various functions for time
//addition and substractions
time_in_ms convert_into_ms(struct timeval curr);
time_in_ms time_minus(time_in_ms a, time_in_ms b);
time_in_ms time_plus(time_in_ms a, time_in_ms b);
time_in_ms time_square(time_in_ms a);
time_in_ms time_plus_sqr(time_in_ms a, time_in_ms b);
time_in_ms get_time(long a);
time_in_ms get_time_us(long a);
time_in_ms get_current_time();

#endif /*__PROJ_2_TIME_551_H__*/
