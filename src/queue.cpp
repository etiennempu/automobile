#include <iostream>
#include <stdint.h>       /* int32_t uint32_t */
#include <pthread.h>      /* pthread_t pthread_create pthread_join */

#include "queue.h"

void* producer_task(void* args)
{
	nsCommon::Queue<uint32_t>* queue = (nsCommon::Queue<uint32_t>*) args;
	uint32_t nbSentMessages = 0;
	while(true)
	{
		sleep(1);
		std::cout << "[Producer] sending int: " << nbSentMessages << std::endl;
		queue->push(nbSentMessages++);
	}
}

void* consumer_task(void* args)
{
	nsCommon::Queue<uint32_t>* queue = (nsCommon::Queue<uint32_t>*) args;
	uint32_t receivedInt;
	while(true)
	{
		receivedInt = queue->pop();
		std::cout << "[Consumer] received int: " << receivedInt << std::endl;
		sleep(2);
	}
}

//int main() {
//	pthread_t prodTid, consTid;
//	nsCommon::Queue<uint32_t> sharedQueue(5);
//
//	if(0 != pthread_create(&prodTid, NULL, producer_task, &sharedQueue))
//	{
//		/* Print error */
//		printf("Could not create thread: %d\n", errno);
//		return EXIT_FAILURE;
//	}
//	if(0 != pthread_create(&consTid, NULL, consumer_task, &sharedQueue))
//	{
//		/* Print error */
//		printf("Could not create thread: %d\n", errno);
//		return EXIT_FAILURE;
//	}
//
//	pthread_join(prodTid, NULL);
//	pthread_join(consTid, NULL);
//	return 0;
//}
