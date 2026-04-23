#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#define SHM_KEY 0x1234  
#define DOMAIN_NAME_MAX 64
#define SEM_WRITE_NAME "/sem_write"
#define SEM_READ_NAME "/sem_read"

typedef struct {
    char domain[DOMAIN_NAME_MAX];
} shared_data_t;


void* reader_thread(void* arg) {
    shared_data_t* data = (shared_data_t*)arg;

    sem_t* sem_write = sem_open(SEM_WRITE_NAME, 0);
    sem_t* sem_read  = sem_open(SEM_READ_NAME, 0);

    if (sem_write == SEM_FAILED || sem_read == SEM_FAILED) {
        perror("sem_open");
        pthread_exit(NULL);
    }

    while (1) {
        if (sem_wait(sem_read) == -1) {
            if (errno == EINTR) break;
            perror("sem_wait");
            break;
        }

        printf("Reader: domain = %s\n", data->domain);

        if (sem_post(sem_write) == -1) {
            perror("sem_post");
            break;
        }
       
        usleep(100000); 
    }

  
    sem_close(sem_write);
    sem_close(sem_read);
    printf("Reader thread exiting\n");
    pthread_exit(NULL);
}

int main() {
 
    int shmid = shmget(SHM_KEY, sizeof(shared_data_t), 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

   
    shared_data_t* data = (shared_data_t*)shmat(shmid, NULL, 0);
    if (data == (void*)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    pthread_t thread;
    pthread_create(&thread, NULL, reader_thread, data);

    printf("Reader: Press Ctrl+C to stop...\n");

   
    pthread_join(thread, NULL);
    shmdt(data);

    printf("Reader: Shutting down...\n");

    return 0;
}

