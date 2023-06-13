#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_PATH_LEN 512

int queue_n, barber_n, chairs_n;
sem_t ** queue_id, ** barber_id, ** chair_id;
int available_queue_places_id, available_barbers_id, available_chairs_id, first_client_from_queue_id;

int get_haircut_time() {
    return (rand() % 9) + 2;
}

void init_semaphores(int n, sem_t **sem_list, char* name);

void free_all_semaphores();

void set_shared_mem_val(int mem_id, int value);

int get_shared_mem_val(int mem_id);

void add_new_client(int new_client_id);

int queue_client();

void dequeue_client();

void free_semaphore(sem_t *sem_id, int memory_id);

int find_free_semaphore(sem_t **id, int len, int memory_id);

int handle_haircut(int client_id);

int main(int argc, char** argv) {
    if (argc != 4) {
        fprintf(stderr, "Incorrect argument count!\n");
        return 1;
    }

    char cwd[MAX_PATH_LEN];
    if (getcwd(cwd, MAX_PATH_LEN) == 0) {
        perror("Error loading current directory\n");
        return 1;
    }

    queue_n = atoi(argv[1]);
    barber_n = atoi(argv[2]);
    chairs_n = atoi(argv[3]);

    if (queue_n < 0 || barber_n < 0 || chairs_n < 0) {
        fprintf(stderr, "Arguments cannot be less than 0!\n");
        return 1;
    }

    queue_id = calloc(queue_n, sizeof (sem_t *));
    barber_id = calloc(barber_n, sizeof (sem_t *));
    chair_id = calloc(chairs_n, sizeof (sem_t *));

    init_semaphores(queue_n, queue_id, "queue");
    init_semaphores(barber_n, barber_id, "barbers");
    init_semaphores(chairs_n, chair_id, "chairs");

    available_queue_places_id = shm_open("/available_places_in_queue", O_CREAT | O_RDWR, 0644);
    ftruncate(available_queue_places_id, sizeof(int));
    available_barbers_id = shm_open("/available_barbers", O_CREAT | O_RDWR, 0644);
    ftruncate(available_barbers_id, sizeof(int));
    available_chairs_id = shm_open("/available_chairs", O_CREAT | O_RDWR, 0644);
    ftruncate(available_chairs_id, sizeof(int));

    set_shared_mem_val(available_queue_places_id, queue_n);
    set_shared_mem_val(available_barbers_id, barber_n);
    set_shared_mem_val(available_chairs_id, chairs_n);

    first_client_from_queue_id = shm_open("/first_client_from_queue", O_CREAT | O_RDWR, 0644 );
    ftruncate(first_client_from_queue_id, sizeof (int));
    set_shared_mem_val(first_client_from_queue_id, 0);

    char *curr_input = calloc(MAX_PATH_LEN, sizeof (char));

    pid_t pid;

    int curr_client_id = 0;

    printf("Press space to add clients \n");
    printf("Press enter to end the program \n");
    while (fgets(curr_input, MAX_PATH_LEN, stdin)) {
        if (strcmp(curr_input, "\n") == 0) {
            printf("Exiting program");
            break;
        }

        if (strcmp(curr_input, " \n") == 0) {
            pid = fork();
            if (pid == 0) {
                add_new_client(curr_client_id);
                return 0;
            }
            curr_client_id++;
        }
    }
    free(curr_input);

    while (wait(NULL) > 0);

    free_all_semaphores();

    return 0;
}

void init_semaphores(int n, sem_t** sem_list, char* name) {
    char curr_name[MAX_PATH_LEN];
    for (int i = 0; i < n; i++) {
        sprintf(curr_name, "%s_%d", name, i);
        sem_list[i] = sem_open(curr_name, O_CREAT, 0600, 1);
        int tmp;
        sem_getvalue(sem_list[i], &tmp);
    }
}

void free_all_semaphores() {
    for (int i = 0; i < queue_n; i++) {
        sem_close(queue_id[i]);
    }
    for (int i = 0; i < barber_n; i++) {
        sem_close(barber_id[i]);
    }
    for (int i = 0; i < chairs_n; i++) {
        sem_close(chair_id[i]);
    }
    shm_unlink("/available_places_in_queue");
    shm_unlink("/available_barbers");
    shm_unlink("/available_chairs");
    shm_unlink("/first_client_from_queue");
}

