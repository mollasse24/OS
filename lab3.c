
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>

typedef struct {
    int flag;
    const char* thread_name;
} targs;

int pipefd[2];
targs writer_args = {0, "Writer"};
targs reader_args = {0, "Reader"};

void* writer_thread(void* arg) {
    targs* args = (targs*)arg;
    char buffer[256];
    
    printf("[%s] Thread started\n", args->thread_name);
    
    while (!args->flag) {
        if (getdomainname(buffer, sizeof(buffer))) {
            fprintf(stderr, "[%s] Error: getdomainname failed (%s)\n", 
                    args->thread_name, strerror(errno));
            break;
        }
        
        printf("[%s] Trying to write domain '%s' to pipe...\n", 
               args->thread_name, buffer);
        
        ssize_t bytes_written = write(pipefd[1], buffer, strlen(buffer) + 1);
        
        if (bytes_written == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("[%s] Pipe is full, waiting...\n", args->thread_name);
                usleep(100000); // 100ms задержка вместо sleep(1)
            } else {
                fprintf(stderr, "[%s] Write error: %s\n", 
                        args->thread_name, strerror(errno));
                break;
            }
        } else {
            printf("[%s] Successfully wrote %zd bytes to pipe\n", 
                   args->thread_name, bytes_written);
            usleep(500000); // 500ms задержка между записями
        }
    }
    
    printf("[%s] Thread terminating\n", args->thread_name);
    return NULL;
}

void* reader_thread(void* arg) {
    targs* args = (targs*)arg;
    char buffer[256];
    
    printf("[%s] Thread started\n", args->thread_name);
    
    while (!args->flag) {
        memset(buffer, 0, sizeof(buffer));
        
        ssize_t bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1);
        
        if (bytes_read == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(100000); // 100ms задержка при пустом канале
                continue;
            } else {
                fprintf(stderr, "[%s] Read error: %s\n", 
                        args->thread_name, strerror(errno));
                break;
            }
        } else if (bytes_read > 0) {
            printf("[%s] Received %zd bytes: '%s'\n", 
                   args->thread_name, bytes_read, buffer);
        } else {
            printf("[%s] Pipe closed by writer\n", args->thread_name);
        }
    }
    
    printf("[%s] Thread terminating\n", args->thread_name);
    return NULL;
}

int main(int argc, char *argv[]) {
    printf("\n=== Program started ===\n");      
    
    if (argc != 2 || atoi(argv[1]) < 1 || atoi(argv[1]) > 3) {
        fprintf(stderr, "Usage: %s <mode>\n"
                "Available modes:\n"
                "1 - Blocking pipe\n"
                "2 - Non-blocking pipe (pipe2)\n"
                "3 - Non-blocking pipe (fcntl)\n", argv[0]);
        return EXIT_FAILURE;
    }

    int mode = atoi(argv[1]);
    printf("\nSelected mode: %d\n", mode);

switch (mode) {
        case 1:
            printf("Creating pipe in blocking mode...\n");
            if (pipe(pipefd) == -1) {
                perror("pipe() failed");
                return EXIT_FAILURE;
            }
            break;
            
        case 2:
            printf("Creating pipe in non-blocking mode (pipe2)...\n");
            if (pipe2(pipefd, O_NONBLOCK) == -1) {
                perror("pipe2() failed");
                return EXIT_FAILURE;
            }
            break;
            
        case 3:
            printf("Creating pipe in non-blocking mode (fcntl)...\n");
            if (pipe(pipefd) == -1) {
                perror("pipe() failed");
                return EXIT_FAILURE;
            }
            for (int i = 0; i < 2; i++) {
                int flags = fcntl(pipefd[i], F_GETFL);
                if (fcntl(pipefd[i], F_SETFL, flags | O_NONBLOCK) == -1) {
                    perror("fcntl() failed");
                    return EXIT_FAILURE;
                }
            }
            break;
    }

    printf("Pipe created successfully\n");
    

    printf("\nStarting threads...\n");
    pthread_t writer, reader;
    
    if (pthread_create(&writer, NULL, writer_thread, &writer_args)) {
        perror("Failed to create writer thread");
        return EXIT_FAILURE;
    }
    
    if (pthread_create(&reader, NULL, reader_thread, &reader_args)) {
        perror("Failed to create reader thread");
        return EXIT_FAILURE;
    }

    printf("\nThreads started successfully\n");    
    
    printf("\nPress Enter to stop or Ctrl+C to interrupt...\n");
    getchar();
    
    printf("\nTermination requested. Stopping threads...\n");
    writer_args.flag = 1;
    reader_args.flag = 1;
    
    printf("Waiting for threads to finish...\n");
    pthread_join(writer, NULL);
    pthread_join(reader, NULL);
    
    printf("Closing pipe...\n");
    close(pipefd[0]);
    close(pipefd[1]);
    
    printf("\n=== Program finished ===\n");
    return 0;
}
