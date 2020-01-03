#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<time.h>

char isolated_memory[16];

pthread_mutex_t lock_IM = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond_thread1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock_thread1 = PTHREAD_MUTEX_INITIALIZER;

void * correct_thread(void *arg)
{	
	// Copy firmware to the shared memory
	char shared_memory[16], authentication_status[16];
	
	pthread_mutex_lock(&lock_thread1);
	strcpy(shared_memory, (char*)arg);
	
	// Copy firmware to the Isolated Memory
	strcpy(isolated_memory, shared_memory);
	printf("Thread 1: Firmware: %s - is copied to the Isolated Memory\n", isolated_memory);
	sleep(3);

	printf("Thread 1: Isolated Memory is locked now for authorization request\n");
	sleep(3);

	// Lock IM and check for authorization
	pthread_mutex_lock(&lock_IM);
	
	if (strcmp("abcd", isolated_memory) == 0)
	{
		strcpy(authentication_status, "PASSED");
	}
	else
	{
		strcpy(authentication_status, "FAILED");
	}

	// Check the status
	// Unlock the Isolated Memory
	pthread_mutex_unlock(&lock_IM);
	
	printf("Thread 1: Authentication is done. Isolated Memory is unlocked\n");
	sleep(3);
	// Start the attack thread here
	pthread_cond_wait(&cond_thread1, &lock_thread1);
	
	if (strcmp(authentication_status, "PASSED") == 0)
	{
		printf("Thread 1: Authentication PASSED: Isolated Memory is running on: %s\n", isolated_memory);
	}
	else
	{	
		printf("Thread 1: Authentication FAILED: Isolated Memory has %s\n", isolated_memory);

	}
	sleep(3);
	pthread_mutex_unlock(&lock_thread1);
		
	pthread_exit(NULL);
}

void * attack_thread(void *arg)
{
	
    	// Copy firmware to the shared memory
	char shared_memory[16], authentication_status[16];

	pthread_mutex_lock(&lock_thread1);

	strcpy(shared_memory, (char*)arg);
	
	// Copy firmware to the Isolated Memory
	strcpy(isolated_memory, shared_memory);
	printf("Thread 2: Firmware - %s is copied to the Isolated Memory\n", isolated_memory);
	sleep(3);	
	pthread_cond_signal(&cond_thread1);
	
	printf("Thread 2: Isolated Memory is locked now for authorization request\n");
	sleep(3);

	// Lock IM and check for authorization
	pthread_mutex_lock(&lock_IM);
	if (strcmp("abcd", isolated_memory) == 0)
	{
		strcpy(authentication_status, "PASSED");
	}
	else
	{
		strcpy(authentication_status, "FAILED");
	}
	
	// Check the status
	// Unlock the Isolated Memory
	pthread_mutex_unlock(&lock_IM);	
	
	printf("Thread 2: Authentication is done. Isolated Memory is unlocked\n");
	sleep(3);	
	if (strcmp(authentication_status, "PASSED") == 0)
	{
		printf("Thread 2: Authentication PASSED: Isolated Memory is running on: %s\n", isolated_memory);
	}
	else
	{
		printf("Thread 2: Authentication FAILED: Isolated Memory is running on: %s\n", isolated_memory);

	}
	sleep(3);
	pthread_mutex_unlock(&lock_thread1);
	pthread_exit(NULL);
}

int main(void)
{
	pthread_t tid1, tid2;

	printf("Creating two threads - Thread 1 with correct firmware and Thread 2 with the mallicious firmware\n");

	if( pthread_create(&tid1, NULL, correct_thread, "abcd") != 0 )
	        printf("Failed to create thread1\n");
  	
	sleep(1);

	if( pthread_create(&tid2, NULL, attack_thread, "Mallicious") != 0 )
        	printf("Failed to create thread2\n");
	
  	pthread_join(tid1,NULL);
	pthread_join(tid2,NULL);
	exit(0);

}
