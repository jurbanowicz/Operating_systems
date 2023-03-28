#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int requests_no = 0;

void write_numbers() {
    for (int i = 1; i <= 100; i++) {
        printf("%d", i);
    }
}
void get_current_time() {
    time_t raw_time;
    struct tm * current_time;
    time ( &raw_time );
    current_time = localtime ( &raw_time );

    printf(">>>CATCHER: Current time is: %s\n" , asctime (current_time));
}

void get_requests_num(){
    printf("%d", requests_no);
}

void get_time_loop() {
    while(1) {
        get_current_time();
        sleep(1);
    }
}

void execute_mode(int work_mode) {
    switch (work_mode) {
        case 1:
            write_numbers();
            break;
        case 2:
            get_current_time();
            break;
        case 3:
            get_requests_num();
            break;
        case 4:
            get_time_loop();
            break;
        case 5:
            printf("CATCHER: ending program\n");
            exit(0);
        default:
            fprintf(stderr, "Incorrect signal value\n");
            break;
    }
}

void handler(int sign, siginfo_t* info, void* c){
    pid_t sender_pid = info->si_pid;
    printf(">>>CATHCER: Signal %d received\n", sign);
    requests_no++;
    kill(sender_pid, SIGUSR1);
    printf(">>>CATCHER: signal sent\n");
    execute_mode(info->si_status);
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