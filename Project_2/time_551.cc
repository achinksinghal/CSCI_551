#include "queue.h"
#include <math.h>
#include <signal.h>
extern struct timeval start_time;


/*
 * Begin code I did not write.
 * The code is downloaded from http://merlot.usc.edu/cs551-s12/projects/warmup2.html 
 * If the source code requires you to include copyright, put copyright here.
 * */

//this function returns an exponential interval
long ExponentialInterval(float dval, float rate)
{
	float ret;
	rate = (float)rate * (-1) ;
	ret = log(1-dval)/rate;
	ret = ret*1000;
	return (long)ret;
}

//for initializing random function
void InitRandom(long l_seed)
{
	if (l_seed == 0L) 
	{
		time_t localtime=(time_t)0;
		time(&localtime);
		srand48((long)localtime);
	} 
	else 
	{
		srand48(l_seed);
	}
}

//getting random or deterministic interval,
//depending on the value of exponential
int GetInterval(int exponential, double rate)
{
	int ret=0;
	if (exponential == 1) 
	{
		double dval=(double)drand48();
		ret = ExponentialInterval(dval, rate);
	} 
	else 
	{
		double millisecond=((double)1000)/rate;
		ret = round(millisecond);
	}
	if(ret < 1)
	ret = 1;
	if(ret > 10000)
	ret = 10000;

	return ret;
}

/*
 * End code I did not write.
 * */
//converts timeval struct values
//into time_in_ms.
time_in_ms convert_into_ms(struct timeval curr)
{
	time_in_ms ret;

	ret.part_1 = curr.tv_sec*1000 + curr.tv_usec/1000 ;
	ret.part_2 = curr.tv_usec - (curr.tv_usec/1000)*1000;
	return ret;
}

//substract time, and return it into same
//time_in_ms struct
time_in_ms time_minus(time_in_ms a, time_in_ms b)
{
	double a_a;
	double b_b;
	double c_c;
	time_in_ms c;

	c.part_1 = a.part_1 - b.part_1;
	c.part_2 = a.part_2 - b.part_2;
	if(c.part_2 < 0)
	{
		c.part_1--;
		c.part_2 = 1000 + c.part_2;
	}

	return c; 
}

//adds the time and returns 
//into same time_in_ms struct
time_in_ms time_plus(time_in_ms a, time_in_ms b)
{
	double a_a;
	double b_b;
	double c_c;
	time_in_ms c;

	c.part_1 = a.part_1 + b.part_1;
	c.part_2 = a.part_2 + b.part_2;
	if(c.part_2 > 999)
	{
		c.part_1++;
		c.part_2 = c.part_2 - 1000;
	}

	return c; 
}

//square the time taken in time_in_ms struct
time_in_ms time_square(time_in_ms a)
{
	time_in_ms c;

	float a_a = a.part_1 + (float)((float)a.part_2 / 1000);
	a_a = a_a * a_a;

	c.part_1 = (long)a_a;
	c.part_2 = (a_a - (float)c.part_1)*1000;
	
	return c; 
}

//square the time a and then adds 
//into b and returns the value
time_in_ms time_plus_sqr(time_in_ms a, time_in_ms b)
{
	time_in_ms c = time_plus(time_square(a), b);
	return c;
}

//convert the time given in ms
//into time_in_ms struct variable
time_in_ms get_time(long a)
{
	time_in_ms c;
	c.part_1 = a;
	c.part_2 = 0L;
	return c; 
}

//convert the time given in
//mico second into time_in_ms
//struct variable.
time_in_ms get_time_us(long a)
{
	time_in_ms c;
	c.part_1 = a/1000;
	c.part_2 = a%1000;
	return c; 
}

//gives the current time with respect
//to certain values. time which is returned
//is in time_in_ms struct. 
time_in_ms get_current_time()
{
	struct timeval curr_time;
	struct timeval rel_curr_time;
	time_in_ms current_time; 

	gettimeofday(&curr_time, NULL);

	rel_curr_time.tv_usec = curr_time.tv_usec - start_time.tv_usec;
	rel_curr_time.tv_sec = curr_time.tv_sec - start_time.tv_sec;

	if(rel_curr_time.tv_usec < 0 )
	{
		rel_curr_time.tv_sec--;
		rel_curr_time.tv_usec = 1000000 + rel_curr_time.tv_usec ;
	}

	current_time = convert_into_ms(rel_curr_time);

	return current_time; 
}
