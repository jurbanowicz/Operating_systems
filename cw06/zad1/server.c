#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/msg.h>

#include "common.h"

int first_free_id = 0;
int queue_id;
key_t client_queue[CLIENTS_LIMIT];

void handle_init(MessageBuffer *messageBuffer);

void handle_list(int client_id);

void handle_to_all(MessageBuffer *messageBuffer);

void handle_to_one(MessageBuffer *messageBuffer);

void handle_stop(int client_id);

void save_message(MessageBuffer *messageBuffer);

void handle_server_end();


int main(void) {

    for (int i = 0; i < CLIENTS_LIMIT; i++) {
        client_queue[i] = -1;
    }

    key_t queue_key = ftok(HOME_PATH, SERVER_ID);
    queue_id = msgget(queue_key, IPC_CREAT | 0666);

    signal(SIGINT, handle_server_end);

    MessageBuffer *server_buffer = malloc(sizeof (MessageBuffer));

    printf("SERVER INITIALIZED...\n");

    while (1) {
        msgrcv(queue_id, server_buffer, MESSAGE_SIZE, -6, 0);

        switch(server_buffer->type) {
            case INIT:
                printf("INIT RECEIVED\n");
                handle_init(server_buffer);
                save_message(server_buffer);
                break;
            case LIST:
                handle_list(server_buffer->client_id);
                save_message(server_buffer);
                break;
            case TOALL:
                handle_to_all(server_buffer);
                save_message(server_buffer);
                break;
            case TOONE:
                handle_to_one(server_buffer);
                save_message(server_buffer);
                break;
            case STOP:
                handle_stop(server_buffer->client_id);
                save_message(server_buffer);
                break;
            default:
                printf("");
//                printf("Incorrect message type!\n");
        }
    }
}

void handle_init(MessageBuffer *messageBuffer) {
    while (client_queue[first_free_id] != -1 && first_free_id < CLIENTS_LIMIT - 1) {
        first_free_id++;
    }

    if (client_queue[first_free_id] != -1 && first_free_id == CLIENTS_LIMIT - 1) {
        messageBuffer->client_id = -1;
    }
    else {
        messageBuffer->client_id = first_free_id;
        client_queue[first_free_id] = messageBuffer->queue_key;

        if (first_free_id < CLIENTS_LIMIT - 1) {
            first_free_id++;
        }
    }
    int client_queue_id = msgget(messageBuffer->queue_key, 0);
    msgsnd(client_queue_id, messageBuffer, MESSAGE_SIZE, 0);
    save_message(messageBuffer);
}

void handle_list(int client_id) {
    MessageBuffer *messageBuffer = malloc(sizeof (MessageBuffer));
    strcpy(messageBuffer->content, "");

    for (int i = 0; i < CLIENTS_LIMIT; i++) {
        if (client_queue[i] != -1) {
            sprintf(messageBuffer->content + strlen(messageBuffer->content), "Client: %d is active\n", i);
        }
    }

    messageBuffer->type = LIST;
    int client_queue_id = msgget(client_queue[client_id], 0);
    msgsnd(client_queue_id, messageBuffer, MESSAGE_SIZE, 0);
}

void handle_to_all(MessageBuffer *messageBuffer) {
    for (int i = 0; i < CLIENTS_LIMIT; i++) {
        if (client_queue[i] != -1 && messageBuffer->client_id != i) {
            int other_client_id = msgget(client_queue[i], 0);
            msgsnd(other_client_id, messageBuffer, MESSAGE_SIZE, 0);
        }
    }
}

void handle_to_one(MessageBuffer *messageBuffer) {
    if (client_queue[messageBuffer->other_id] != -1) {
        int other_client_id = msgget(client_queue[messageBuffer->other_id], 0);
        msgsnd(other_client_id, messageBuffer, MESSAGE_SIZE, 0);
    }
}

void handle_stop(int client_id) {
    client_queue[client_id] = -1;
    if (client_id < first_free_id) {
        first_free_id = client_id;
    }
}

void save_message(MessageBuffer *messageBuffer) {
    struct tm time = messageBuffer->time_struct;

    FILE *f = fopen("message_logs.txt", "a");

    switch (messageBuffer->type) {
        case INIT:
            if (messageBuffer->client_id == -1) {
                fprintf(f, "INIT -> Clients limit reached\n");
            } else {
                fprintf(f, "INIT -> Client id: %d", messageBuffer->client_id);
            }
            break;
        case LIST:
            fprintf(f, "LIST -> Client id: %d", messageBuffer->client_id);
            break;
        case TOALL:
            fprintf(f, "2ALL -> Sender id: %d", messageBuffer->client_id);
            fprintf(f, "2ALL -> Message: %s", messageBuffer->content);
            break;
        case TOONE:
            fprintf(f, "2ONE -> Sender id: %d, Receiver id: %d", messageBuffer->client_id, messageBuffer->other_id);
            fprintf(f, "2ONE -> Message: %s", messageBuffer->content);
            break;
        case STOP:
            fprintf(f, "STOP -> Client id: %d", messageBuffer->client_id);
            break;
    }
    fprintf(f, "Message sent at: %02d-%02d-%02d\n", time.tm_hour, time.tm_min, time.tm_sec);

    fclose(f);
}


void handle_server_end() {
    MessageBuffer *messageBuffer= malloc(sizeof (MessageBuffer));

    for (int i = 0; i < CLIENTS_LIMIT; i++) {
        if (client_queue[i] != -1) {
            messageBuffer->type = STOP;
            int client_queue_id = msgget(client_queue[i], 0);
            msgsnd(client_queue_id, messageBuffer, MESSAGE_SIZE, 0);
            msgrcv(queue_id, messageBuffer, MESSAGE_SIZE, STOP, 0);
        }
    }

    msgctl(queue_id, IPC_RMID, NULL);
    printf("SERVER CLOSING\n");
    exit(0);
}