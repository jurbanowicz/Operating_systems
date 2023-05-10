#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <stdbool.h>
#include "grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void handler() {}

int main(int argc, char** argv)
{
    if (argc != 2) {
        fprintf(stderr, "Invalid arguments\n");
    }
    int n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "Number of thread must be greater than 0!\n");
    }

	srand(time(NULL));
	setlocale(LC_CTYPE, "");
	initscr(); // Start curses mode

    signal(SIGUSR1, handler);

	char *foreground = create_grid();
	char *background = create_grid();
	char *tmp;

	init_grid(foreground);
    init_threads(n, foreground, background);

	while (true)
	{
		draw_grid(foreground);
		usleep(500 * 1000);

		// Step simulation
		update_grid(foreground, background);
		tmp = foreground;
		foreground = background;
		background = tmp;
	}

	endwin(); // End curses mode
	destroy_grid(foreground);
	destroy_grid(background);

	return 0;
}
