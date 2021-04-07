#include "server.h"

int main(int argc, char *argv[]) {
	char *ip = "";
	uint16_t port = 0;
	handle_args(&port, &ip, argc, argv);
	printf("PORT: %u\n", (unsigned int)port);
	printf("IP: %s\n", ip);
	printf("Use this to run client.\n");
	int option = 1, listenfd = 1, connfd = 0;

	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	pthread_t tid = 0;

	(void)signal(SIGPIPE, SIG_IGN);

	listenfd = socket(AF_INET, SOCK_STREAM, 0); //This will create a new socket and also return the identifier of the socket into fd.
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port); //Define the port at which the server will listen for connections.
  serv_addr.sin_addr.s_addr = inet_addr(ip);

	if(setsockopt(listenfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,(socklen_t)sizeof(option)) < 0){
		perror("ERROR: setsockopt failed");
    return EXIT_FAILURE;
	}

	/* Bind */
  if(bind(listenfd, (struct sockaddr*)&serv_addr, (socklen_t)sizeof(serv_addr)) < 0) {
    perror("ERROR: Socket binding failed");
    return EXIT_FAILURE;
  }

	//Listen for client connections. Maximum 10 connections will be permitted.
	if (listen(listenfd, 10) < 0) {
		perror("ERROR: Socket listening failed");
		return EXIT_FAILURE;
	}

	printf("CHATROOM SERVER\n");

	while (1==1) {
		socklen_t clilen = (socklen_t)sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);
		if((cli_count + 1) == MAX_CLIENTS){
			printf("Max clients reached. Rejected.\n");
			//print_client_addr(cli_addr);
			//printf(":%d\n", cli_addr.sin_port);
			(void)close(connfd);
			continue;
		}

		/* Client settings */
		CLIENT *cli = (CLIENT *)malloc(sizeof(CLIENT));
		if(cli == NULL)
			return EXIT_FAILURE;
		cli->address = cli_addr;
		cli->sockfd = connfd;
		cli->uid = uid++;
		cli->color = 0;

		/* Add client to the queue and fork thread */
		queue_add(cli);
		if(pthread_create(&tid, NULL, &handle_client, (void*)cli) != 0){
			printf("ERROR: pthread\n");
			free(cli);
			return EXIT_FAILURE;
		}

		/* Reduce CPU usage */
		(void)sleep(1);
	}

	return EXIT_SUCCESS;
}
