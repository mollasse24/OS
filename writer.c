#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/utsname.h>

#define SHM_KEY 0x1234
#define DOMAIN_NAME_MAX 64
#define SEM_WRITE_NAME "/sem_write"
#define SEM_READ_NAME "/sem_read"

typedef struct {
    char domain[DOMAIN_NAME_MAX];
} shared_data_t;

void* writer_thread(void* arg) {
    shared_data_t* data = (shared_data_t*)arg;

    sem_t* sem_write = sem_open(SEM_WRITE_NAME, 0);
    sem_t* sem_read  = sem_open(SEM_READ_NAME, 0);

    if (sem_write == SEM_FAILED || sem_read == SEM_FAILED) {
        perror("sem_open");
        pthread_exit(NULL);
    }

    while (1) {
        char domain[DOMAIN_NAME_MAX];
        if (getdomainname(domain, DOMAIN_NAME_MAX) == -1) {
            perror("getdomainname");
            break;
        }

        if (sem_wait(sem_write) == -1) {
            perror("sem_wait write");
            break;
        }

        strncpy(data->domain, domain, DOMAIN_NAME_MAX - 1);
        data->domain[DOMAIN_NAME_MAX - 1] = '\0';
        printf("Writer: wrote '%s'\n", domain);

        sem_post(sem_read);
        sleep(1);
    }

    sem_close(sem_write);
    sem_close(sem_read);
    pthread_exit(NULL);
}

int main() {
    int shmid = shmget(SHM_KEY, sizeof(shared_data_t), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    shared_data_t* data = (shared_data_t*)shmat(shmid, NULL, 0);
    if (data == (void*)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    sem_t* sem_write = sem_open(SEM_WRITE_NAME, O_CREAT, 0666, 1);
    sem_t* sem_read  = sem_open(SEM_READ_NAME,  O_CREAT, 0666, 0);

    if (sem_write == SEM_FAILED || sem_read == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    pthread_t tid;
    pthread_create(&tid, NULL, writer_thread, data);

    printf("Writer: Press Ctrl+C to stop...\n");

    pthread_join(tid, NULL);  

    shmdt(data);
    shmctl(shmid, IPC_RMID, NULL);

    sem_close(sem_write);
    sem_close(sem_read);
    sem_unlink(SEM_WRITE_NAME);
    sem_unlink(SEM_READ_NAME);

    return 0;
}

