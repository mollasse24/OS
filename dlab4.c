#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]) {
    
    printf("\n\nChild PID: %d\n", getpid());
    printf("Parent PID: %d\n", getppid());

    if (envp != NULL && envp[0] != NULL) {
        printf("Environment variables:\n");
        for (int i = 0; envp[i] != NULL; i++) {
            printf("%s\n", envp[i]);
        }
    }
    for (int i = 1; i < argc; i++) {
        printf("Arg %d: %s\n", i, argv[i]);
        sleep(1);
    }
    return 5;
}

