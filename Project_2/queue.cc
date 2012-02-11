#include "queue.h"

//#define AVE_Q1
time_in_ms get_current_time();
time_in_ms time_minus(time_in_ms a, time_in_ms b);
time_in_ms time_plus(time_in_ms a, time_in_ms b);
extern int num_cust;
extern int num_cust_served;

//this initializes the queue
//which we meant in project as 
//Q1.
int queueInitialize(Queue *q)
{
	if(q == NULL)
	{
		return 0;
	}
	
	q->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	q->cond = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
	pthread_mutex_lock(&q->mutex);
	q->head = NULL;
	q->tail = NULL;
	q->count = 0;
	q->total_added = 0;
	q->total_removed = 0;
	q->total_tried = 0;
	q->total_cust_with_time = 0;
	q->time_q_used.part_1 = 0;
	q->time_q_used.part_2 = 0;
	pthread_mutex_unlock(&q->mutex);
	return 1;
}

//adds an element at the 
//tail of the queue.
//first competes for
//lock to enter into queue. after 
//entering into the queue, releases the
//locks.
//
//prints the time of entering in the queue.
//element is dropped if queue size is > certain value.
int queueAddToTail(Queue *q, Element *ele)
{
	if(q == NULL || ele == NULL)
	{
		return 0;
	}
	
	pthread_mutex_lock(&q->mutex);

	if(q->total_tried >= q->total_number)
	{
		ele->time_entered_q = get_current_time();
		printf("%08ld.%03dms: c%d dropped\n", ele->time_entered_q.part_1, ele->time_entered_q.part_2, ele->value);
		pthread_mutex_unlock(&q->mutex);
		return 3;
	}
	q->total_tried++;

	if(q->count >= q->size)
	{
		ele->time_entered_q = get_current_time();
		printf("%08ld.%03dms: c%d dropped\n", ele->time_entered_q.part_1, ele->time_entered_q.part_2, ele->value);
		pthread_mutex_unlock(&q->mutex);
		return 2;
	}

	if(q->count == 0)
	{
		q->head = ele;
		q->tail = ele;
		ele->next = NULL;
	}
	else
	{
		q->tail->next = ele;
		q->tail = ele;
		ele->next = NULL;
	}
	ele->time_entered_q = get_current_time();
	printf("%08ld.%03dms: c%d enters Q1\n", ele->time_entered_q.part_1, ele->time_entered_q.part_2, ele->value);

	if(q->count == 0)
	{
		pthread_cond_broadcast(&q->cond);
	}



#ifdef AVE_Q1
	time_in_ms now = ele->time_entered_q;
	q->time_q_used = time_minus(now, q->time_q_used);
	q->total_cust_with_time = (float)(((float)(((float)q->time_q_used.part_1 + (float)q->time_q_used.part_2 / 1000))/1000) * q->count) + (float)q->total_cust_with_time;
#else

#endif


	q->count++;
	q->total_added++;
	pthread_mutex_unlock(&q->mutex);
	return 1;
}

//removes an element from the head of the queue.
//first grabs the lock, removes the element, then releases 
//the lock. prints queue leaving time. 
int queueRemoveFromHead(Queue *q, Element **ele)
{
	if(q == NULL || ele == NULL)
	{
		return 0;
	}
	
	pthread_mutex_lock(&q->mutex);

	if(q->total_tried >= q->total_number && q->count == 0)
	{
		*ele = NULL;
		pthread_mutex_unlock(&q->mutex);
		return 2;
	}

	if(q->count == 0)
	{
		pthread_cond_wait(&q->cond, &q->mutex);
	}

	if(q->total_tried >= q->total_number && q->count == 0)
	{
		*ele = NULL;
		pthread_mutex_unlock(&q->mutex);
		return 2;
	}

	if(q->count == 0)
	{
		*ele = NULL;
		pthread_mutex_unlock(&q->mutex);
		return 0;
	}
	else
	{
		*ele = q->head;
		q->head = q->head->next;
		(*ele)->next = NULL;
	}

	(*ele)->time_left_q = get_current_time();
	time_in_ms queue_time = time_minus((*ele)->time_left_q, (*ele)->time_entered_q);
	printf("%08ld.%03dms: c%d leaves Q1, time in Q1 = %ld.%03dms\n", (*ele)->time_left_q.part_1, (*ele)->time_left_q.part_2, (*ele)->value, queue_time.part_1, queue_time.part_2);

#ifdef AVE_Q1
	time_in_ms now = get_current_time();
	q->time_q_used = time_minus(now, q->time_q_used);
	q->total_cust_with_time = (float)(((float)(((float)q->time_q_used.part_1 + (float)q->time_q_used.part_2 / 1000))/1000) * q->count) + (float)q->total_cust_with_time;
#else
	q->total_cust_with_time = (float)((float)((float)queue_time.part_1 + (float)queue_time.part_2 / 1000)) + (float)q->total_cust_with_time;
#endif
	q->count--;
	q->total_removed++;
	if(q->count == 0)
	{
		q->head = NULL;
		q->tail = NULL;
	}
	pthread_mutex_unlock(&q->mutex);
	return 1;
}

