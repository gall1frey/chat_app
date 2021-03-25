#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_CLIENTS 5
#define LEN 2080

extern char *colours[8];
enum COLOR{WHITE, PURPLE, YELLOW, GREEN, BLUE, RED, CYAN};

 static _Atomic unsigned int cli_count = 0;
 static int uid = 10;

typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
  enum COLOR color;
} CLIENT;

extern CLIENT *clients[MAX_CLIENTS];
extern pthread_mutex_t clients_mutex;

void str_overwrite_stdout();

void str_trim_lf (char* arr, int length);

void queue_add(CLIENT *cl);

void queue_remove(int uid);

void send_msg(char *msg, int uid);

int name_exists(char *name, int uid);

void *handle_client(void *c_id);
