#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <string.h>

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Ivalid arguments!\n");
        return 1;
    }
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start_time);

    pid_t pid;
    double dx = strtod(argv[1], NULL);
    int n = atoi(argv[2]);
    double block_width = 1.0/n;
    double total = 0.0;

    mkfifo("results", 0666);

    for (int i = 0; i < n; i++) {
        pid = fork();
        if (pid == 0) {
            char* current_a = calloc(256, sizeof (char));
            char* current_b = calloc(256, sizeof (char));
            snprintf(current_a, 256, "%lf", i*block_width);
            snprintf(current_b, 256, "%lf", (i+1)*block_width);
            execl("./integrate", "integrate", argv[1], current_a, current_b, NULL);
            free(current_a);
            free(current_b);
            return 0;
        }
    }

    int total_read = 0;

    char* read_buff = calloc(2048, sizeof (char));
    int fifo = open("results", O_RDONLY);

    while (total_read < n) {
        size_t size = read(fifo, read_buff, 2048);
        read_buff[size] = 0;
        char delim[] = "\n";
        char *tmp_results;
        tmp_results = strtok(read_buff, delim);

        for (; tmp_results; tmp_results = strtok(NULL, delim)) {
            total += strtod(tmp_results, NULL);
            total_read++;
        }
    }


    clock_gettime(CLOCK_REALTIME, &end_time);
    double total_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec)/pow(10,9);
    printf("RESULT: %lf, DX: %.10f n-programs: %d, TOTAL TIME: %f\n", total, dx, n, total_time);

    free(read_buff);
    remove("results");

    return 0;
}