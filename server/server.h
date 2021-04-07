#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <regex.h>

#define MAX_CLIENTS 10
#define LEN 2048
#define NAME_LEN 32
#define MSG_LEN 2096

extern char *colours[7];
enum COLOR{WHITE, PURPLE, YELLOW, GREEN, BLUE, RED, CYAN};

 static unsigned int cli_count = 0;
 /*@unused@*/ static int uid = 10;

typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
  enum COLOR color;
} CLIENT;

extern CLIENT *clients[MAX_CLIENTS];
extern pthread_mutex_t clients_mutex;

//void str_overwrite_stdout();

void str_trim_lf (char* arr, size_t length);

void queue_add(CLIENT *cl);

void queue_remove(int uid);

void send_msg(char *msg, int uid);

int name_exists(char *name, int uid);

void *handle_client(void *c_id);

void handle_args(uint16_t *port, char **ip, int argc, char *argv[]);
