#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    
    printf("Parent program arguments:\n");
    for (int i = 1; i < argc; i++) {
        printf("Arg %d: %s\n", i, argv[i]);
    }

    printf("Parent PID: %d\n", getppid());
    printf("Self PID: %d\n", getpid());
  
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        
        char *envp[] = {"VAR1=value1", "VAR2=value2", NULL};
        
        char *child_args[] = {
            "./dlab4",  
            argv[1],       
            argv[2],      
            NULL           
        };
        
        execle("./dlab4", child_args[0], child_args[1], child_args[2], NULL, envp);
      
        perror("execle failed");
        exit(1);
    } else {
        
        printf("Child PID: %d\n", pid);
        
        
        int status;
        while(waitpid(pid, &status, WNOHANG) == 0);
        {
            printf("\n\nwaiting\n\n");
            sleep(1);
        }
        
        if (WEXITSTATUS(status) == 5) {
            printf("Child exited with status: %d\n", WEXITSTATUS(status));
        }
    }

    return 0;
}

