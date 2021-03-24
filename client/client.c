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

volatile sig_atomic_t flag = 0;
struct sockaddr_in serv; //This is our main socket variable.
int fd; //This is the socket file descriptor that will be used to identify the socket
int conn; //This is the connection file descriptor that will be used to distinguish client connections.
//char message[LEN-32] = ""; //This array will store the messages that are sent by the server
char name[32]; //This array will store the client's name

void str_overwrite_stdout() {
  printf("%s", "> ");
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

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void send_func() {
  char message[LEN] = {};
	char buffer[LEN + 32] = {};
  while(1) {
    str_overwrite_stdout();
    fgets(message, 100, stdin);
    str_trim_lf(message,LEN);
    if(!strcmp(message,"exit"))
      break;
    else{
      sprintf(buffer,"%s: %s\n", name, message);
      send(fd, buffer, strlen(buffer), 0);
    }
    bzero(message,LEN);
    bzero(buffer,LEN+32);
  }
  catch_ctrl_c_and_exit(2);
}

void recv_func() {
  char message[LEN] = {};
  //printf("Thread starting...\n");
  while (1) {
    int receive = recv(fd, message, LEN, 0);
    if (receive > 0) {
      printf("%s\n", message);
      str_overwrite_stdout();
    } else if (receive == 0) {
      break;
    } else {
      catch_ctrl_c_and_exit(2);
    }
    memset(message,0,sizeof(message));
  }
}

int main(int argc, char const *argv[]) {
  signal(SIGINT, catch_ctrl_c_and_exit);

  fd = socket(AF_INET, SOCK_STREAM, 0);
  serv.sin_family = AF_INET;
  serv.sin_port = htons(8096);
  inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr); //This binds the client to localhost
  connect(fd, (struct sockaddr *)&serv, sizeof(serv)); //This connects the client to the server.

  //Set name
  while (1) {
    printf("Please enter your name: ");
    fgets(name, 32, stdin);
    str_trim_lf(name, strlen(name));
    //Name validation
    if (strlen(name) > 32 || strlen(name) < 2){
  		printf("Name must be less than 30 and more than 2 characters.\n");
  		return EXIT_FAILURE;
  	}
    char buf[32] = {};
    sprintf(buf,"NM>%s",name);
    printf("[DEBUG]: Sending: %s\n",buf);
    send(fd, buf, 32, 0);
    int receive = recv(fd, buf, 3, 0);
    printf("[DEBUG]: Received: %s\n",buf);
    if (receive > 0 && !strcmp(buf,"OK")){
      break;
    }
    else
      printf("Name already exists in chatroom. Try again.\n");
  }

  printf("WELCOME TO CHATROOM!\n");

  pthread_t send_msg_thread;
  if(pthread_create(&send_msg_thread, NULL, (void *) send_func, NULL) != 0){
		printf("ERROR: pthread\n");
    return EXIT_FAILURE;
	}

	pthread_t recv_msg_thread;
  if(pthread_create(&recv_msg_thread, NULL, (void *) recv_func, NULL) != 0){
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

  while (1){
    if(flag){
      printf("\nBye\n");
      break;
    }
  }

  close(fd);

  return EXIT_SUCCESS;
}
