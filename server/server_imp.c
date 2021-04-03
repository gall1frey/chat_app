#include "server.h"

char *colours[7] = {"\033[0;37m", "\033[0;35m", "\033[0;33m", "\033[0;32m", "\033[0;34m", "\033[0;31m", "\033[0;36m"};
CLIENT *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void str_overwrite_stdout() {
    printf("\r%s", "> ");
    if(fflush(stdout) != 0)
      printf("SOME ERROR\n");
}

void str_trim_lf (char arr[], size_t length) {
  int i;
  for (i = 0; i < (int)length; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

void queue_add(CLIENT *cl){
	if(pthread_mutex_lock(&clients_mutex) != 0)
    printf("SOME ERROR\n");
  int i = 0;
	for(i=0; i < MAX_CLIENTS; ++i){
		if(!clients[i]){
			clients[i] = cl;
			clients[i]->color = (i%6)+1;
			break;
		}
	}
	if(pthread_mutex_unlock(&clients_mutex) != 0)
    printf("SOME ERROR\n");
}

void queue_remove(int uid){
  if(pthread_mutex_lock(&clients_mutex) != 0)
    printf("SOME ERROR\n");
  int i;
	for(i=0; i < MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid == uid){
				clients[i] = NULL;
				break;
			}
		}
	}

  if(pthread_mutex_unlock(&clients_mutex) != 0)
    printf("SOME ERROR\n");
}

void send_msg(char *msg, int uid){
  if(pthread_mutex_lock(&clients_mutex) != 0)
    printf("SOME ERROR\n");
  int i;
	for(i=0; i<MAX_CLIENTS; i++){
		if(clients[i] != NULL && clients[i]->uid != uid){
			if(write(clients[i]->sockfd,msg,strlen(msg)) < 0){
				perror("[ERROR]: Write to descriptor failed.");
				break;
			}
		}
	}
  if(pthread_mutex_unlock(&clients_mutex) != 0)
    printf("SOME ERROR\n");
}

int name_exists(char *name, int uid){
	int flag = 0;
  if(pthread_mutex_lock(&clients_mutex) != 0)
    printf("SOME ERROR\n");
  unsigned int i;
	for(i=0; i < cli_count; i++){
		//printf("[DEBUG]: %s : %s\n", clients[i]->name,name);
		if(clients[i] != NULL && strcmp(clients[i]->name,name) == 0 && clients[i]->uid != uid){
			flag = 1;
			break;
		}
	}
  if(pthread_mutex_unlock(&clients_mutex) != 0)
    printf("SOME ERROR\n");
	return flag;
}

void *handle_client(void *c_id){
	char name_msg[32] = "";
	char message_buf[LEN] = "";
	int leave_flag = 0;
	cli_count++;
	CLIENT *cli = (CLIENT *)c_id;

	int receive = (int)recv(cli->sockfd, name_msg, LEN, 0);
	if (receive > 0 && name_msg[0] == 'N' && name_msg[1] == 'M' && name_msg[2] == '>'){
		int i = 0;
		while(name_msg[i] != '\0' && i < 32){
			cli->name[i] = name_msg[i+3];
			i++;
		}
		if(name_exists(cli->name,cli->uid) == 0){
			(void)snprintf(message_buf,sizeof(message_buf), "%s%s has joined%s",colours[cli->color], cli->name,colours[0]);
			printf("%s%s\n", message_buf,colours[0]);
			(void)send(cli->sockfd,"OK",3,0);
			send_msg(message_buf,cli->uid);
		} else {
			leave_flag = 1;
		}
	} else {
		leave_flag = 1;
	}
	bzero(message_buf, LEN);
	while(1){
		if(leave_flag) break;

		int RECV = (int)recv(cli->sockfd, message_buf, LEN, 0);
		if(RECV > 0){
			if( strlen(message_buf) > 0 ){
				char tmp[2080] = "";
				(void)snprintf(tmp,sizeof(tmp),"%s%s%s",colours[cli->color],message_buf,colours[0]);
				(void)snprintf(message_buf,sizeof(message_buf),"%s",tmp);
				send_msg(message_buf, cli->uid);
				str_trim_lf(message_buf, strlen(message_buf));
				printf("%s%s -> %s%s\n",colours[cli->color],cli->name,message_buf,colours[0]);
			}
		} else if (RECV == 0 || strcmp(message_buf, "exit") == 0){
			(void)snprintf(message_buf, sizeof(message_buf), "%s%s has left%s.",colours[cli->color],cli->name,colours[0]);
			printf("%s\n", message_buf);
			send_msg(message_buf, cli->uid);
			leave_flag = 1;
		} else {
			printf("[ERROR]: -1\n");
			leave_flag = 1;
		}
		bzero(message_buf, LEN);
	}
	(void)close(cli->sockfd);
	queue_remove(cli->uid);
	free(cli);
	cli_count--;
	(void)pthread_detach(pthread_self());
	return NULL;
}

void handle_args(uint16_t *port, char **ip, int argc, char *argv[]){
  if (argc == 3){
    //Both IP and port given
    if(strlen(argv[1]) <= 5) {
      *port = (uint16_t)atoi(argv[1]);
      *ip = argv[2];
      return;
    } else {
      *port = (uint16_t)atoi(argv[2]);
      *ip = argv[1];
      return;
    }
  } else if (argc == 2) {
    if (strlen(argv[1]) <= 5) {
      *port = (uint16_t)atoi(argv[1]);
      *ip = "127.0.0.1";
      return;
    } else if (strlen(argv[1]) >= 7){
      *port = (uint16_t)8096;
      *ip = argv[1];
      return;
    }
  }
  //No args given
  *port = (uint16_t)8096;
  *ip = "127.0.0.1";
}
