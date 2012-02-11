#include "queue.h"
#include "time_551.h"
#include <math.h>
#include <signal.h>

#define ACTUAL_TIME
#define Customer Element
#define round(X) (((X)>= 0)?(int)((X)+0.5):(int)((X)-0.5))

//global variables declaration
//used to capture command line argument
char dist[10];
int expo;
char tsfile[100];
char is_tsfile_given=0;
char num_server;
float lambda;
float mu;
long int seed;
int size;
int num_cust;

//time and customer counting 
//related global variables 
//declared here.
int num_cust_arrived=0;
int num_cust_serviced=0;
int num_cust_dropped=0;
time_in_ms total_arrival_time;
time_in_ms total_service_time;
time_in_ms total_time_in_system;
time_in_ms total_time_in_system_sqr;

//other global variables
//for trace file and signal handling
float total_cust_with_time_at_s[2];
FILE *trace_file;
sigset_t set;
void *handler(void *arg);
void signal_handler(int sig);
struct sigaction act;
char keep_rounding = 1;

//queue and referential
//starting time is declared
Queue q;
struct timeval start_time;

void *creating_customer(void *arg);
void *treating_customer(void *arg);

//check whether a sring represents
//a float number or not. if  so, 
//fills the float variable pointer
int checkFloatChar(char *str, float *value)
{
	int i=0;
	for(i=0; i<strlen(str); i++)
	{
		if(str[i] != '.')
		{
			if((str[i] < '0' || str[i] > '9'))
			{
				return 0;
			}
		}
	}
	sscanf(str, "%f", value);
	return 1;
}

//check whether a sring represents
//a integerr or not. if  so, 
//fills the int variable pointer
int checkIntChar(char *str, int *value)
{
	int i=0;
	for(i=0; i<strlen(str); i++)
	{
		if(str[i] < '0' || str[i] > '9')
		{
			return 0;
		}
	}
	*value = atoi(str);
	return 1;
}

//check whether a sring represents
//a long integer or not. if  so, 
//fills the long int variable pointer
int checkLongChar(char *str, long int *value)
{
	int i=0;
	for(i=0; i<strlen(str); i++)
	{
		if(str[i] < '0' || str[i] > '9')
		{
			return 0;
		}
	}
	sscanf(str, "%ld", value);
	return 1;
}

