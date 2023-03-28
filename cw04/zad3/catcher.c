#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int requests_no = 0;
int current_mode;
int to_execute = 0;

void write_numbers() {
    for (int i = 1; i <= 100; i++) {
        printf("%d\n", i);
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
    printf(">>>CATCHER: Number of requests received: %d\n", requests_no);
}

void get_time_loop() {
    while(current_mode == 4) {
        get_current_time();
        sleep(1);
    }
    to_execute = 1;
}

void execute_mode(int work_mode) {
    switch (work_mode) {
        case 1:
            write_numbers();
            to_execute = 0;
            break;
        case 2:
            get_current_time();
            to_execute = 0;
            break;
        case 3:
            get_requests_num();
            to_execute = 0;
            break;
        case 4:
            get_time_loop();
            break;
        case 5:
            printf(">>>CATCHER: ending program\n");
            exit(0);
        default:
            fprintf(stderr, "Incorrect work mode value\n");
    }

}

void handler(int sign, siginfo_t* info, void* c){
    pid_t sender_pid = info->si_pid;
    current_mode = info->si_status;
    printf(">>>CATCHER: Signal %d received\n", sign);
    requests_no++;
    kill(sender_pid, SIGUSR1);
    printf(">>>CATCHER: signal confirmation sent to sender\n");
    to_execute = 1;
}

int main(void) {
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);

    printf(">>>CATCHER: starting program -> PID: %d\n", getpid());

    while (1) {
        if (to_execute) {
        execute_mode(current_mode);
        }
    }
    return 0;
}