#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>

#define MAX_PATH_LEN 512

int queue_n, barber_n, chairs_n;
int queue_id, barber_id, chair_id;
int available_queue_places_id, available_barbers_id, available_chairs_id, first_client_from_queue_id;

int get_haircut_time() {
    return (rand() % 9) + 2;
}

void init_semaphores(int n, int sem_id);

void free_all_semaphores();

void set_shared_mem_val(int mem_id, int value);

int get_shared_mem_val(int mem_id);

void add_new_client(int new_client_id);

int queue_client();

void dequeue_client();

void free_semaphore(int id, int sem_id, int memory_id);

int find_free_semaphore(int id, int len, int memory_id);

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

    key_t queue_key, barber_key, chair_key;

    key_t available_queue_places, available_barbers, available_chairs;
    key_t first_client_from_queue_key;

    queue_n = atoi(argv[1]);
    barber_n = atoi(argv[2]);
    chairs_n = atoi(argv[3]);

    if (queue_n < 0 || barber_n < 0 || chairs_n < 0) {
        fprintf(stderr, "Arguments cannot be less than 0!\n");
        return 1;
    }

    queue_key = ftok(cwd, 0);
    barber_key = ftok(cwd, 1);
    chair_key = ftok(cwd, 2);

    queue_id = semget(queue_key, queue_n, IPC_CREAT | 0600);
    barber_id = semget(barber_key, barber_n, IPC_CREAT | 0600);
    chair_id = semget(chair_key, chairs_n, IPC_CREAT | 0600);

    init_semaphores(queue_n, queue_id);
    init_semaphores(barber_n, barber_id);
    init_semaphores(chairs_n, chair_id);

    available_queue_places = ftok(cwd, 3);
    available_barbers = ftok(cwd, 4);
    available_chairs = ftok(cwd, 5);

    available_queue_places_id = shmget(available_queue_places, sizeof (int), IPC_CREAT|0600);
    available_barbers_id = shmget(available_barbers, sizeof (int), IPC_CREAT|0600);
    available_chairs_id = shmget(available_chairs, sizeof (int), IPC_CREAT|0600);

    set_shared_mem_val(available_queue_places_id, queue_n);
    set_shared_mem_val(available_barbers_id, barber_n);
    set_shared_mem_val(available_chairs_id, chairs_n);

    first_client_from_queue_key = ftok(cwd, 6);
    first_client_from_queue_id = shmget(first_client_from_queue_key, sizeof (int), IPC_CREAT|0600);
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

void init_semaphores(int n, int sem_id) {
    for (int i = 0; i < n; i++) {
        if (semctl(sem_id, i, SETVAL, 1) == -1) {
            perror("Error initializing semaphore\n");
        }
    }
}

void free_all_semaphores() {
    semctl(queue_id, 0, IPC_RMID);
    semctl(barber_id, 0, IPC_RMID);
    semctl(chair_id, 0, IPC_RMID);
    shmctl(available_queue_places_id, IPC_RMID, NULL);
    shmctl(available_barbers_id, IPC_RMID, NULL);
    shmctl(available_chairs_id, IPC_RMID, NULL);
    shmctl(first_client_from_queue_id, IPC_RMID, NULL);
}

void set_shared_mem_val(int mem_id, int value) {
    int *wsk = shmat(mem_id, NULL, 0);
    *wsk = value;
    shmdt(wsk);
}

int get_shared_mem_val(int mem_id) {
    int tmp;
    int* wsk = shmat(mem_id, NULL, 0);
    tmp = *wsk;
    shmdt(wsk);

    return tmp;
}

void add_new_client(int new_client_id) {
    int client_queue_idx, haircut_error;

    printf("Client %d is entering hair saloon\n", new_client_id);

    if (get_shared_mem_val(available_queue_places_id) == 0) {
        printf("No spaces in queue available, client leaving...\n");
        return;
    }

    if (get_shared_mem_val(available_queue_places_id) == queue_n) {
        haircut_error = handle_haircut(new_client_id);
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
        while (semctl(queue_id, client_queue_idx, GETVAL, NULL)==0){}
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
    free_semaphore(barber_id, client_barber_id, available_barbers_id);
    free_semaphore(chair_id, client_chair_id, available_chairs_id);
    return 0;
}

int queue_client() {
    struct sembuf sem_buff;

    sem_buff.sem_flg = 0;
    sem_buff.sem_op = -1;

    for (int i = get_shared_mem_val(first_client_from_queue_id); i < queue_n; i++) {
        if(semctl(queue_id,i,GETVAL,NULL)==1){
            sem_buff.sem_num = i;
            set_shared_mem_val(available_queue_places_id, get_shared_mem_val(available_queue_places_id)-1);
            semop(queue_id, &sem_buff, 1);
            return i;
        }
    }
    for (int i = 0; i < get_shared_mem_val(first_client_from_queue_id); i++) {
        if(semctl(queue_id,i,GETVAL,NULL)==1){
            sem_buff.sem_num = i;
            set_shared_mem_val(available_queue_places_id, get_shared_mem_val(available_queue_places_id)-1);
            semop(queue_id, &sem_buff, 1);
            return i;
        }
    }
    return -1;
}

void dequeue_client() {
    struct sembuf sem_buff;
    sem_buff.sem_op = 1;
    sem_buff.sem_flg = 0;
    sem_buff.sem_num = get_shared_mem_val(first_client_from_queue_id);
    semop(queue_id, &sem_buff, 1);
    set_shared_mem_val(available_queue_places_id, get_shared_mem_val(available_queue_places_id)+1);
    set_shared_mem_val(first_client_from_queue_id, (get_shared_mem_val(first_client_from_queue_id)+1)%queue_n);
}

void free_semaphore(int id, int sem_id, int memory_id) {
    struct sembuf sem_buff;
    sem_buff.sem_op = 1;
    sem_buff.sem_flg = 0;
    sem_buff.sem_num = sem_id;
    semop(id, &sem_buff, 1);
    set_shared_mem_val(memory_id, get_shared_mem_val(memory_id)+1);
}

int find_free_semaphore(int id, int len, int memory_id) {
    struct sembuf sem_buff;
    sem_buff.sem_op = -1;
    sem_buff.sem_flg = 0;
    for (int i = 0; i < len; i++)
    {
        if(semctl(id,i,GETVAL,NULL)==1){
            sem_buff.sem_num = i;
            set_shared_mem_val(memory_id, get_shared_mem_val(memory_id)-1);
            semop(id, &sem_buff, 1);
            return i;
        }
    }
    return -1;
}