//main mm2 program
int main(int argc, char **argv)
{
	int arg_length=1;
	char lambda_char[10];
	char mu_char[10];
	char seed_char[10];
	char size_char[10];
	char num_char[10];
	
//fills the default values
	num_server = 2;
	lambda = 0.5;
	mu = 0.35;
	seed = 0;
	size = 5;
	num_cust = 20;
	expo = 1;
	start_time.tv_sec = 0;
	start_time.tv_usec = 0;

//memsets the string variables
//that are going to capture 
//the command-line arguments
	memset(lambda_char, '\0', 10);
	memset(mu_char, '\0', 10);
	memset(seed_char, '\0', 10);
	memset(size_char, '\0', 10);
	memset(num_char, '\0', 10);
	memset(dist, '\0', 10);
	memset(tsfile, '\0', 100);

	strcpy(lambda_char, "0.5");
	strcpy(mu_char, "0.35");
	strcpy(dist, "exp");

	arg_length = 1;
//while for capturing 
//command line arguments
	while(arg_length < argc)
	{
		//capturing lambda from command line
		if(!strcmp(argv[arg_length], "-lambda"))
		{
			if(arg_length < (argc-1))
			{
				strcpy(lambda_char, argv[arg_length + 1]);
				if(checkFloatChar(lambda_char, &lambda))
				{
					arg_length += 2;
				}
				else
				{
					printf("Incorrect lambda value: %s\n", lambda_char);
					exit(0);
				}
			}
			else
			{
				printf("lambda not provided\n");
				exit(0);
			}
		}
		//capturing mu from command line
		else if(!strcmp(argv[arg_length], "-mu"))
		{
			if(arg_length < (argc-1))
			{
				strcpy(mu_char, argv[arg_length + 1]);
				if(checkFloatChar(mu_char, &mu))
				{
					arg_length += 2;
				}
				else
				{
					printf("Incorrect mu value: %s\n", mu_char);
					exit(0);
				}
			}
			else
			{
				printf("mu not provided\n");
				exit(0);
			}
		}
		//capturing seed from command line
		else if(!strcmp(argv[arg_length], "-seed"))
		{
			if(arg_length < (argc-1))
			{
				strcpy(seed_char, argv[arg_length + 1]);
				if(checkLongChar(seed_char, &seed))
				{
					arg_length += 2;
				}
				else
				{
					printf("Incorrect seed value: %s\n", seed_char);
					exit(0);
				}
			}
			else
			{
				printf("seed not provided\n");
				exit(0);
			}
		}
		//capturing size from command line
		else if(!strcmp(argv[arg_length], "-size"))
		{
			if(arg_length < (argc-1))
			{
				strcpy(size_char, argv[arg_length + 1]);
				if(checkIntChar(size_char, &size))
				{
					arg_length += 2;
				}
				else
				{
					printf("Incorrect size value: %s\n", size_char);
					exit(0);
				}
			}
			else
			{
				printf("size not provided\n");
				exit(0);
			}
		}
		//capturing n as number of customers from command line
		else if(!strcmp(argv[arg_length], "-n"))
		{
			if(arg_length < (argc-1))
			{
				strcpy(num_char, argv[arg_length + 1]);
				if(checkIntChar(num_char, &num_cust))
				{
					arg_length += 2;
				}
				else
				{
					printf("Incorrect num value: %s\n", num_char);
					exit(0);
				}
			}
			else
			{
				printf("num not provided\n");
				exit(0);
			}
		}
		//capturing s as if its a single server run or double server
		else if(!strcmp(argv[arg_length], "-s"))
		{
			if(arg_length < argc)
			{
				num_server = 1;
				arg_length += 1;
			}
			else
			{
				printf("not valid\n");
				exit(0);
			}
		}
		//capturing d as whether to use determistic or exponential values
		//from lambda and mu to calculate service and inter-arrival time
		else if(!strcmp(argv[arg_length], "-d"))
		{
			if(arg_length < (argc-1))
			{
				if(!strcmp(argv[arg_length + 1], "exp"))
				{
					expo = 1;
					strcpy(dist, argv[arg_length + 1]);
					arg_length += 2;
				}
				else if(!strcmp(argv[arg_length + 1], "det"))
				{
					expo = 0;
					strcpy(dist, argv[arg_length + 1]);
					arg_length += 2;
				}
				else
				{
					printf("Incorrect dist value: %s\n", argv[arg_length + 1]);
					exit(0);
				}
			}
			else
			{
				printf("num not provided\n");
				exit(0);
			}
		}
		//capturing tsfile from the command line
		else if(!strcmp(argv[arg_length], "-t"))
		{
			if(arg_length < (argc-1))
			{
					if(argv[arg_length + 1][0] != '-')
					{
						is_tsfile_given = 1;
						strcpy(tsfile, argv[arg_length + 1]);
						arg_length += 2;
					}
					else
					{
						if(
							!strcmp(argv[arg_length + 1], "-lambda") ||
							!strcmp(argv[arg_length + 1], "-seed") ||
							!strcmp(argv[arg_length + 1], "-size") ||
							!strcmp(argv[arg_length + 1], "-mu") ||
							!strcmp(argv[arg_length + 1], "-n") ||
							!strcmp(argv[arg_length + 1], "-t") ||
							!strcmp(argv[arg_length + 1], "-d")
						  )
						{
							printf("tsfile name is mixed with commandline argument option\n");
							exit(0);
						}
						else
						{
							is_tsfile_given = 1;
							strcpy(tsfile, argv[arg_length + 1]);
							arg_length += 2;
						}
					}
			}
			else
			{
				printf("tsfile name not provided\n");
				exit(0);
			}
		}
		//for any illegal input
		else
		{
				printf("illegal command line argument %s\n", argv[arg_length]);
				exit(0);
		}
	}


/*
 * command line arguments have been taken care and initialized.
 * now accordingly values are set
 * */
	printf("\nParameters:\n");
	if(is_tsfile_given == 0)
	{
		printf("    lambda = %s\n", lambda_char);
		printf("    mu = %s\n", mu_char);
	}

	if(num_server == 1)
		printf("    system = M/M/1\n");
	if(num_server == 2)
		printf("    system = M/M/2\n");

	if(is_tsfile_given == 0)
	{
		printf("    seed = %ld\n", seed);
	}

	printf("    size = %d\n", size);
	if(is_tsfile_given == 0)
	{
		printf("    number = %d\n", num_cust);
		printf("    distribution = %s\n", dist);
	}

	if(is_tsfile_given == 1)
	{
		trace_file = fopen(tsfile, "r");
		if(trace_file == NULL)
		{
			printf("ERROR: %s does not exist.\n", tsfile);
			exit(0);
		}
		char line[5];
		memset(line, '\0', 5);
		fgets ( line, 5, trace_file);
		if(fgets ( line, 5, trace_file) == NULL)
		{
			printf("ERROR: %s does not exist.\n", tsfile);
			exit(0);
		}

		sscanf(line, "%d", &num_cust);

		printf("    number = %d\n", num_cust);
		printf("    tsfile = %s\n", tsfile);
	}

	printf("\n");
//starting signal handler
	int i=0;
	for(i=0; i<num_server; i++)
	total_cust_with_time_at_s[i] = 0;

	pthread_t signal_thread;
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	pthread_sigmask(SIG_BLOCK, &set, NULL);
	pthread_create(&signal_thread, NULL, handler, NULL);
//initialize random number generator
	InitRandom(seed);
	q.total_number = num_cust;
	q.size = size;

	gettimeofday(&start_time, NULL);
//initializing variables for over-all statistics
	total_arrival_time.part_1 = 0;
	total_arrival_time.part_2 = 0;
	total_service_time.part_1 = 0;
	total_service_time.part_2 = 0;
	total_time_in_system.part_1 = 0;
	total_time_in_system.part_2 = 0;
	total_time_in_system_sqr.part_1 = 0;
	total_time_in_system_sqr.part_2 = 0;
//initializing queue
	queueInitialize(&q);

	printf("%08ld.%03ldms: emulation begins\n", 0, 0);
//creating customer thread
	pthread_t creating_customer_thread;
	pthread_create(&creating_customer_thread, NULL, creating_customer, (void *)NULL);
//creating customer treating server thread
	pthread_t treating_customer_thread[2];
	for(i=0; i<num_server; i++)
	{
		pthread_create(&treating_customer_thread[i], NULL, treating_customer, (void *)(i+1));
	}

//calling join on threads
	pthread_join( creating_customer_thread, NULL);
	for(i=0; i<num_server; i++)
	{
		pthread_join( treating_customer_thread[i], NULL);
	}

	pthread_join( signal_thread, NULL);

//calulating statistics 
//after either program completion or
//graceful shutdown of program. 
//that is no thread is running at this time.
        float ave_arrival_time = (float)((float)total_arrival_time.part_1 + ((float)total_arrival_time.part_2 / 1000))/num_cust_arrived;
        float ave_service_time = (float)((float)total_service_time.part_1 + ((float)total_service_time.part_2 / 1000))/num_cust_serviced;
        float ave_time_in_system = (float)((float)total_time_in_system.part_1 + ((float)total_time_in_system.part_2 / 1000))/num_cust_serviced;
        float ave_time_in_system_sqr = (float)((float)total_time_in_system_sqr.part_1 + ((float)total_time_in_system_sqr.part_2 / 1000))/num_cust_serviced;
	float variance = ave_time_in_system_sqr - (ave_time_in_system * ave_time_in_system);
	if(variance < 0.0f)
	{
		variance = (-1) * variance;
	}
	float std_dev;
	std_dev = sqrt(variance);
        float drop_prob = (float)num_cust_dropped / num_cust_arrived;

	time_in_ms now = get_current_time();
        float ave_cust_in_q = (float)q.total_cust_with_time / ((((float)now.part_1 + (float)now.part_2 / 1000)));

        float ave_cust_in_s[2];

	for(i=0; i<num_server; i++)
        ave_cust_in_s[i] = (float)total_cust_with_time_at_s[i] / ((((float)now.part_1 + (float)now.part_2 / 1000))/1000);
//printng statistics.
	printf("\nStatistics:\n\n");
	printf("    average inter-arrival time = %.6f\n", ave_arrival_time);
	printf("    average service time = %.6f\n", ave_service_time);
	printf("\n");
	printf("    average number of customers in Q1 = %.6f\n", ave_cust_in_q);
	for(i=0; i<num_server; i++)
	printf("    average number of customers at S%d = %.6f\n", i+1, ave_cust_in_s[i]);
	printf("\n");
	printf("    average time spent in system = %.6f\n", ave_time_in_system);
	printf("    standard deviation for time spent in system = %.6f\n", std_dev);
	printf("\n");
	printf("    customer drop probability = %.6f\n", drop_prob);
}

