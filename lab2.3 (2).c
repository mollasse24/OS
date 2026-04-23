#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>

sem_t semaphore;

typedef struct
{
    int flag;
    char sym;
} targs;

void* my_flow1(void* arg)
{
    targs* args = (targs*) arg;
    printf("flow1 is working\n");
    
    while (args->flag == 0)
    {
        while (1)
        {
            if (sem_trywait(&semaphore) == 0)
            {
                printf("captured1\n");
                for (int i = 0; i < 10; i++)
                {
                    putchar(args->sym);
                    fflush(stdout);
                    sleep(1);
                }
                printf("\n");
                sem_post(&semaphore);
                printf("free1\n");
                break;
            }
            else
            {
                usleep(10000);
            }
        }
    }
    printf("flow1 is finished\n");
    return NULL;
}

void* my_flow2(void* arg)
{
    targs* args = (targs*) arg;
    printf("flow2 is working\n");
    
    while (args->flag == 0)
    {
        while (1)
        {
            if (sem_trywait(&semaphore) == 0)
            {
                printf("captured2\n");
                for (int i = 0; i < 10; i++)
                {
                    putchar(args->sym);
                    fflush(stdout);
                    sleep(1);
                }
                printf("\n");
                sem_post(&semaphore);
                printf("free2\n");
                break;
            }
            else
            {
                usleep(10000);
            }
        }
    }
    printf("flow2 is finished\n");
    return NULL;
}

int main()
{
    printf("main is working\n");
    targs arg1 = {0, '1'};
    targs arg2 = {0, '2'};

    sem_init(&semaphore, 0, 1);

    pthread_t flow1, flow2;
    pthread_create(&flow1, NULL, my_flow1, &arg1);
    pthread_create(&flow2, NULL, my_flow2, &arg2);

    printf("waiting for a click\n");
    getchar();
    printf("pressed\n");

    arg1.flag = 1;
    arg2.flag = 1;

    pthread_join(flow1, NULL);
    pthread_join(flow2, NULL);

    sem_destroy(&semaphore);
    printf("main is finished\n");
    return 0;
}