//empties the queue if it is desired 
//when ctrl + c is pressed.
//also prints the queue leaving time
int queueEmpty(Queue *q, Element **ele)
{
	if(q == NULL || ele == NULL)
	{
		return 0;
	}

	if(q->count != 0 && q->head != NULL)
	{
		*ele = q->head;
		q->head = q->head->next;
		(*ele)->next = NULL;
		(*ele)->time_left_q = get_current_time();

		time_in_ms queue_time = time_minus((*ele)->time_left_q, (*ele)->time_entered_q);
		printf("%08ld.%03dms: c%d leaves Q1, time in Q1 = %ld.%03dms\n", (*ele)->time_left_q.part_1, (*ele)->time_left_q.part_2, (*ele)->value, queue_time.part_1, queue_time.part_2);


#ifdef AVE_Q1
		time_in_ms now = get_current_time();
		q->time_q_used = time_minus(now, q->time_q_used);
		q->total_cust_with_time = (float)(((float)(((float)q->time_q_used.part_1 + (float)q->time_q_used.part_2 / 1000))/1000) * q->count) + (float)q->total_cust_with_time;
		time_in_ms now = get_current_time();
#else
		q->total_cust_with_time = (float)((float)((float)queue_time.part_1 + (float)queue_time.part_2 / 1000)) + (float)q->total_cust_with_time;
#endif
		q->count--;
		q->total_removed++;
	}
	else
	{
		*ele = NULL;
		return 0;
	}
	if(q->count == 0)
	{
		q->head = NULL;
		q->tail = NULL;
	}
	return 1;
}

//returns the head of the queue. but doesnot
//removes the element and also makes the next pointer 
//null in the returned pointer
int queueGetHead(Queue *q, Element *ele)
{
	if(q == NULL || ele == NULL)
	{
		return 0;
	}
	
	pthread_mutex_lock(&q->mutex);
	if(q->count==0)
	{
		pthread_mutex_unlock(&q->mutex);
		return 0;
	}
	else
	{
		*ele = *q->head;
		ele->next = NULL;
	}
	pthread_mutex_unlock(&q->mutex);
	return 1;
}

//returns the tail of the queue. but doesnot
//removes the element and also makes the next pointer 
//null in the returned pointer
int queueGetTail(Queue *q, Element *ele)
{
	if(q == NULL || ele == NULL)
	{
		return 0;
	}
	
	pthread_mutex_lock(&q->mutex);
	if(q->count==0)
	{
		pthread_mutex_unlock(&q->mutex);
		return 0;
	}
	else
	{
		*ele = *q->tail;
		ele->next = NULL;
	}
	pthread_mutex_unlock(&q->mutex);
	return 1;
}

//gives the element count of the queue.
int queueElementCount(Queue *q)
{
	if(q == NULL)
	{
		return 0;
	}
	int count=0;	
	pthread_mutex_lock(&q->mutex);
	count = q->count;
	pthread_mutex_unlock(&q->mutex);
	return count;
}

