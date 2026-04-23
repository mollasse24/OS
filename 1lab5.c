#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/select.h>
#include <signal.h>       
#include <time.h>         
#define SEM_NAME "/mysem"
#define FILENAME "output.txt"

int main() {
    sem_t *sem;
    FILE *file;
    
    sem = sem_open(SEM_NAME, O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }
    
    file = fopen(FILENAME, "a");
    if (file == NULL) {
        perror("fopen failed");
        sem_close(sem);
        sem_unlink(SEM_NAME);
        exit(EXIT_FAILURE);
    }

    while (1) {        
        sem_wait(sem);
        
        for (int i = 0; i < 10; i++) {
            fputc('1', file);
            fflush(file);  
            putchar('1');
            fflush(stdout);
            sleep(1);
        }
        
        sem_post(sem);
        
        sleep(1);
    
        struct timespec ts = {0, 0};  
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        
        if (pselect(STDIN_FILENO + 1, &fds, NULL, NULL, &ts, NULL) == 1) {
            break;
        }
    }
   
    fclose(file);
    sem_close(sem);
    sem_unlink(SEM_NAME);  
    
    return 0;
}





