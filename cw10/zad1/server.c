#include "common.h"

#define MAX_CLIENTS 16
#define PING_INTERVAL 10

static pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t client_activity_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t client_no_mutex = PTHREAD_MUTEX_INITIALIZER;

static int next_id = 0;
static int server_close = 0;
static int socket_1;
int port;
static int clients[MAX_CLIENTS];
static int client_activity[MAX_CLIENTS];
static int clients_no = 0;
FILE *message_history;

int create_server_socket(int type, in_addr_t addr_t)
{
    int main_socket;
    struct sockaddr_in server;
    memset(&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = addr_t;

    main_socket = socket(AF_INET, type, 0);
    if (main_socket == -1)
    {
        fprintf(stderr, "Error creating socket\n");
        return -1;
    }
    int status = bind(main_socket, (struct sockaddr *)&server, sizeof server);

    if (status == -1)
    {
        fprintf(stderr, "Error binding socket\n");
        return -2;
    }
    return main_socket;
}

void init_clients_arrays() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = -1;
        client_activity[i] = 0;
    }
}

void init_client(int client_id) {
    Message msg;
    msg.msg_type = INIT;
    msg.client_id = client_id;
    pthread_mutex_lock(&client_mutex);
    write(clients[client_id], &msg, sizeof(msg));
    pthread_mutex_unlock(&client_mutex);
}

void handle_sigint(int signo) {
    Message msg;
    msg.msg_type = STOP;
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
        write(clients[i], &msg, sizeof(msg));
        }
    }
    pthread_mutex_unlock(&client_mutex);
    printf("Server closing...\n");
    exit(0);
}

void *ping_clients(void *arg) {
    while (1) {
        sleep(PING_INTERVAL);
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            client_activity[i] = 0;
        }
        Message msg;
        msg.msg_type = PING;
        pthread_mutex_lock(&client_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
        if (clients[i] != -1) {
            write(clients[i], &msg, sizeof(msg));
        }
        }
        pthread_mutex_unlock(&client_mutex);
        sleep(PING_INTERVAL);

        msg.msg_type = STOP;
        pthread_mutex_lock(&client_mutex);
        pthread_mutex_lock(&client_activity_mutex);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if(clients[i] != -1 && client_activity[i] == 0) {
                pthread_mutex_lock(&client_no_mutex);
                clients_no--;
                pthread_mutex_unlock(&client_no_mutex);
                clients[i] = -1;
                write(clients[i], &msg, sizeof(msg));
                printf("Client %d has been disconnected - not responding\n", i);
            }
        }
        pthread_mutex_unlock(&client_activity_mutex);
        pthread_mutex_unlock(&client_mutex);
        pthread_mutex_lock(&client_no_mutex);
        if (clients_no == 0) {
        pthread_mutex_unlock(&client_no_mutex);
        exit(0);
        }
        pthread_mutex_unlock(&client_no_mutex);
    }
}

void write_to_file(int client_id, MessageType type) {
    time_t curr_time = time(NULL);
    struct tm loc_time = *localtime(&curr_time);
    char buff[MAX_MESSAGE_SIZE / 2], to_save[MAX_MESSAGE_SIZE];
    snprintf(buff, MAX_MESSAGE_SIZE / 2, "%02d:%02d:%02d", loc_time.tm_hour, loc_time.tm_min, loc_time.tm_sec);
    snprintf(to_save, MAX_MESSAGE_SIZE, "ClientID: %d, type: %s, time: %s\n", client_id, get_message_type(type), buff);
    fwrite(to_save, sizeof(char), strlen(to_save), message_history);
}

void send_message_to_client(int id, MessageType type, char *text, int sender_id, char* client_name)
{
    Message msg;
    time_t curr_time = time(NULL);
    struct tm loc_time = *localtime(&curr_time);
    snprintf(msg.date, MAX_MESSAGE_SIZE, "%02d:%02d:%02d", loc_time.tm_hour, loc_time.tm_min, loc_time.tm_sec);
    msg.msg_type = type;
    msg.from_id = sender_id;
    msg.to_id = id;
    strcpy(msg.client_name, client_name);
    strcpy(msg.msg_text, text);
    pthread_mutex_lock(&client_mutex);
    write(clients[id], &msg, sizeof(msg));
    pthread_mutex_unlock(&client_mutex);
}


