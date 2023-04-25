#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/msg.h>

#include "common.h"

key_t queue_key;
int client_queue_id;
int server_queue_id;
int client_id;

int handle_init();
void handle_list();
void handle_to_all(char* message);
void handle_to_one(char* message, int other_id);
void handle_stop();
void handle_server_message();

int main(void) {

    srand(time(NULL));

    queue_key = ftok(HOME_PATH, rand() % 255 + 1);
    client_queue_id = msgget(queue_key, IPC_CREAT | 0666);
    key_t server_key = ftok(HOME_PATH, SERVER_ID);
    server_queue_id = msgget(server_key, 0);
    client_id = handle_init();

    signal(SIGINT, handle_stop);

    size_t len = 0;
    size_t read;
    char *command = NULL;

    printf("Client initialized, id: %d\n", client_id);
    while (1) {
        printf("Enter command: ");
        read = getline(&command, &len, stdin);
        command[read - 1] = '\0';

        handle_server_message();

        if (strcmp(command, "") == 0) {
            printf("No command entered!\n");
            continue;
        }


        char *current_command = strtok(command, " ");

        printf("CURRENT COMMAND: %s\n", current_command);
        if (strcmp(current_command, "LIST") == 0) {
            handle_list();
        }
        else if (strcmp(current_command, "2ALL") == 0) {
            current_command = strtok(NULL, " ");
            char* message = current_command;
            handle_to_all(message);
        } else if (strcmp(current_command, "2ONE") == 0) {
            current_command = strtok(NULL, " ");
            int other_id = atoi(current_command);
            current_command = strtok(NULL, " ");
            char* message = current_command;
            handle_to_one(message, other_id);
        } else if(strcmp(current_command, "STOP") == 0) {
            handle_stop();
        } else {
            printf("Incorrect command!\n");
        }
    }
}

int handle_init() {
    time_t current_time = time(NULL);
    MessageBuffer *messageBuffer = malloc(sizeof (MessageBuffer));

    messageBuffer->time_struct = *localtime(&current_time);
    messageBuffer->type = INIT;
    messageBuffer->queue_key = queue_key;

    msgsnd(server_queue_id, messageBuffer, MESSAGE_SIZE, 0);
    msgrcv(client_queue_id, messageBuffer, MESSAGE_SIZE, 0, 0);

    int current_id = messageBuffer->client_id;
    if (current_id == -1) {
        printf("Client limit reached, exiting! \n");
        exit(0);
    }

    return current_id;
}
void handle_list() {
    time_t current_time = time(NULL);
    MessageBuffer *messageBuffer = malloc(sizeof (MessageBuffer));

    messageBuffer->time_struct = *localtime(&current_time);
    messageBuffer->type = LIST;
    messageBuffer->client_id = client_id;

    msgsnd(server_queue_id, messageBuffer, MESSAGE_SIZE, 0);
    msgrcv(client_queue_id, messageBuffer, MESSAGE_SIZE, 0, 0);

    printf("Message from server: %s\n", messageBuffer->content);
}
void handle_to_all(char* message) {
    time_t current_time = time(NULL);
    MessageBuffer *messageBuffer = malloc(sizeof (MessageBuffer));

    messageBuffer->time_struct = *localtime(&current_time);
    messageBuffer->type = TOALL;
    messageBuffer->client_id = client_id;
    strcpy(messageBuffer->content, message);

    msgsnd(server_queue_id, messageBuffer, MESSAGE_SIZE, 0);
}
void handle_to_one(char* message, int other_id) {
    time_t current_time = time(NULL);
    MessageBuffer *messageBuffer = malloc(sizeof (MessageBuffer));

    messageBuffer->time_struct = *localtime(&current_time);
    messageBuffer->type = TOONE;
    messageBuffer->client_id = client_id;
    messageBuffer->other_id = other_id;
    strcpy(messageBuffer->content, message);

    msgsnd(server_queue_id, messageBuffer, MESSAGE_SIZE, 0);
}
void handle_stop() {
    time_t current_time = time(NULL);
    MessageBuffer *messageBuffer = malloc(sizeof (MessageBuffer));

    messageBuffer->time_struct = *localtime(&current_time);
    messageBuffer->type = STOP;
    messageBuffer->client_id = client_id;

    msgsnd(server_queue_id, messageBuffer, MESSAGE_SIZE, 0);
    msgctl(client_queue_id, IPC_RMID, NULL);
    exit(0);
}
void handle_server_message() {
    MessageBuffer *messageBuffer = malloc(sizeof (MessageBuffer));

    while (msgrcv(client_queue_id, messageBuffer, MESSAGE_SIZE, 0 , IPC_NOWAIT) >= 0) {
        if (messageBuffer->type == STOP) {
            printf("Received STOP message from server, exiting\n");
            handle_stop();
        } else {

            struct tm current_time = messageBuffer->time_struct;
            printf("Message received from: %d, sent at: %02d:%02d:%02d:\n%s\n",
                   messageBuffer->client_id,
                   current_time.tm_hour,
                   current_time.tm_min,
                   current_time.tm_sec,
                   messageBuffer->content);
        }
    }
}

