#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void handler(int sign, siginfo_t* info, void* c){
    pid_t sender_pid = info->si_pid;
    printf(">>>CATHCER: Signal %d received\n", sign);
    sleep(1);
    kill(sender_pid, SIGUSR1);
    printf(">>>CATCHER: signal sent\n");
}

int main(void) {
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);

    printf("%d\n", getpid());

    while (1) {

    }

    return 0;
}