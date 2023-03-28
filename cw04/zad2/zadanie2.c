#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int depth = 0;
int id = 0;
#define DEPTH_LIMIT 5

void info_handler(int sign , siginfo_t *info, void *p3 ) {
    printf("Signal number: %d, PID: %d\n", info->si_signo, info->si_pid);
}

void depth_handler(int sign , siginfo_t *info, void *p3 ) {
    int current = id;
    printf("IN -> ID : %d , DEPTH: %d\n", current, depth);
    id++;
    depth++;

    if (id < DEPTH_LIMIT) {
        raise(SIGUSR1);
    }
    depth--;
    printf("OUT ->  ID : %d , DEPTH: %d\n", current, depth);
    printf("Signal info -> signal number: %d, PID: %d\n", info->si_signo, info->si_pid);
}

void install_handler(void (*handler)(int, siginfo_t*, void*), int sign , int flag){
    struct sigaction act;
    act.sa_sigaction=handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = flag;
    sigaction(sign,&act,NULL);
}

int main(void) {

    install_handler(info_handler , SIGUSR1 , SA_SIGINFO);
    printf("SA_SIGINFO TEST\n");
    raise(SIGUSR1);

    install_handler(depth_handler , SIGUSR1 , SA_NODEFER);
    printf("SA_NODEFER TEST\n");
    raise(SIGUSR1);

//    install_handler(depth_handler , SIGUSR1 , SA_RESETHAND);
//    printf("SA_RESETHAND TEST\n");
//    raise(SIGUSR1);



    return 0;
}