#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

void confirm_signal(int sign) {
    printf(">>>SENDER: catcher has received and processed signal\n");
}

void send_to_catcher(pid_t catcher_pid, int mode) {
    sigset_t set;
//    union sigval sig_val;
//    sig_val.si = mode;
    sigval_t sig_val = {mode};
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);

    signal(SIGUSR1, confirm_signal);
    sigset_t newmask, oldmask;

    sigemptyset(&newmask);
    sigaddset(&newmask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);

    //kill(catcher_pid, SIGUSR1);
    sigqueue(catcher_pid, SIGUSR1, sig_val);
    printf(">>>SENDER: Signal was sent\n");

    sigsuspend(&oldmask);
    sigprocmask(SIG_UNBLOCK, &newmask, NULL);
}

int main(int argc, char** argv) {
    if (argc <= 2) {
        fprintf(stderr, "Incorrect arguments!\n");
        return 1;
    }
    pid_t catcher_pid = atoi(argv[1]);

    int work_mode;
    for (int i = 2; i < argc; i++) {

        work_mode = atoi(argv[i]);
        send_to_catcher(catcher_pid, work_mode);
    }
    return 0;
}