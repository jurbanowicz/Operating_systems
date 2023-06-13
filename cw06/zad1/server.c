#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/ipc.h>

#include "common.h"

int first_free_id = 0;
int queue_id;
key_t client_queue[CLIENTS_LIMIT];
MessageBuffer server_buffer;

void handle_init();

void handle_list(int client_id);

void handle_to_all();

void handle_to_one();

void handle_stop(int client_id);

void save_message();

void handle_server_end();


int main(void) {

    for (int i = 0; i < CLIENTS_LIMIT; i++) {
        client_queue[i] = -1;
    }

    char cwd[256];
    getcwd(cwd, sizeof (cwd));

    key_t queue_key = ftok(cwd, SERVER_ID);
    queue_id = msgget(queue_key, IPC_CREAT | 0600);

    signal(SIGINT, handle_server_end);

    printf("SERVER INITIALIZED...\n");

    while (1) {
        msgrcv(queue_id, &server_buffer, MESSAGE_SIZE, -6, 0);

        printf("CURRENT MESSAGE %ld\n", server_buffer.type);

        switch(server_buffer.type) {
            case INIT:
                handle_init();
                save_message();
                break;
            case LIST:
                handle_list(server_buffer.client_id);
                save_message();
                break;
            case TOALL:
                handle_to_all();
                save_message();
                break;
            case TOONE:
                handle_to_one();
                save_message();
                break;
            case STOP:
                handle_stop(server_buffer.client_id);
                save_message();
                break;
            default:
                printf("Incorrect message type!\n");
        }
    }
}

void handle_init() {
    while (client_queue[first_free_id] != -1 && first_free_id < CLIENTS_LIMIT - 1) {
        first_free_id++;
    }

    if (client_queue[first_free_id] != -1 && first_free_id == CLIENTS_LIMIT - 1) {
        server_buffer.client_id = -1;
    }
    else {
        server_buffer.client_id = first_free_id;
        client_queue[first_free_id] = server_buffer.queue_key;

        if (first_free_id < CLIENTS_LIMIT - 1) {
            first_free_id++;
        }
    }
    int client_queue_id = msgget(server_buffer.queue_key, 0600);
    msgsnd(client_queue_id, &server_buffer, sizeof (server_buffer), 0);
    save_message();
}

void handle_list(int client_id) {
    MessageBuffer *messageBuffer = malloc(sizeof (MessageBuffer));
    strcpy(messageBuffer->content, "");

    for (int i = 0; i < CLIENTS_LIMIT; i++) {
        if (client_queue[i] != -1) {
//            sprintf(messageBuffer->content + strlen(messageBuffer->content), "Client: %d is active\n", i);
            char tmp[256];
            sprintf(tmp, "Client: %d is active\n", i);
            strcat(messageBuffer->content, tmp);
        }

    }

    messageBuffer->type = LIST;
    int client_queue_id = msgget(client_queue[client_id], 0);
    msgsnd(client_queue_id, messageBuffer, MESSAGE_SIZE, 0);
}

void handle_to_all() {
    for (int i = 0; i < CLIENTS_LIMIT; i++) {
        if (client_queue[i] != -1 && server_buffer.client_id != i) {
            int other_client_id = msgget(client_queue[i], 0);
            msgsnd(other_client_id, &server_buffer, MESSAGE_SIZE, 0);
        }
    }
}

void handle_to_one() {
    if (client_queue[server_buffer.other_id] != -1) {
        int other_client_id = msgget(client_queue[server_buffer.other_id], 0);
        msgsnd(other_client_id, &server_buffer, MESSAGE_SIZE, 0);
    }
}

void handle_stop(int client_id) {
    client_queue[client_id] = -1;
    if (client_id < first_free_id) {
        first_free_id = client_id;
    }
}

void save_message() {
    struct tm time = server_buffer.time_struct;

    FILE *f = fopen("message_logs.txt", "a");

    switch (server_buffer.type) {
        case INIT:
            if (server_buffer.client_id == -1) {
                fprintf(f, "INIT -> Clients limit reached\n");
            } else {
                fprintf(f, "INIT -> Client id: %d\n", server_buffer.client_id);
            }
            break;
        case LIST:
            fprintf(f, "LIST -> Client id: %d\n", server_buffer.client_id);
            break;
        case TOALL:
            fprintf(f, "2ALL -> Sender id: %d\n", server_buffer.client_id);
            fprintf(f, "2ALL -> Message: %s\n", server_buffer.content);
            break;
        case TOONE:
            fprintf(f, "2ONE -> Sender id: %d, Receiver id: %d\n", server_buffer.client_id, server_buffer.other_id);
            fprintf(f, "2ONE -> Message: %s\n", server_buffer.content);
            break;
        case STOP:
            fprintf(f, "STOP -> Client id: %d\n", server_buffer.client_id);
            break;
    }
    fprintf(f, "Message sent at: %02d:%02d:%02d\n", time.tm_hour, time.tm_min, time.tm_sec);

    fclose(f);
}


void handle_server_end() {

    for (int i = 0; i < CLIENTS_LIMIT; i++) {
        if (client_queue[i] != -1) {
            server_buffer.type = STOP;
            int client_queue_id = msgget(client_queue[i], 0600);
            msgsnd(client_queue_id, &server_buffer, MESSAGE_SIZE, 0);
            msgrcv(queue_id, &server_buffer, MESSAGE_SIZE, STOP, 0);
        }
    }

    msgctl(queue_id, IPC_RMID, NULL);
    printf("SERVER CLOSING\n");
    exit(0);
}