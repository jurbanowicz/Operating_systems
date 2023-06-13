#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define N_REINDEERS 9
#define N_ELVES 10

pthread_mutex_t santa_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t elf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reindeer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t delivery_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t santa_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t reindeer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t elves_cond = PTHREAD_COND_INITIALIZER;

void *santa_func(void *arg);
void *reindeer_func(void *arg);
void *elf_func(void * arg);

int reindeer_returned = 0;
int elves_waiting = 0;
int deliveries_made = 0;


int main(void) {
    srand(time(NULL));
    pthread_t santa_thread, reindeer_threads[N_REINDEERS], elf_threads[N_ELVES];

    pthread_create(&santa_thread, NULL, santa_func, NULL);

    for (int i = 0; i < N_REINDEERS; i++) {
        int *id = malloc(sizeof (int));
        *id = i + 1;
        pthread_create(&reindeer_threads[i], NULL, reindeer_func, (void*)id);
    }

    for (int i = 0; i < N_ELVES; i++) {
        int *id = malloc(sizeof (int));
        *id = i + 1;
        pthread_create(&elf_threads[i], NULL, elf_func, (void*)id);
    }

    for (int i = 0; i < N_REINDEERS; i++) {
        pthread_join(reindeer_threads[i], NULL);
    }
    for (int i = 0; i < N_ELVES; i++) {
        pthread_join(elf_threads[i], NULL);
    }
    pthread_join(santa_thread, NULL);

    return 0;
}

void *santa_func(void* arg) {
    while (deliveries_made < 3) {
        pthread_mutex_lock(&santa_mutex);
        while (!(reindeer_returned == N_REINDEERS || (elves_waiting >= 3))) {
            printf("Santa: Going to sleep...\n");
//            printf("Reindeer waiting %d, elves waiting %d, deliveries made %d \n", reindeer_returned, elves_waiting, deliveries_made);
            pthread_cond_wait(&santa_cond, &santa_mutex);
        }
        if (reindeer_returned == N_REINDEERS) {
            printf("Santa: Waking up...\n");
            printf("Santa: Delivering presents!\n");
            sleep(rand() % 3 + 2);
            reindeer_returned = 0;
            printf("Santa: All presents delivered\n");
            pthread_mutex_lock(&delivery_mutex);
            deliveries_made++;
            pthread_mutex_unlock(&delivery_mutex);
            pthread_cond_broadcast(&reindeer_cond);
        }
        else if (elves_waiting == 3) {
            printf("Santa: Waking up...\n");
            printf("Santa: solving elves problems\n");
            sleep(rand() % 2 + 1);
            elves_waiting -= 3;
            pthread_cond_broadcast(&elves_cond);
        }

        pthread_mutex_unlock(&santa_mutex);
    }

    printf("Santa: All jobs done!\n");
    pthread_cond_broadcast(&reindeer_cond);
    pthread_cond_broadcast(&elves_cond);
    pthread_exit(NULL);
}

void *reindeer_func(void *arg) {
    int id = *((int*)arg);
    while (1) {
        pthread_mutex_lock(&delivery_mutex);
        if (deliveries_made == 3) {break;}
        pthread_mutex_unlock(&delivery_mutex);
        sleep(rand() % 6 + 5);

        pthread_mutex_lock(&santa_mutex);
        pthread_mutex_lock(&reindeer_mutex);
        printf("Reindeer: %d reindeer back from holidays, %d reindeers waiting for santa\n", id, ++reindeer_returned);
        pthread_mutex_unlock(&reindeer_mutex);
        if (reindeer_returned == N_REINDEERS) {
            printf("Reindeer: %d, waking up santa\n", id);
            pthread_cond_signal(&santa_cond);
        } else {
            pthread_cond_wait(&reindeer_cond, &santa_mutex);
        }

        pthread_mutex_unlock(&santa_mutex);
//        pthread_cond_wait(&reindeer_cond, &santa_mutex);
    }
    pthread_exit(NULL);
}

void *elf_func(void *arg) {
    int id = *((int*)arg);

    while (1) {
        pthread_mutex_lock(&delivery_mutex);
        if (deliveries_made == 3) {break;}
        pthread_mutex_unlock(&delivery_mutex);
        sleep(rand() % 4 + 2);

        pthread_mutex_lock(&santa_mutex);
        pthread_mutex_lock(&elf_mutex);

        if (elves_waiting < 3) {
            printf("Elf: %d has a problem, %d elves waiting for santa\n", id, ++elves_waiting);

            if (elves_waiting == 3) {
                printf("Elf: %d, waking up santa\n", id);
                pthread_cond_signal(&santa_cond);
            }
            pthread_mutex_unlock(&elf_mutex);
            pthread_mutex_unlock(&santa_mutex);
            pthread_cond_wait(&elves_cond, &santa_mutex);

        }
        else {
            printf("Elf: %d, resolving the issue by himself\n", id);
            pthread_mutex_unlock(&elf_mutex);
            pthread_mutex_unlock(&santa_mutex);
            continue;
        }
        pthread_mutex_unlock(&elf_mutex);
        pthread_mutex_unlock(&santa_mutex);
    }

    pthread_exit(NULL);
}