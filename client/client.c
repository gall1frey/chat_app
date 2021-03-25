#include "client.h"


int main(int argc, char const *argv[]) {
  signal(SIGINT, catch_ctrl_c_and_exit);

  fd = socket(AF_INET, SOCK_STREAM, 0);
  serv.sin_family = AF_INET;
  serv.sin_port = htons(8096);
  inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr); //This binds the client to localhost
  connect(fd, (struct sockaddr *)&serv, sizeof(serv)); //This connects the client to the server.

  //Set name
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
  //printf("[DEBUG]: Sending: %s\n",buf);
  send(fd, buf, 32, 0);
  int receive = recv(fd, buf, 3, 0);
  //printf("[DEBUG]: Received: %s\n",buf);
  if (!(receive > 0 && !strcmp(buf,"OK"))) {
    printf("Name already exists in chatroom. Try again.\n");
    close(fd);
    return EXIT_FAILURE;
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
