#pragma once
#include <stdbool.h>

typedef struct thread_args {
    int row;
    int col;
    int fields_n;
    char* src;
    char* dst;
}thread_args;

char *create_grid();
void destroy_grid(char *grid);
void draw_grid(char *grid);
void init_grid(char *grid);
bool is_alive(int row, int col, char *grid);
void update_grid(char *src, char *dst);
void init_threads(int n, char* src, char* dst);
void *update_single_cell(void * args);