#include "client.h"

volatile sig_atomic_t flag = 0;
struct sockaddr_in serv;
int fd;
int conn;
char name[32];

void str_overwrite_stdout() {
  printf("%s", "> ");
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

void catch_ctrl_c_and_exit( /*@unused@*/ int sig) {
    flag = 1;
}

void send_func() {
  char message[LEN] = "";
	char buffer[LEN + 32] = "";
  while(1) {
    str_overwrite_stdout();
    (void)fgets(message, 100, stdin);
    str_trim_lf(message,LEN);
    if(!strcmp(message,"exit"))
      break;
    else{
      int len = snprintf(buffer,LEN+32,"%s: %s", name, message);
      (void)send(fd, buffer, (size_t)len, 0);
    }
    bzero(message,LEN);
    bzero(buffer,LEN+32);
  }
  catch_ctrl_c_and_exit(2);
}

void recv_func() {
  char message[LEN] = "";
  //printf("Thread starting...\n");
  while (1) {
    int receive = (int)recv(fd, message, LEN, 0);
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
