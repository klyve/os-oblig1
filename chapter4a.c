#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int forkCount = 6;

void process(int pid, int time);
void runFork(int pid, int sec, pid_t forks[forkCount]);

int main(void) {
    pid_t forks[forkCount];
    
    runFork(0, 1, forks); 
    runFork(2, 3, forks);
    waitpid(forks[0], NULL, 0);

    runFork(1, 2, forks);
    runFork(4, 3, forks);
    waitpid(forks[1], NULL, 0);

    runFork(3, 2, forks);
    waitpid(forks[4], NULL, 0);

    runFork(5, 3, forks);
    waitpid(forks[5], NULL, 0);

    return 0;
}




void process(int pid, int time) {
    printf("Prosess %d kjører\n", pid);
    sleep(time);
    printf("Prosess %d kjørte i %d sekunder\n", pid, time);
}

// Run the fork 
void runFork(int pid, int sec, pid_t forks[forkCount]) {
  forks[pid] = fork();
  if(forks[pid] == 0) {
    process(pid, sec);
    exit(0);
  }
}