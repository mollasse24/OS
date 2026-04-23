#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <seccomp.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

void setup_seccomp(int mode) {
    scmp_filter_ctx ctx;
    uint32_t action;

    switch (mode) {
        case 1: action = SCMP_ACT_KILL_PROCESS; break;
        case 2: action = SCMP_ACT_ERRNO(EPERM); break;
        case 3: action = SCMP_ACT_TRAP; break;
        case 4: action = SCMP_ACT_LOG; break;
        default:
            fprintf(stderr, "Unknown mode: %d\n", mode);
            exit(EXIT_FAILURE);
    }

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        perror("prctl(NO_NEW_PRIVS)");
        exit(EXIT_FAILURE);
    }

    ctx = seccomp_init(SCMP_ACT_ALLOW);
    if (!ctx) {
        perror("seccomp_init");
        exit(EXIT_FAILURE);
    }

    if (seccomp_rule_add(ctx, action, SCMP_SYS(sysinfo), 0) < 0) {
        perror("seccomp_rule_add");
        seccomp_release(ctx);
        exit(EXIT_FAILURE);
    }

    if (seccomp_load(ctx) < 0) {
        perror("seccomp_load");
        seccomp_release(ctx);
        exit(EXIT_FAILURE);
    }

    seccomp_release(ctx);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <mode 1-4>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int mode = atoi(argv[1]);
    if (mode < 1 || mode > 4) {
        fprintf(stderr, "Mode must be 1 to 4.\n");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return EXIT_FAILURE;
    } else if (pid == 0) {
        
        setup_seccomp(mode);
        char *args[] = {"./child", NULL};
        execv("./child", args);
        perror("execv failed");
        exit(EXIT_FAILURE);
    } else {
        
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Child exited with code %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child killed by signal %d (%s)\n", WTERMSIG(status), strsignal(WTERMSIG(status)));
        } else {
            printf("Child terminated unexpectedly\n");
        }
    }

    return 0;
}

