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

static _Atomic unsigned int cli_count = 0;
static int uid = 10;

typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
  char color[32];
} CLIENT;

CLIENT *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void str_overwrite_stdout() {
    printf("\r%s", "> ");
    fflush(stdout);
}

void str_trim_lf (char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

void queue_add(CLIENT *cl){
	pthread_mutex_lock(&clients_mutex);
	for(int i=0; i < MAX_CLIENTS; ++i){
		if(!clients[i]){
			clients[i] = cl;
			break;
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

void queue_remove(int uid){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid == uid){
				clients[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

void send_msg(char *msg, int uid){
	pthread_mutex_lock(&clients_mutex);
	for(int i=0; i<MAX_CLIENTS; i++){
		if(clients[i] && clients[i]->uid != uid){
			if(write(clients[i]->sockfd,msg,strlen(msg)) < 0){
				perror("[ERROR]: Write to descriptor failed.");
				break;
			}
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *c_id){
	char name_msg[32] = {};
	char message_buf[LEN];
	int leave_flag = 0;
	cli_count++;
	CLIENT *cli = (CLIENT *)c_id;

	int receive = recv(cli->sockfd, name_msg, LEN, 0);
	if (receive > 0 && name_msg[0] == 'N' && name_msg[1] == 'M' && name_msg[2] == '>'){
		int i = 0;
		while(name_msg[i] != '\0' && i < 32){
			cli->name[i] = name_msg[i+3];
			i++;
		}
		sprintf(message_buf, "%s has joined\n", cli->name);
		printf("%s\n", message_buf);
		send(cli->sockfd,"OK",3,0);
		send_msg(message_buf,cli->uid);
	} else {
		leave_flag = 1;
	}
	bzero(message_buf, LEN);
	while(1){
		if(leave_flag) break;

		int receive = recv(cli->sockfd, message_buf, LEN, 0);
		if(receive > 0){
			if( strlen(message_buf) > 0 ){

				send_msg(message_buf, cli->uid);
				str_trim_lf(message_buf, strlen(message_buf));
				printf("%s -> %s\n",cli->name,message_buf);
			}
		} else if (receive == 0 || !strcmp(message_buf, "exit")){
			sprintf(message_buf, "%s has left.\n", cli->name);
			printf("%s", message_buf);
			send_msg(message_buf, cli->uid);
			leave_flag = 1;
		} else {
			printf("[ERROR]: -1\n");
			leave_flag = 1;
		}
		bzero(message_buf, LEN);
	}
	close(cli->sockfd);
	queue_remove(cli->uid);
	free(cli);
	cli_count--;
	pthread_detach(pthread_self());
	return NULL;
}

int main(int argc, char const *argv[]) {
	char *ip = "127.0.0.1";
	int port = 8096;
	int option = 1, listenfd = 1, connfd = 0;

	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	pthread_t tid;

	signal(SIGPIPE, SIG_IGN);

	listenfd = socket(AF_INET, SOCK_STREAM, 0); //This will create a new socket and also return the identifier of the socket into fd.
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port); //Define the port at which the server will listen for connections.
  serv_addr.sin_addr.s_addr = inet_addr(ip);

	if(setsockopt(listenfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){
		perror("ERROR: setsockopt failed");
    return EXIT_FAILURE;
	}

	/* Bind */
  if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR: Socket binding failed");
    return EXIT_FAILURE;
  }

	//Listen for client connections. Maximum 5 connections will be permitted.
	if (listen(listenfd, 10) < 0) {
		perror("ERROR: Socket listening failed");
		return EXIT_FAILURE;
	}

	printf("CHATROOM SERVER\n");

	while (1) {
		socklen_t clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);
		if((cli_count + 1) == MAX_CLIENTS){
			printf("Max clients reached. Rejected: ");
			//print_client_addr(cli_addr);
			printf(":%d\n", cli_addr.sin_port);
			close(connfd);
			continue;
		}

		/* Client settings */
		CLIENT *cli = (CLIENT *)malloc(sizeof(CLIENT));
		cli->address = cli_addr;
		cli->sockfd = connfd;
		cli->uid = uid++;

		/* Add client to the queue and fork thread */
		queue_add(cli);
		pthread_create(&tid, NULL, &handle_client, (void*)cli);

		/* Reduce CPU usage */
		sleep(1);
	}

	return EXIT_SUCCESS;
}