//allocates memory to the queue
//element.
Element* allocateElement()
{
	Element *ele = new Element;
	ele->value = 0;
	ele->next = NULL;
	ele->time_arrived.part_1 = 0;
	ele->time_arrived.part_2 = 0;
	ele->time_entered_q.part_1 = 0;
	ele->time_entered_q.part_2 = 0;
	ele->time_left_q.part_1 = 0;
	ele->time_left_q.part_2 = 0;
	ele->time_begun_service.part_1 = 0;
	ele->time_begun_service.part_2 = 0;
	ele->time_departed.part_1 = 0;
	ele->time_departed.part_2 = 0;
	ele->time_in_system.part_1 = 0;
	ele->time_in_system.part_2 = 0;
	return ele;
}

//deallocate the queue element
void deallocateElement(Element *ele)
{
	if(ele != NULL)
	free(ele);
}

//prints the queue
int printQueue(Queue *q)
{
	if(q == NULL)
	{
		return 0;
	}
	
	pthread_mutex_lock(&q->mutex);
	for(Element *ele = q->head; ele != NULL; ele=ele->next)
	{
		printf("ele value is %d\n", ele->value);
	}

	pthread_mutex_unlock(&q->mutex);
}

//calls cv on wait. 
//but doesnot requests for
//queue lock
int queueWait(Queue *q)
{
	if(q == NULL)
	{
		return 0;
	}
	
	pthread_cond_wait(&q->cond, &q->mutex);
	return 1;
}

//call for acquiring queue lock
int queueLock(Queue *q)
{
	if(q == NULL)
	{
		return 0;
	}
	
	pthread_mutex_lock(&q->mutex);
	return 1;
}

//call for releasing queue lock
int queueUnlock(Queue *q)
{
	if(q == NULL)
	{
		return 0;
	}
	
	pthread_mutex_unlock(&q->mutex);
	return 1;
}

//call for cv signal. but
//don't call queue lock release
int queueSignal(Queue *q)
{
	if(q == NULL)
	{
		return 0;
	}
	
	pthread_cond_signal(&q->cond);
	return 1;
}

//broadcast queue cv.
int queueBroadcast(Queue *q)
{
	if(q == NULL)
	{
		return 0;
	}
	
	pthread_cond_broadcast(&q->cond);
	return 1;
}

//an example queue program
//gives the insight how to use the queue
//and check queue behavior.
int queue_main()
{
	Queue q;
	int i=0, value=0;
	Element element;
	element.value = -1;

	queueInitialize(&q);
	for(i=0; i<10;i++)
	{
		value = rand() % 100;
		Element *ele = allocateElement();
		ele->value = value;
		queueAddToTail(&q, ele);
		printf("Element added is %d\n", ele->value);
	}
	
	printQueue(&q);

	element.value=-1;	
	queueGetHead(&q, &element);
	printf("Element got at head is %d\n", element.value);
	element.value=-1;	
	queueGetTail(&q, &element);
	printf("Element got at tail is %d\n", element.value);

	while(queueElementCount(&q) != 5)
	{
		Element *ele;
		queueRemoveFromHead(&q, &ele);
		printf("Element removed is %d\n", ele->value);
		deallocateElement(ele);
	}
	
	element.value=-1;	
	queueGetHead(&q, &element);
	printf("Element got at head is %d\n", element.value);
	element.value=-1;	
	queueGetTail(&q, &element);
	printf("Element got at tail is %d\n", element.value);

	printQueue(&q);
	for(i=0; i<10;i++)
	{
		value = rand() % 100;
		Element *ele = allocateElement();
		ele->value = value;
		queueAddToTail(&q, ele);
		printf("Element added is %d\n", ele->value);
	}
	printQueue(&q);

	element.value=-1;	
	queueGetHead(&q, &element);
	printf("Element got at head is %d\n", element.value);
	element.value=-1;	
	queueGetTail(&q, &element);
	printf("Element got at tail is %d\n", element.value);


	while(queueElementCount(&q) != 0)
	{
		Element *ele = new Element;
		queueRemoveFromHead(&q, &ele);
		printf("Element removed is %d\n", ele->value);
		deallocateElement(ele);
	}

	printQueue(&q);

	element.value=-1;	
	queueGetHead(&q, &element);
	printf("Element got at head is %d\n", element.value);
	element.value=-1;	
	queueGetTail(&q, &element);
	printf("Element got at tail is %d\n", element.value);
}
