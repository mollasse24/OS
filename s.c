#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <sys/utsname.h>

#define SOCKET_PATH "/tmp/udp_unix_socket_server.soc"
#define BUFFER_SIZE 256
#define _GNU_SOURCE
struct request {
    char data[BUFFER_SIZE];
    struct sockaddr_un client_addr;
    socklen_t client_addr_len;
    STAILQ_ENTRY(request) entries;
};

STAILQ_HEAD(request_queue, request);

struct request_queue queue;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

volatile int finish = 0;
int server_socket = -1;

 void *receive_thread(void *arg) {
    while (!finish) {
        struct request *req = malloc(sizeof(struct request));
        if (!req) continue;

        memset(req, 0, sizeof(*req));
        // Инициализируем длину адреса клиента
        req->client_addr_len = sizeof(req->client_addr);

        ssize_t bytes = recvfrom(server_socket, req->data, sizeof(req->data), 0,
                                 (struct sockaddr *)&req->client_addr, &req->client_addr_len);

        if (bytes > 0) {
            pthread_mutex_lock(&queue_mutex);
            STAILQ_INSERT_TAIL(&queue, req, entries);
            pthread_mutex_unlock(&queue_mutex);

            printf("[Server] Received request: %s\n", req->data);
            printf("[Server] Client address: %s\n", req->client_addr.sun_path);
        } else {
            free(req);
            usleep(100000);
        }
    }
    return NULL;
}


void *process_thread(void *arg) {
    while (!finish) {
        pthread_mutex_lock(&queue_mutex);
        struct request *req = STAILQ_FIRST(&queue);
        if (req) {
            STAILQ_REMOVE_HEAD(&queue, entries);
        }
        pthread_mutex_unlock(&queue_mutex);

        if (req) {
            char response[BUFFER_SIZE];
            char domain[BUFFER_SIZE] = {0}; 
            getdomainname(domain, sizeof(domain));     
            snprintf(response, sizeof(response), "%s | Domain: %s", req->data, domain);

            sendto(server_socket, response, strlen(response) + 1, 0,
                   (struct sockaddr *)&req->client_addr, req->client_addr_len);

            printf("[Server] Sent response: %s\n", response);
            free(req);
        } else {
            usleep(100000);
        }
    }
    return NULL;
}

void cleanup() {
    if (server_socket != -1) close(server_socket);
    unlink(SOCKET_PATH);
}

void signal_handler(int sig) {
    finish = 1;
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    STAILQ_INIT(&queue);

    server_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (server_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    fcntl(server_socket, F_SETFL, O_NONBLOCK);

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
    unlink(SOCKET_PATH);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        cleanup();
        exit(EXIT_FAILURE);
    }

    printf("[Server] Started at %s\n", SOCKET_PATH);

    pthread_t recv_tid, proc_tid;
    pthread_create(&recv_tid, NULL, receive_thread, NULL);
    pthread_create(&proc_tid, NULL, process_thread, NULL);

    getchar(); // Ожидание нажатия клавиши
    finish = 1;

    pthread_join(recv_tid, NULL);
    pthread_join(proc_tid, NULL);

    cleanup();
    printf("[Server] Shutdown.\n");
    return 0;
}

