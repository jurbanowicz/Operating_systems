#include <stdio.h>
#include <string.h>
#include <signal.h>

void test_exec(int val) {
    sigset_t newmask, oldmask, set;

    switch (val) {
        case 0:
            raise(SIGUSR1);
            printf("SIGUSR1 sent by exec process\n");
            break;

        case 1:
            printf("Handler not tested by exec process\n");
            break;

        case 2:
            printf("SIGUSR1 sent by exec process\n");
            raise(SIGUSR1);
            break;

        case 3:
            sigemptyset(&newmask);
            sigaddset(&newmask, SIGUSR1);
            sigprocmask(SIG_BLOCK, &newmask, &oldmask);

            sigpending(&set);
            if (sigismember(&set, SIGUSR1) == 1) {
                printf("SIGUSR1 is pending in exec process\n");
            } else {
                printf("SIGUSR1 is NOT pending in exec process\n");
            }
            break;

        default:
            fprintf(stderr, "Incorrect argument value\n");
            break;
    }
}

int main(int argc, char** argv) {
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

    test_exec(val);

    return 0;
}