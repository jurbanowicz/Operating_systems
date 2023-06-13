#include "grid.h"
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>

const int grid_width = 30;
const int grid_height = 30;
static pthread_t *threads;

char *create_grid()
{
    return malloc(sizeof(char) * grid_width * grid_height);
}

void destroy_grid(char *grid)
{
    free(grid);
    free(threads);
}

void draw_grid(char *grid)
{
    for (int i = 0; i < grid_height; ++i)
    {
        // Two characters for more uniform spaces (vertical vs horizontal)
        for (int j = 0; j < grid_width; ++j)
        {
            if (grid[i * grid_width + j])
            {
                mvprintw(i, j * 2, "■");
                mvprintw(i, j * 2 + 1, " ");
            }
            else
            {
                mvprintw(i, j * 2, " ");
                mvprintw(i, j * 2 + 1, " ");
            }
        }
    }

    refresh();
}

void init_grid(char *grid)
{
    for (int i = 0; i < grid_width * grid_height; ++i)
        grid[i] = rand() % 2 == 0;
}

bool is_alive(int row, int col, char *grid)
{

    int count = 0;
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            if (i == 0 && j == 0)
            {
                continue;
            }
            int r = row + i;
            int c = col + j;
            if (r < 0 || r >= grid_height || c < 0 || c >= grid_width)
            {
                continue;
            }
            if (grid[grid_width * r + c])
            {
                count++;
            }
        }
    }

    if (grid[row * grid_width + col])
    {
        if (count == 2 || count == 3)
            return true;
        else
            return false;
    }
    else
    {
        if (count == 3)
            return true;
        else
            return false;
    }
}

void update_grid(char *src, char *dst)
{
    for (int i = 0; i < grid_height; ++i)
    {
        for (int j = 0; j < grid_width; ++j)
        {
            pthread_kill(threads[i*grid_width + j], SIGUSR1);
        }
    }
}
void *update_single_cell(void * args) {
    thread_args *t_args = (thread_args *) args;
    char *tmp;
    while (1) {
        for (int i = 0; i < t_args->fields_n; i++) {
            int curr_row = t_args->row + i % grid_height;
            int curr_col = t_args->col + i / grid_height;
            t_args->dst[curr_row* grid_width + curr_col] = is_alive(curr_row, curr_col, t_args->src);
            tmp = t_args->src;
            t_args->src = t_args->dst;
            t_args->dst = tmp;
        }
        pause();
    }
}

void init_threads(int n, char* src, char*dst) {
    int grid_len = grid_width * grid_height;
    if (n == -1 || n > grid_len) {
        n = grid_len;
    }
    threads = malloc(n * sizeof (pthread_t));

    int fields_for_thread = grid_len / n;
    int leftover = grid_len % n;

    for (int i = 0; i < grid_len - leftover; i += fields_for_thread) {
        thread_args *args = malloc(sizeof (thread_args));
        int row = i % grid_height;
        int col = i / grid_height;
        args->row = row;
        args->col = col;
        args->fields_n = fields_for_thread;
        args->src = src;
        args->dst = dst;
    }
    for (int i = grid_len - leftover; i < grid_len; i += (fields_for_thread + 1)) {
        thread_args *args = malloc(sizeof (thread_args));
        int row = i % grid_height;
        int col = i % grid_width;
        args->row = row;
        args->col = col;
        args->fields_n = fields_for_thread + 1;
        args->src = src;
        args->dst = dst;
    }
}
