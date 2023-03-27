#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void handler(int signum, siginfo_t * si, void *p3) {
    printf("Handling signal %d, %d war %d\n", si->si_pid, si->si_uid, si->si_value);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Incorrect arguments\n");
        return 1;
    }
    pid_t pid;
    union sigval war;

    pid = atoi(argv[1]);
    sigqueue(pid, SIGUSR1, war);



    return 0;
}