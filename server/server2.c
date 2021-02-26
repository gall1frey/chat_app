#include <stdio.h>
#include <sys/socket.h> //For Sockets
#include <stdlib.h>
#include <netinet/in.h> //For the AF_INET (Address Family)
#include <sys/types.h>
#include <pthread.h>
#include <string.h>

struct sockaddr_in serv; //This is our main socket variable.
int fd; //This is the socket file descriptor that will be used to identify the socket
int conn; //This is the connection file descriptor that will be used to distinguish client connections.
char message[100]; //This array will store the messages that are sent by the server
int sockets[5];
int i = 0;

void* server_distrib(void *arg) {
  //char msg[100];
  printf("THREAD STARTED\n");
  while (recv(conn, message, 100, 0)>0) {
      printf("Message Received: %s\n", message);
      for(int j = 0; j<5; j++)
        send(sockets[j],message,strlen(message),0);
      //An extra breaking condition can be added here (to terminate the child process)
      strcpy(message,"\0");
  }
  return NULL;
}

int main(int argc, char const *argv[]) {
  serv.sin_family = AF_INET;
  serv.sin_port = htons(8096); //Define the port at which the server will listen for connections.
  serv.sin_addr.s_addr = INADDR_ANY;
  fd = socket(AF_INET, SOCK_STREAM, 0); //This will create a new socket and also return the identifier of the socket into fd.
  // To handle errors, you can add an if condition that checks whether fd is greater than 0. If it isn't, prompt an error
  bind(fd, (struct sockaddr *)&serv, sizeof(serv)); //assigns the address specified by serv to the socket
  listen(fd,5); //Listen for client connections. Maximum 5 connections will be permitted.
  pthread_t thread[5];
  while(conn = accept(fd, (struct sockaddr *)NULL, NULL)){
    sockets[i++%5] = conn;
    pthread_create(&thread[i++%5],NULL,server_distrib,NULL);
  }

  //Now we start handling the connections.

  return 0;
}
