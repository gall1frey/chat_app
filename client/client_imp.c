#include "client.h"

volatile sig_atomic_t flag = 0;
struct sockaddr_in serv;
int fd;
int conn;
char name[NAME_LEN];

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
	//char buffer[LEN + 32] = "";
  while(1) {
    str_overwrite_stdout();
    (void)fgets(message, LEN, stdin);
    str_trim_lf(message,LEN);
    if(!strcmp(message,"exit"))
      break;
    else{
      //int len = snprintf(buffer,LEN,"%s", message);
      int len = strlen(message);
      (void)send(fd, message, (size_t)len, 0);
    }
    bzero(message,LEN);
    //bzero(buffer,LEN+32);
  }
  catch_ctrl_c_and_exit(2);
}

void recv_func() {
  char message[RECV_LEN] = "";
  //printf("Thread starting...\n");
  while (1) {
    int receive = (int)recv(fd, message, RECV_LEN, 0);
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
