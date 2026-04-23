#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <errno.h>

volatile int finish = 0; // Флаг завершения
mqd_t mq;                // Идентификатор очереди сообщений

void *thread_func(void *arg) {
    char buffer[256];
    unsigned int prio;

    while (!finish) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_read = mq_receive(mq, buffer, sizeof(buffer), &prio);

        if (bytes_read >= 0) {
            printf("Получено сообщение: %s\n", buffer);
        } else {
            if (errno != EAGAIN) {
                perror("mq_receive");
            }
            usleep(100000); 
        }
    }
    return NULL;
}

int main() {
    pthread_t tid;
    const char *queue_name = "/test_queue";

    mq = mq_open(queue_name, O_RDONLY | O_NONBLOCK);
    if (mq == (mqd_t) -1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&tid, NULL, thread_func, NULL) != 0) {
        perror("pthread_create");
        mq_close(mq);
        exit(EXIT_FAILURE);
    }

    printf("Нажмите Enter для завершения приема...\n");
    getchar();
    finish = 1;

    pthread_join(tid, NULL);

    mq_close(mq);
    mq_unlink(queue_name);

    printf("Приемник завершил работу.\n");
    return 0;
}