//signal handler is defined here
void *handler(void *arg)
{
        act.sa_handler = signal_handler;
        sigaction(SIGINT, &act, NULL);
        pthread_sigmask(SIG_UNBLOCK, &set, NULL);
	while(keep_rounding == 1)
	{
        	usleep(1000); /* give user time to hit CTRL-C */
	}
}

//handler call this handle function
void signal_handler(int sig)
{
	keep_rounding = 0;
	queueLock(&q);
	while(q.count != 0)
	{
		Customer *cust;
		queueEmpty(&q, &cust);
	}
	q.total_tried = q.total_number;
	q.count = 0;
	queueBroadcast(&q);
	queueUnlock(&q);
}

//main thread for customer treating server
void *treating_customer(void *arg)
{
	int i=1;
	int server_num = (int)arg;
	long int wait = 0;
	time_in_ms inter_arrival;
	inter_arrival.part_1 = 0;
	inter_arrival.part_2 = 0;
	
	while(1)//while for treating customers
	{
		Customer *cust;
		if( queueRemoveFromHead(&q, &cust)== 2)//remove element from queue
		break;

		if(cust != NULL)
		{//calculating various time in time_in_ms struct
			cust->time_begun_service = get_current_time();
			printf("%08ld.%03dms: c%d begin service as S%d\n", cust->time_begun_service.part_1, cust->time_begun_service.part_2, cust->value, server_num);
			//calculating waiting service time
			wait = cust->service_time.part_1*1000 + cust->service_time.part_2;
			usleep(wait);

#ifdef ACTUAL_TIME
			cust->time_departed = get_current_time();
#else
			cust->time_departed = time_plus(cust->time_begun_service, get_time_us(wait));
#endif

			inter_arrival = time_minus(cust->time_departed, cust->time_begun_service);
			//calculating time spent in system
			cust->time_in_system = time_minus(cust->time_departed, cust->time_arrived);

			printf("%08ld.%03dms: c%d departs from S%d, service time = %ld.%03dms\n", cust->time_departed.part_1, cust->time_departed.part_2, cust->value, server_num, inter_arrival.part_1, inter_arrival.part_2);
			printf("                time spent in system = %ld.%03dms\n", cust->time_in_system.part_1, cust->time_in_system.part_2);
			//calculating service time
			total_service_time = time_plus(inter_arrival, total_service_time);	
			total_time_in_system = time_plus(cust->time_in_system, total_time_in_system);	
			total_time_in_system_sqr = time_plus_sqr(cust->time_in_system, total_time_in_system_sqr);	
			total_cust_with_time_at_s[server_num-1] = (float)(((float)(((float)inter_arrival.part_1 + (float)inter_arrival.part_2 / 1000))/1000)) + (float)total_cust_with_time_at_s[server_num-1];
			//deallocating the customer element
			deallocateElement(cust);
			num_cust_serviced++;
		}
	}
	keep_rounding = 0;
}

