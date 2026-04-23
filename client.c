#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>

#define SERVER_SOCKET_PATH "/tmp/udp_unix_socket_server.soc"
#define CLIENT_SOCKET_PATH "/tmp/udp_unix_socket_client.soc"
#define BUFFER_SIZE 256

volatile int finish = 0;
int client_socket = -1;

void *send_thread(void *arg) {
    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SERVER_SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    int counter = 1;
    while (!finish) {
        char buffer[BUFFER_SIZE];
        snprintf(buffer, sizeof(buffer), "Request #%d", counter++);

        sendto(client_socket, buffer, strlen(buffer) + 1, 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr));

        printf("[Client] Sent: %s\n", buffer);
        sleep(1);
    }
    return NULL;
}

void *receive_thread(void *arg) {
    while (!finish) {
        char buffer[BUFFER_SIZE];
        struct sockaddr_un from;
        socklen_t fromlen = sizeof(from);

        ssize_t bytes = recvfrom(client_socket, buffer, sizeof(buffer), 0,
                                 (struct sockaddr *)&from, &fromlen);

        if (bytes > 0) {
            printf("[Client] Received: %s\n", buffer);
        } else {
            usleep(100000);
        }
    }
    return NULL;
}

void cleanup() {
    if (client_socket != -1) close(client_socket);
    unlink(CLIENT_SOCKET_PATH);
}

void signal_handler(int sig) {
    finish = 1;
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    client_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    fcntl(client_socket, F_SETFL, O_NONBLOCK);

    struct sockaddr_un client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sun_family = AF_UNIX;
    strncpy(client_addr.sun_path, CLIENT_SOCKET_PATH, sizeof(client_addr.sun_path) - 1);
    unlink(CLIENT_SOCKET_PATH);

    if (bind(client_socket, (struct sockaddr *)&client_addr, sizeof(client_addr)) == -1) {
        perror("bind");
        cleanup();
        exit(EXIT_FAILURE);
    }

    printf("[Client] Bound to %s\n", CLIENT_SOCKET_PATH);

    pthread_t send_tid, recv_tid;
    pthread_create(&send_tid, NULL, send_thread, NULL);
    pthread_create(&recv_tid, NULL, receive_thread, NULL);

    getchar(); // Ожидание нажатия клавиши
    finish = 1;

    pthread_join(send_tid, NULL);
    pthread_join(recv_tid, NULL);

    cleanup();
    printf("[Client] Shutdown.\n");
    return 0;
}

