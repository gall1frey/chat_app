#include "client.h"

volatile sig_atomic_t flag = 0;
struct sockaddr_in serv;
int fd;
int conn;
char name[32];

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
