#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <sys/utsname.h>

volatile int finish = 0; // Флаг завершения
mqd_t mq;                // Идентификатор очереди сообщений

void *thread_func(void *arg) {
    char domainname[256];
    while (!finish) {
        if (getdomainname(domainname, sizeof(domainname)) != 0) {
            perror("getdomainname");
            strcpy(domainname, "unknown");
        }
        printf("Отправка: %s\n", domainname);

        if (mq_send(mq, domainname, strlen(domainname) + 1, 0) == -1) {
            perror("mq_send");
        }

        sleep(1);
    }
    return NULL;
}

int main() {
    struct mq_attr attr;
    pthread_t tid;
    const char *queue_name = "/test_queue";

    attr.mq_flags = O_NONBLOCK;
    attr.mq_maxmsg = 10;        
    attr.mq_msgsize = 256;      
    attr.mq_curmsgs = 0;

    mq = mq_open(queue_name, O_CREAT | O_WRONLY | O_NONBLOCK, 0666, &attr);
    if (mq == (mqd_t) -1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&tid, NULL, thread_func, NULL) != 0) {
        perror("pthread_create");
        mq_close(mq);
        mq_unlink(queue_name);
        exit(EXIT_FAILURE);
    }

    printf("Нажмите Enter для завершения отправки...\n");
    getchar();
    finish = 1;

    pthread_join(tid, NULL);

    mq_close(mq);
    mq_unlink(queue_name);

    printf("Отправитель завершил работу.\n");
    return 0;
}

