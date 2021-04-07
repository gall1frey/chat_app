#include "server.h"

char *colours[7] = {"\033[0;37m", "\033[0;35m", "\033[0;33m", "\033[0;32m", "\033[0;34m", "\033[0;31m", "\033[0;36m"};
CLIENT *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

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
	char name_msg[NAME_LEN] = "";
	char message_buf[LEN] = "";
	int leave_flag = 0;
	cli_count++;
	CLIENT *cli = (CLIENT *)c_id;

	int receive = (int)recv(cli->sockfd, name_msg, NAME_LEN+3, 0);
	if (receive > 0 && name_msg[0] == 'N' && name_msg[1] == 'M' && name_msg[2] == '>'){
		int i = 0;
		while(name_msg[i] != '\0' && i < NAME_LEN){
			cli->name[i] = name_msg[i+3];
			i++;
		}
		if(name_exists(cli->name,cli->uid) == 0){
			(void)snprintf(message_buf,sizeof(message_buf), "%s%s has joined%s",colours[cli->color], cli->name,colours[0]);
			printf("%s\n", message_buf);
			(void)send(cli->sockfd,"OK",3,0);
			send_msg(message_buf,cli->uid);
		} else {
			leave_flag = 1;
		}
	} else {
		leave_flag = 1;
	}
	bzero(message_buf, LEN);
	while(1==1){
		if(leave_flag == 1) break;

		int RECV = (int)recv(cli->sockfd, message_buf, LEN, 0);
		if(RECV > 0){
			if( strlen(message_buf) > 0 ){
				char message[MSG_LEN] = "";
				(void)snprintf(message,sizeof(message),"%s%s: %s%s",colours[cli->color],cli->name,message_buf,colours[0]);
				//(void)snprintf(message_buf,sizeof(message_buf),"%s",tmp);
				send_msg(message, cli->uid);
				str_trim_lf(message, strlen(message));
				printf("%s%s%s\n",colours[cli->color],message,colours[0]);
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
  regex_t ip_regex, port_regex;
  (void)regcomp(&ip_regex,
        "^([0-9]|[1-9][0-9]|1([0-9][0-9])|2([0-4][0-9]|5[0-5]))."
         "([0-9]|[1-9][0-9]|1([0-9][0-9])|2([0-4][0-9]|5[0-5]))."
         "([0-9]|[1-9][0-9]|1([0-9][0-9])|2([0-4][0-9]|5[0-5]))."
         "([0-9]|[1-9][0-9]|1([0-9][0-9])|2([0-4][0-9]|5[0-5]))$", REG_EXTENDED);
  (void)regcomp(&port_regex,
         "^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}"
         "|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$",REG_EXTENDED);
  int i;
  for(i=1;i<argc;i++){
    if(regexec(&ip_regex, argv[i], 0, NULL, 0) == 0)
      (void)snprintf(*ip,16,"%s",argv[i]);
    else if(regexec(&port_regex,argv[i], 0, NULL, 0) == 0)
      *port = (uint16_t)atoi(argv[i]);
  }
  if(*port == 0)
    *port = (uint16_t)8096;
  if(strcmp(*ip,"")==0)
    (void)snprintf(*ip,16,"127.0.0.1");
}
