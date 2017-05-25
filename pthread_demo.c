#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 8
#define END_FLAG -1
struct Products
{
	int buffer[BUFFER_SIZE];
	/*保证存取操作的原子性 互斥性*/
	pthread_mutex_t locker;
	/*是否可读*/
	pthread_cond_t notEmpty;
	/*是否可写*/
	pthread_cond_t notFull;
	int posReadFrom;
	int posWriteTo;
};

int BufferIsFull(struct Products* products)
{
	if ((products->posWriteTo + 1) % BUFFER_SIZE == products->posReadFrom)
		return (1);
	
	return (0);
}

int BufferIsEmpty(struct Products* products)
{
	if (products->posWriteTo == products->posReadFrom)

		return (1);
	return (0);
}

void Produce(struct Products* products, int item)
{
	pthread_mutex_lock(&products->locker);
	while(BufferIsFull(products))
		pthread_cond_wait(&products->notFull, &products->locker);
	products->buffer[products->posWriteTo] = item;
	products->posWriteTo++;
	if (products->posWriteTo >= BUFFER_SIZE)
		products->posWriteTo = 0;
	pthread_cond_signal(&products->notEmpty);
	pthread_mutex_unlock(&products->locker);
}
int Consume(struct Products* products)
{
	int item;
	pthread_mutex_lock(&products->locker);
	while(BufferIsEmpty(products))
		pthread_cond_wait(&products->notEmpty, &products->locker);
	item = products->buffer[products->posReadFrom];
	products->posReadFrom++;
	if(products->posReadFrom >= BUFFER_SIZE)
		products->posReadFrom = 0;
	pthread_cond_signal(&products->notFull);
	pthread_mutex_unlock(&products->locker);
	return item;
}
struct Products products;
void* ProducerThread(void* data)
{
	int i;
	for(i=0; i < 16; ++i) {
		printf("producer: %d\n", i);
		Produce(&products, i);
	}
	Produce(&products, END_FLAG);
	puts("[+] ProducerThread done");
	return NULL;
}

void* ConsumerThread(void* data)
{ 
	int item;
	while(1) {
		item = Consume(&products);
		if( END_FLAG == item)
			break;
		printf("consumer: %d\n", item);
	}
	puts("[+] ConsumerThread done");
	return NULL;
}

int main(void) {
	pthread_t producer, consumer;
	pthread_create(&producer, NULL, &ProducerThread, NULL);
	pthread_create(&consumer, NULL, &ConsumerThread, NULL);
	puts("[+] Created producer and consumer");
	pthread_join(producer, NULL);
	pthread_join(consumer, NULL);
	return 0;
}

