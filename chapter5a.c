#include <stdio.h>        // printf
#include <stdlib.h>       // exit
#include <unistd.h>       // fork
#include <sys/wait.h>     // waitpid
#include <sys/types.h>    // pid_t
#include <pthread.h>
#ifndef __APPLE__
    #include <semaphores.h>
    sem_t mutex;
#else
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#endif


pthread_t threads[6];
void *process(void *args);
void spawn(int num, int time);
void joinThread(int id);
// Thread arguments
struct threadArgs {
  int num;
  int time;
};


/*
    Using Mutex instead of semaphores on OSX
    semaphores is deprecated on OSX. 
*/
int main() {
  
  #ifndef __APPLE__
      sem_init(&mutex, 0, 3);
  #endif

  spawn(0, 1);
  spawn(2, 3);

  joinThread(0);
  spawn(1, 2);
  spawn(4, 3);

  spawn(3, 2);

  joinThread(4);
  spawn(5, 3);

  joinThread(5);

  return 0;
}


// Function for the threads to run
void *process(void *args) {

    #ifndef __APPLE__
        sem_wait(&mutex);
    #else 
        pthread_mutex_lock(&mutex);
    #endif


    struct threadArgs a = *((struct threadArgs *)args);
    printf("Prosess %d kjører\n", a.num);

    sleep(a.time);

    #ifndef __APPLE__
        sem_post(&mutex);
    #else 
        pthread_mutex_unlock(&mutex);
    #endif

    printf("Prosess %d kjørte i %d sekunder\n", a.num, a.time);

    free(args); // Free memory
    return NULL;
}


// Spawn a new thread
void spawn(int num, int time) {
    // Allocate space for thread args
  struct threadArgs *args = malloc(sizeof(struct threadArgs));
  args->num = num;
  args->time = time; 

  
  int status = pthread_create(&threads[num], NULL, process, args);
  if (status != 0) exit(EXIT_FAILURE);
}

// Joins a specified thread
void joinThread(int id) {
  pthread_join(threads[id], NULL);
}