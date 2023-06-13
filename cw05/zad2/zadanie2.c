#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

double func(double x) {
    return 4/(x*x + 1);
}

double integrate(double a, double b, double dx) {
    double sum = 0;
    for (double x = a; x < b; x += dx) {
        sum += func(x)*dx;
    }
    return sum;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Incorrect arguments!\n");
    }
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start_time);


    pid_t pid;
    size_t size;
    int i;
    double dx = strtod(argv[1], NULL);
    int n = atoi(argv[2]);
    double block_width = 1.0/n;
    double total = 0.0, tmp_result;

    int pipes[n][2];

    for (i = 0; i < n; i++) {
        pipe(pipes[i]);
    }

    char* write_buff = calloc(256, sizeof (char));

    for (i = 0; i < n; i++) {
        pid = fork();
        if (pid == 0) {
            close(pipes[i][0]);
            tmp_result = integrate(block_width * i, block_width * (i + 1), dx);
            size = snprintf(write_buff, 256, "%lf", tmp_result);
            write(pipes[i][1], write_buff, size);
            return 0;
        }
    }
    while (wait(NULL) > 0);

    char* read_buff = calloc(256, sizeof(char));
    for (i = 0; i < n; i++) {
        close(pipes[i][1]);
        size = read(pipes[i][0], read_buff, 256);
        read_buff[size] = 0;
        total += strtod(read_buff, NULL);
    }
    free(write_buff);
    free(read_buff);

    clock_gettime(CLOCK_REALTIME, &end_time);
    double total_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec)/pow(10,9);
    printf("RESULT: %lf, DX: %.10f n-processes: %d, TOTAL TIME: %f\n", total, dx, n, total_time);

    return 0;

}