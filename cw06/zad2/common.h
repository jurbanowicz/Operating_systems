#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#define server_queue "/SERVER"
#define CLIENTS_LIMIT 10
#define MESSAGE_SIZE_LIMIT 512

typedef struct message_buffer {
    int client_id;
    int other_id;
    long type;
    char content[MESSAGE_SIZE_LIMIT];
    struct tm time_struct;
} message_buffer;

typedef enum Message_type {
    INIT = 1,
    LIST = 2,
    TONE = 3,
    TALL = 4,
    STOP = 5
} Message_type;

const int MSG_SIZE = sizeof(message_buffer);
