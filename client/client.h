#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LEN 2048

extern volatile sig_atomic_t flag;
extern struct sockaddr_in serv; //This is our main socket variable.
extern int fd; //This is the socket file descriptor that will be used to identify the socket
extern int conn; //This is the connection file descriptor that will be used to distinguish client connections.
extern char name[32]; //This array will store the client's name

void str_overwrite_stdout();
void str_trim_lf (char* arr, size_t length);
void catch_ctrl_c_and_exit(int sig);
void send_func();
void recv_func();
void handle_args(uint16_t *port, char **ip, int argc, char *argv[]);
