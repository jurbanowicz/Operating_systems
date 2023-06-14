#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <pthread.h>

#define MAX_MESSAGE_SIZE 512

typedef enum MessageType {
  STOP = 1,
  LIST = 2,
  TO_ONE = 3,
  TO_ALL = 4,
  INIT = 5,
  PING = 6
} MessageType;

typedef struct Message {
  MessageType msg_type;
  int from_id, to_id;
  char msg_text[MAX_MESSAGE_SIZE];
  int client_id;
  char date[MAX_MESSAGE_SIZE];
  char client_name[MAX_MESSAGE_SIZE];
} Message;


typedef struct ClientThreadArgs {
  int client_id;
  int socket_no;
} ClientThreadArgs;

typedef enum ClientCommand
{
    COMMAND_LIST,
    COMMAND_2ALL,
    COMMAND_2ONE,
    COMMAND_STOP,
    COMMAND_INVALID,
    COMMAND_HELP
} CleintCommand;

char *get_message_type(MessageType type) {
  switch (type)
  {
  case STOP:
    return "STOP";
  case LIST:
    return "LSIT";
  case TO_ONE:
    return "TO_ONE";
  case TO_ALL:
    return "TO_ALL";
  case INIT:
    return "INIT";
  case PING:
    return "PING";
  default:
    return "WRONG MESSAGE TYPE";
  }
}