#include "server.h"

int main(int argc, char const *argv[]) {
	char *ip = int port = (argc >= 3)? argv[1] : "127.0.0.1";
	int port = (argc >= 3)? atoi(argv[2]) : 8096;
	printf("PORT: %d\n", port);
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

	//Listen for client connections. Maximum 10 connections will be permitted.
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
