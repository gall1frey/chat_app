#include "client.h"

int main(int argc, char *argv[]) {
  char* IP = "127.0.0.1";
  uint16_t PORT = 8096;
  handle_args(&PORT, &IP, argc, argv);
  printf("PORT: %u\n", (unsigned int)PORT);
  printf("IP: %s\n", IP);
  (void)signal(SIGINT, catch_ctrl_c_and_exit);

  fd = socket(AF_INET, SOCK_STREAM, 0);
  serv.sin_family = AF_INET;
  serv.sin_port = htons(PORT);
  if (inet_pton(AF_INET, IP, &serv.sin_addr) != 1){
    printf("SOME ERROR OCCURED!\n");
    return EXIT_FAILURE;
  }
  if(connect(fd, (struct sockaddr *)&serv, (socklen_t)sizeof(serv)) != 0) //This connects the client to the server.
    return EXIT_FAILURE;
  //Set name
  printf("Please enter your name: ");
  (void)fgets(name, NAME_LEN-1, stdin);
  (void)fflush(stdin);
  str_trim_lf(name, strlen(name));
  //Name validation
  if (strlen(name) > NAME_LEN-1 || strlen(name) < 2){
		printf("Name must be less than 32 and more than 2 characters.\n");
		return EXIT_FAILURE;
	}
  char buf[NAME_LEN+3] = "";
  int len = snprintf(buf,sizeof(buf),"NM>%s",name);
  //printf("[DEBUG]: Sending: %s\n",buf);
  (void)send(fd, buf, (size_t)len, 0);
  int receive = (int)recv(fd, buf, 3, 0);
  //printf("[DEBUG]: Received: %s\n",buf);
  if (!(receive > 0 && strcmp(buf,"OK") == 0)) {
    printf("Name already exists in chatroom. Try again.\n");
    (void)close(fd);
    return EXIT_FAILURE;
  }

  printf("WELCOME TO CHATROOM!\n");

  pthread_t send_msg_thread = 0;
  if(pthread_create(&send_msg_thread, NULL, (void *) send_func, NULL) != 0){
		printf("ERROR: pthread\n");
    return EXIT_FAILURE;
	}

	pthread_t recv_msg_thread = 0;
  if(pthread_create(&recv_msg_thread, NULL, (void *) recv_func, NULL) != 0){
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

  while (1==1){
    if(flag == 1){
      printf("\nBye\n");
      break;
    }
  }

  (void)close(fd);

  return EXIT_SUCCESS;
}
