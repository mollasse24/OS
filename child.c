#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <signal.h>

void sigsys_handler(int signum, siginfo_t *info, void *context) {
    printf("Caught SIGSYS: unauthorized syscall attempted (syscall number: %d)\n", info->si_syscall);
    exit(1);
}

int main() {
    printf("Child PID: %d, Parent PID: %d\n", getpid(), getppid());
    struct sigaction sa;
    sa.sa_sigaction = sigsys_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSYS, &sa, NULL);

    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        printf("Total RAM: %lu bytes\n", info.totalram);
    } else {
        perror("sysinfo failed");
    }

    return 0;
}

