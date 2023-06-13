#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


int main(void) {
    pid_t pid;
    int fd[2];
    char buf[256];
    int w;

    pipe(fd);
    pid = fork();
    if (pid == 0) {
        close(fd[0]);
        w = write(fd[1], "1234567890", 11);
        sleep(5);
        return 0;
    }

    close(fd[1]);
    w = read(fd[0], buf, 11);
    printf("%s\n", buf);
    return 0;
}