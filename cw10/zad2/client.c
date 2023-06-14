#include "common.h"


static int socket_1;
static int client_id;
static int is_accepted = 0;
static struct sockaddr_in ser;
static int port_no;
char* ip_adress;
char* name;

int create_socket_client(int type) {
    int main_socket;

    main_socket = socket(AF_INET, type, 0);
    if (main_socket == -1) {
        fprintf(stderr, "Error creating socket\n");
        return -1;
    }
    return main_socket;
}

enum ClientCommand get_user_command(char *buff) {
    fgets(buff, MAX_MESSAGE_SIZE, stdin);
    buff[strlen(buff) - 1] = '\0';
    if (strcmp(buff, "LIST") == 0) { return COMMAND_LIST; }
    if (strcmp(buff, "STOP") == 0) { return COMMAND_STOP; }
    if (strcmp(buff, "HELP") == 0) { return COMMAND_HELP; }
    if (strncmp(buff, "2ALL ", strlen("2ALL ")) == 0) { return COMMAND_2ALL; }
    if (strncmp(buff, "2ONE ", strlen("2ONE ")) == 0) { return COMMAND_2ONE; }

    return COMMAND_INVALID;
}

void print_help() {
    printf("Available commands:\n");
    printf("'LIST' -> lists users currently connected to the server\n");
    printf("'STOP' -> logs out from the server and stops the current program\n");
    printf("'2ONE [id] [message]' -> sends [message] to client with given [id]\n");
    printf("'2ALL [message]' -> sends [message] to all users\n");
}

void my_exit() {
    close(socket_1);
}

void send_message(MessageType type, char *text, int to_id) {
    Message msg;
    msg.from_id = client_id;
    msg.msg_type = type;
    msg.to_id = to_id;
    strcpy(msg.client_name, name);
    strcpy(msg.msg_text, text);
    sendto(socket_1, &msg, sizeof(msg), 0, (struct sockaddr *)&ser, sizeof(ser));
}

void signal_handler(int signo) {
    Message msg;
    msg.msg_type = STOP;
    sendto(socket_1, &msg, sizeof(msg), 0, (struct sockaddr *)&ser, sizeof(ser));
    printf("User logging out...\n");
    exit(0);
}

void *client_thread_handler(void *arg) {
    int status;
    Message msg;
    while (1)
    {
        status = read(socket_1, &msg, sizeof(Message));
        if (status <= 0)
        {
            printf("Server not responding...\nClosing programe\n");
            exit(0);
        }
        switch (msg.msg_type)
        {
        case INIT:
            client_id = msg.client_id;
            is_accepted = 1;
            printf("Logged on to hte server, client id: %d\n", client_id);
            break;
        case STOP:
            printf("Server not responding...\nClosing programe\n");
            exit(0);
            break;
        case PING:
            send_message(PING, "", -1);
            break;
        default:
            printf("\n[%s] Message recieved %s, from client: %s, type: %s\n", msg.date, msg.msg_text, msg.client_name, get_message_type(msg.msg_type));
            break;
        }
    }
}

int main(int argc, char** argv) {
    if(argc!=4){
        printf("Incorrect argumetns!\n");
        return 1;
    }
    name = argv[1];
    ip_adress = argv[2];
    port_no = atoi(argv[3]);
    if(port_no == 0){
        printf("Port must be a numebr\n");
        return 1;
    }
    atexit(my_exit);
    signal(SIGINT, signal_handler);

    socket_1 = create_socket_client(SOCK_DGRAM);
    if(socket_1 == -1){
        return 1;
    }

    memset(&ser, 0, sizeof(ser));
    ser.sin_family = AF_INET;
    ser.sin_port = htons(port_no);
    ser.sin_addr.s_addr = inet_addr(ip_adress);

    pthread_t client_thread;
    pthread_create(&client_thread, (void *)NULL, client_thread_handler, (void *)NULL);

    int flag = 0;
    enum ClientCommand command = 0;
    char buff[MAX_MESSAGE_SIZE];
    char *content;
    int to_send_id;
    int counter = 0;
    

    while(!is_accepted && counter < 10){
        sleep(1);
        counter++;
    }
    if(!is_accepted){
        printf("Server not responding\n");
        exit(0);
    }
    while (command != COMMAND_STOP)
    {
        printf("ENTER A COMMAND: ('HELP' to get available commands)\n");
        command = get_user_command(buff);
        switch (command)
        {
        case COMMAND_STOP:
            flag = 1;
            Message msg;
            msg.msg_type = STOP;
            msg.from_id = client_id;
            sendto(socket_1, &msg, sizeof(msg), 0, (struct sockaddr *)&ser, sizeof(ser));
            
            break;

        case COMMAND_INVALID:
            printf("Invalid command (you may enter help for command list)\n");
            break;

        case COMMAND_HELP:
            print_help();
            break;

        case COMMAND_LIST:
            send_message(LIST, "", -1);
            break;

        case COMMAND_2ALL:
            send_message(TO_ALL, &buff[5], -1);
            break;

        case COMMAND_2ONE:
            content = strstr(buff, " ");
            to_send_id = atoi(content);
            content = strstr(content, &content[1]);
            content = strstr(content, " ");
            content = strstr(content, &content[1]);
            send_message(TO_ONE, content, to_send_id);
            break;

        default:
            break;
        }
        if (flag)
            break;
    }
    printf("Client exiting\n");
    exit(0);
    return 0;
}