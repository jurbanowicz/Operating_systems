#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/msg.h>


#define HOME_PATH getenv("HOME")
#define SERVER_ID 1
#define CLIENTS_LIMIT 10
#define MESSAGE_LEN_LIMIT 512

typedef struct MessageBuffer {
    long type;
    key_t queue_key;
    int client_id;
    int other_id;
    char content[MESSAGE_LEN_LIMIT];
    struct tm time_struct;
} MessageBuffer;

const int MESSAGE_SIZE = sizeof(MessageBuffer);

typedef enum MessageType {
    INIT = 1,
    LIST = 2,
    TOALL = 3,
    TOONE = 4,
    STOP = 5
} MessageType;
