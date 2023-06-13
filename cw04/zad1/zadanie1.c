#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void handler(int signum){
    printf("Signal %d was handled correctly\n", signum);
}

void test_fork(int val) {
    sigset_t newmask, oldmask, set;
    pid_t pid;

    switch (val) {
        case 0:
            signal(SIGUSR1, SIG_IGN);
            raise(SIGUSR1);
            printf("SIGUSR1 sent by parent process\n");

            pid = fork();
            if (pid == 0){
                printf("SIGUSR1 sent by child process\n");
                raise(SIGUSR1);
                exit(0);
            }
            break;

        case 1:
            signal(SIGUSR1, handler);
            printf("SIGUSR1 ent by parent process\n");
            raise(SIGUSR1);

            pid = fork();
            if (pid == 0){
                printf("SIGUSR1 sent by child process\n");
                raise(SIGUSR1);
                exit(0);
            }
            break;

        case 2:
            sigemptyset(&newmask);
            sigaddset(&newmask, SIGUSR1);
            sigprocmask(SIG_BLOCK, &newmask, &oldmask);

            printf("SIGUSR1 sent by parent process\n");
            raise(SIGUSR1);

            pid = fork();
            if (pid == 0) {
                raise(SIGUSR1);
                printf("SIGUSR1 sent by child process\n");
                exit(0);
            }
            break;

        case 3:
            sigemptyset(&newmask);
            sigaddset(&newmask, SIGUSR1);
            sigprocmask(SIG_BLOCK, &newmask, &oldmask);

            printf("SIGUSR1 sent by parent process\n");
            raise(SIGUSR1);

            sigpending(&set);
            if (sigismember(&set, SIGUSR1) == 1) {
                printf("SIGUSR1 is pending in parent process\n");
            } else {
                printf("SIGUSR1 is NOT pending in parent process\n");
            }

            pid = fork();
            if (pid == 0){
                sigpending(&set);
                if (sigismember(&set, SIGUSR1) == 1) {
                    printf("SIGUSR1 is pending in child process\n");
                } else {
                    printf("SIGUSR1 is NOT pending in child process\n");
                }
                exit(0);
            }
            break;

        default:
            fprintf(stderr, "Incorrect argument value\n");
            break;
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr ,"Incorrect arguments!\n");
        return 1;
    }
    int val;
    /*
     * val depending on input:
     * 0 - ignore
     * 1 - handler
     * 2 - mask
     * 3 - pending
     */
    if (strcmp(argv[1], "ignore") == 0) val = 0;
    if (strcmp(argv[1], "handler") == 0) val = 1;
    if (strcmp(argv[1], "mask") == 0) val = 2;
    if (strcmp(argv[1], "pending") == 0) val = 3;

    test_fork(val);

    while (wait(NULL) > 0);

    printf("TESTING EXEC: \n");

    execl("./exec_test", "./exec_test", argv[1], NULL);

    return 0;
}