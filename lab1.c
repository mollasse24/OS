#include "pthread.h"
#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"

typedef struct
{
  int flag; 
  char sym;
}targs;

void* my_flow1(void* arg)
{
  printf("flow1 is working\n");
  targs* args = (targs*) arg;  
  while(args->flag == 0)
  {
    putchar(args->sym); 
    fflush(stdout);
    sleep(1);
  }
  printf("flow1 is finished\n");
  int* exit_code = malloc(sizeof(int));
  *exit_code = 8;
  pthread_exit(exit_code);
}

void* my_flow2(void* arg)
{
  printf("flow2 is working\n");
  targs* args = (targs*) arg;  
  while(args->flag == 0)
  {
    putchar(args->sym); 
    fflush(stdout);
    sleep(1);
  }
  printf("flow2 is finished\n");
  int* exit_code = malloc(sizeof(int));
  *exit_code = 9;
  pthread_exit(exit_code);  
}

int main()
{
  printf("main is working\n"); 
  int* exitcode1, exitcode2;
  targs arg1, arg2;
  pthread_t thread1, thread2; 
  
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  
  arg1.flag = 0;
  arg1.sym = '1';
  arg2.flag = 0;
  arg2.sym = '2'; 
  pthread_create(&thread1, &attr, my_flow1, &arg1);
  pthread_create(&thread2, &attr, my_flow2, &arg2); 
  
  pthread_attr_destroy(&attr);
  
  printf("waiting for a click\n");
  getchar(); 
  printf("pressed\n");
  arg1.flag = 1;
  arg2.flag = 1;
  
  int  err1 = pthread_join(thread1, (void**) &exitcode1); 
  int err2 = pthread_join(thread2, (void**) &exitcode2); 
  if(err1 != 0)
    printf("thread1 fail: %d\n", err1);
  if(err2 != 0)
    printf("thread2 fail: %d\n", err2);
    
  /*printf("%d\n", *exitcode1);
  printf("%d\n", *exitcode2);*/ 
  
  printf("main is finished\n");
  return 0;
}
