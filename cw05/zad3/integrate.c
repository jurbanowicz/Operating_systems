#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

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

int main(int argc, char** argv) {
    if (argc != 4) {
        fprintf(stderr, "Invalid arguments1\n");
        return 1;
    }
    double dx = strtod(argv[1], NULL);
    double a = strtod(argv[2], NULL);
    double b = strtod(argv[3], NULL);
    double result = integrate(a, b, dx);

    char* write_buff = calloc(256, sizeof (char));
    size_t size = snprintf(write_buff, 256, "%lf\n", result);

    int fifo = open("results", O_WRONLY);
    write(fifo, write_buff, size);

    close(fifo);
    free(write_buff);
    return 0;
}