//main thread function for creating customers
void *creating_customer(void *arg)
{
	int i=1;
	long int wait = 0;
	long int actual_wait = 0;
	long int service = 0;
	char line[100];
	int ret=0;
	time_in_ms inter_arrival;
	time_in_ms last_arrived_time;
	time_in_ms arrived_time;
	time_in_ms arrived;
	time_in_ms time_diff;
	last_arrived_time.part_1 = 0;
	last_arrived_time.part_2 = 0;
	inter_arrival.part_1 = 0;
	inter_arrival.part_2 = 0;
	
	while(i <= num_cust)//while loop for creating customers
	{
		//getting inter-arrival and service time
		//for the customer
		if(is_tsfile_given)
		{
			memset(line, '\0', 100);
			if(fgets ( line, 100, trace_file) == NULL)
			{
				printf("\n\nERROR: reading file\n\n");
				exit(0);
			}
			sscanf(line, "%ld %ld", &wait, &service);
		}
		else
		{
			wait = GetInterval(expo, lambda);
			service = GetInterval(expo, mu);
		}
		//calculating different times
		//getting arrived time
		arrived_time = get_current_time();

		//calculating actual waiting
		//time for making inter-arrival
		//distinction
#if 10
		time_diff = time_minus(arrived_time, last_arrived_time);		
		actual_wait = (wait*1000) - ((time_diff.part_1*1000) + time_diff.part_2);
		if(actual_wait < 0)
		{
			actual_wait = 0;
		}
		usleep(actual_wait);
#else
		usleep(wait*1000);
#endif

	
#ifdef ACTUAL_TIME
		arrived = get_current_time();
		Customer *cust=allocateElement();
		cust->value = i;
		cust->time_arrived = arrived;
		cust->service_time = get_time(service);
		inter_arrival = time_minus(cust->time_arrived, inter_arrival);
#else
		Customer *cust=allocateElement();
		cust->value = i;
		cust->time_arrived = time_plus( arrived_time, get_time_us(actual_wait));
		cust->service_time = get_time(service);
		inter_arrival = get_time(wait);
#endif
		//printing inter-arrival time
		printf("%08ld.%03dms: c%d arrives inter-arrival time = %ld.%03dms\n", cust->time_arrived.part_1, cust->time_arrived.part_2, i, inter_arrival.part_1, inter_arrival.part_2);
		total_arrival_time = time_plus(inter_arrival, total_arrival_time);			

		last_arrived_time = cust->time_arrived;
		inter_arrival = cust->time_arrived;
	
		num_cust_arrived++;
		//adding customer to the tail of Q1
		ret = queueAddToTail(&q, cust);

		if(ret == 2)
		{
			num_cust_dropped++;
		}
	
		if(ret == 3)
		{
			break;
		}

		i++;
	}
	keep_rounding = 0;
}
