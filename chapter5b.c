#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     /* usleep */
#include <pthread.h>
#include <semaphore.h>
#define SHARED 0        /* process-sharing if !=0, thread-sharing if =0*/
#define BUF_SIZE 10
#define MAX_MOD 100000
#define NUM_ITER 200

void *Producer(void *); /* Producer thread */
void *Consumer(void *); /* Consumer thread */

// Join and spawn threads
void joinThreads(void);
void spawnThreads(int threadCount);

sem_t empty;            /* empty: How many empty buffer slots */
sem_t full;             /* full: How many full buffer slots */
sem_t b;                /* b: binary, used as a mutex */
int g_data[BUF_SIZE];   /* shared finite buffer  */
int g_idx;              /* index to next available slot in buffer, 
                           remember that globals are set to zero
                           according to C standard, so no init needed  */

// Consumers pool
pthread_t *consumersPool;
int consumersPoolSize = 0;

// Producers pool
pthread_t *producersPool;
int producersPoolSize = 0;


int main(int argc, char const **argv) {
	printf("main started\n");

	// Initialie the semaphores
	sem_init(&empty, SHARED, BUF_SIZE);
	sem_init(&full, SHARED, 0);
	sem_init(&b, SHARED, 1);

    switch(argc) {
        case 2: {
            spawnThreads(atoi(argv[1]));
        }break;
        default: spawnThreads(1);
    };
    joinThreads();

	printf("main done\n");

	return 0;
}



void spawnThreads(int threadCount) {
    // Allocate space for threads
    producersPool = malloc(threadCount*sizeof(pthread_t));
    consumersPool = malloc(threadCount*sizeof(pthread_t));
    consumersPoolSize = threadCount;
    producersPoolSize = threadCount;

    int i;
    // Spawn threads
    for(i = 0; i < threadCount; ++i) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&producersPool[i], NULL, Producer, &id);
        pthread_create(&consumersPool[i], NULL, Consumer, &id);
    }
}


void joinThreads(void) {
    int i;
    // Join threads
    for(i = 0; producersPoolSize; ++i) {
        pthread_join(producersPool[i], NULL);
    }
    for(i = 0; consumersPoolSize; ++i) {
        pthread_join(consumersPool[i], NULL);
    }

    // Free memory allocated for threads
    if(consumersPool)
        free(consumersPool);
    if(producersPool)
        free(producersPool);
    
    consumersPoolSize = 0;
    producersPoolSize = 0;
}



void *Producer(void *arg) {
	int i=0, j;

	while(i < NUM_ITER) {
		// pretend to generate an item by a random wait
		usleep(rand()%MAX_MOD);
		
		// Wait for at least one empty slot
		sem_wait(&empty);
		// Wait for exclusive access to the buffer
		sem_wait(&b);
		
		// Check if there is content there already. If so, print a warning and exit.
		if(g_data[g_idx] == 1) { 
			printf("Producer overwrites!, idx er %d\n",g_idx); 
			exit(0); 
		}
		
		// Fill buffer with "data" (ie: 1) and increase the index.
		g_data[g_idx]=1;
		g_idx++;
		
		// Print buffer status.
		j=0; printf("(Producer, idx is %d): ",g_idx);
		while(j < g_idx) { j++; printf("="); } printf("\n");
		
		// Leave region with exlusive access
		sem_post(&b);
		// Increase the counter of full bufferslots.
		sem_post(&full);
		
		i++;		
	}

    // Free pointer
    free(arg);

	return 0;
}


void *Consumer(void *arg) {
	int i=0, j;

	while(i < NUM_ITER) {
		// Wait a random amount of time, simulating consuming of an item.
		usleep(rand()%MAX_MOD);
		
		// Wait for at least one slot to be full
		sem_wait(&full);
		// Wait for exclusive access to the buffers
		sem_wait(&b);
		
		// Checkt that the buffer actually contains some data at the current slot.
		if(g_data[g_idx-1] == 0) { 
			printf("Consumes nothing!, idx er %d\n",g_idx);
			exit(0);
		}
		
		// Remove the data from the buffer (ie: Set it to 0) 
		g_data[g_idx-1]=0;
		g_idx--;
		
		// Print the current buffer status
		j=0; printf("(Consumer, idx is %d): ",g_idx);
		while(j < g_idx) { j++; printf("="); } printf("\n");
		
		// Leave region with exclusive access
		sem_post(&b);
		// Increase the counter of empty slots.
		sem_post(&empty);  	

		i++;
	}
    
    // Free pointer
    free(arg);

	return 0;

}