void set_shared_mem_val(int mem_id, int value) {
    int *p = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, mem_id, 0);
    *p = value;
    munmap(p, sizeof(int));
}

int get_shared_mem_val(int mem_id) {
    int tmp;
    int *p = mmap(NULL, sizeof(int), PROT_READ, MAP_SHARED, mem_id, 0);
    tmp = *p;
    munmap(p, sizeof(int));
    return tmp;
}

void add_new_client(int new_client_id) {
    int client_queue_idx, haircut_error;
    int tmp;

    printf("Client %d is entering hair saloon\n", new_client_id);

    if (get_shared_mem_val(available_queue_places_id) == 0) {
        printf("No spaces in queue available, client leaving...\n");
        return;
    }

    if (get_shared_mem_val(available_queue_places_id) == queue_n) {
        haircut_error = handle_haircut(new_client_id);
        printf("Haircut error: %d\n", haircut_error);
        if (haircut_error == 0) {
            if (get_shared_mem_val(available_queue_places_id) < queue_n) {
                dequeue_client();
            }
            return;
        }
    }

    if (get_shared_mem_val(available_queue_places_id) > 0) {
        client_queue_idx = queue_client();
        printf("Client %d is waiting in queue: %d\n", new_client_id, client_queue_idx);

        sem_getvalue(queue_id[client_queue_idx], &tmp);
        while (tmp==0){
            sem_getvalue(queue_id[client_queue_idx], &tmp);
        }
        printf("Client %d is out of queue\n", new_client_id);
        handle_haircut(new_client_id);

        if (get_shared_mem_val(available_queue_places_id) < queue_n) {
            dequeue_client();
        }
        return;

    }
}

int handle_haircut(int client_id) {
    int haircut_time, client_barber_id, client_chair_id;
    haircut_time = get_haircut_time();
    client_barber_id = find_free_semaphore(barber_id, barber_n, available_barbers_id);
    if (client_barber_id == -1) {
        return 1;
    }
    client_chair_id = find_free_semaphore(chair_id, chairs_n, available_chairs_id);
    if (client_chair_id == -1) {
        return 2;
    }
    printf("Client %d will be cut by barber %d in chair %d, time of haircut: %d\n", client_id, client_barber_id, client_chair_id, haircut_time);

    sleep(haircut_time);

    printf("Client %d haircut is done\n", client_id);
    free_semaphore(barber_id[client_barber_id], available_barbers_id);
    free_semaphore(chair_id[client_chair_id], available_chairs_id);
    return 0;
}

int queue_client() {
    int tmp;
    for (int i = get_shared_mem_val(first_client_from_queue_id); i < queue_n; i++) {
        sem_getvalue(queue_id[i], &tmp);
        if (tmp == 1) {
            set_shared_mem_val(available_queue_places_id, get_shared_mem_val(available_queue_places_id) - 1);
            sem_wait(queue_id[i]);
            return i;
        }
    }
    for (int i = 0; i < get_shared_mem_val(first_client_from_queue_id); i++) {
        sem_getvalue(queue_id[i], &tmp);
        if(tmp==1){
            set_shared_mem_val(available_queue_places_id, get_shared_mem_val(available_queue_places_id) - 1);
            sem_wait(queue_id[i]);
            return i;
        }
    }
    return -1;
}

void dequeue_client() {
    sem_post(queue_id[get_shared_mem_val(first_client_from_queue_id)]);
    set_shared_mem_val(available_queue_places_id, get_shared_mem_val(available_queue_places_id) + 1);
    set_shared_mem_val(first_client_from_queue_id, (get_shared_mem_val(first_client_from_queue_id) + 1) % queue_n);
}

void free_semaphore(sem_t *sem_id, int memory_id) {
    sem_post(sem_id);
    set_shared_mem_val(memory_id, get_shared_mem_val(memory_id) + 1);
}

int find_free_semaphore(sem_t **id, int len, int memory_id) {
    int tmp = 0;
    for (int i = 0; i < len; i++)
    {
        sem_getvalue(id[i], &tmp);
        if(tmp==1){
            set_shared_mem_val(memory_id, get_shared_mem_val(memory_id) - 1);
            sem_wait(id[i]);
            return i;
        }
    }
    return -1;
}