void *client_handler(void *arg) {
    pthread_mutex_lock(&client_no_mutex);
    clients_no++;
    pthread_mutex_unlock(&client_no_mutex);
    ClientThreadArgs *args = (ClientThreadArgs *)arg;
    int id =  args->client_id;
    int client_socket = args->socket_no;
    int client_close = 0;
    Message msg;
    while (!client_close) {
        read(client_socket, &msg, sizeof(Message));
        write_to_file(id, msg.msg_type);
        switch (msg.msg_type) {
        case STOP:
            pthread_mutex_lock(&client_mutex);
            clients[id] = -1;
            pthread_mutex_unlock(&client_mutex);
            client_close = 1;
            printf("User %d disconnected from the server - logged out\n", id);
            break;  

        case LIST:
            pthread_mutex_lock(&client_mutex);
            printf("List of clinets: \n");
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] != -1) {
                    printf("Client %d\n", i);
                }
            }
            pthread_mutex_unlock(&client_mutex);
            break;

        case TO_ONE:
            pthread_mutex_lock(&client_mutex);
            if (clients[msg.to_id] != -1)
            {
                pthread_mutex_unlock(&client_mutex);
                send_message_to_client(msg.to_id, TO_ONE, msg.msg_text, id, msg.client_name);
            }
            else
            {
                pthread_mutex_unlock(&client_mutex);
            }
            break;
        case TO_ALL:
            for (int i = 0; i < MAX_CLIENTS; i++) {
                pthread_mutex_lock(&client_mutex);
                if (clients[i] != -1 && i != id) {
                    pthread_mutex_unlock(&client_mutex);
                    send_message_to_client(i, TO_ALL, msg.msg_text, id, msg.client_name);
                } else {
                    pthread_mutex_unlock(&client_mutex);
                }
            }
            break;

        case PING:
            pthread_mutex_lock(&client_activity_mutex);
            client_activity[msg.from_id] = 1;
            pthread_mutex_unlock(&client_activity_mutex);
            break;

        default:
            break;
        }

    }
}


void my_exit() {
    fclose(message_history);
    close(socket_1);
}

int main(int argc, char** argv)
{
    if (argc != 2){
        printf("Incorrect arguments\n");
        return 1;
    }
    port = atoi(argv[1]);
    if(port == 0) {
        printf("port must be a numebr\n");
        return 1;
    }

    atexit(my_exit);
    init_clients_arrays();

    signal(SIGINT, handle_sigint);
    int status, socket_2;
    pthread_t client_pinger;
    message_history = fopen("logs.txt", "w");

    socket_1 = create_server_socket(SOCK_STREAM, htonl(INADDR_ANY));

    status = listen(socket_1, 10);
    if (status == -1) {
        fprintf(stderr, "Error while creating queue\n");
        exit(1);
    }

    pthread_create(&client_pinger, NULL, ping_clients, (void *)NULL );

    while (!server_close) {
        socket_2 = accept(socket_1, NULL, NULL);

        if (socket_2 == -1) {
        fprintf(stderr, "Error accepting\n");
        exit(1);
        }
        if (next_id == 16) {
        close(socket_2);
        continue;
        } 
        pthread_mutex_lock(&client_mutex);
        clients[next_id] = socket_2;
        printf("Client %d connected to the server\n", next_id);
        pthread_mutex_unlock(&client_mutex);
        init_client(next_id);

        pthread_t client_thread;
        ClientThreadArgs args = {next_id, socket_2};
        pthread_create(&client_thread, NULL, client_handler, (void *)&args );
        next_id++;
    }
    exit(0);
    return 0;
}

