#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "math.h"

#include "handlers/handlers.h"
#include "../shared/connection/message_interactions.h"

int* initializeServer(char* ip, int port, bool save_log)
{
	int welcomeSocket, firstSocket, secondSocket;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;

	/*---- Creación del Socket. Se pasan 3 argumentos ----*/
	/* 1) Internet domain 2) Stream socket 3) Default protocol (TCP en este caso) */
	welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);

	/*---- Configuración de la estructura del servidor ----*/
	/* Address family = Internet */
	serverAddr.sin_family = AF_INET;
	/* Set port number */
	serverAddr.sin_port = htons(port);
	/* Setear IP address como localhost */
	serverAddr.sin_addr.s_addr = inet_addr(ip);
	/* Setear todos los bits del padding en 0 */
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	/*---- Bindear la struct al socket ----*/
	bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	/*---- Listen del socket, con un máximo de 5 conexiones (solo como ejemplo) ----*/
	if (listen(welcomeSocket, 5) == 0)
		printf("Waiting player to connect...\n");
	else
		printf("Error\n");

	addr_size = sizeof serverStorage;
	firstSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);

	char* msg_in;
	char* msg_out;
	msg_in = receiveMessage(firstSocket, save_log);
	msg_out = response_handler(msg_in, NULL, 0);
	sendMessage(firstSocket, msg_out, save_log);
	free(msg_in);

	printf("\nConected player 1\n");

		/*---- Listen del socket, con un máximo de 5 conexiones (solo como ejemplo) ----*/
	if (listen(welcomeSocket, 5) == 0)
		printf("Waiting for the second player to connect...\n");
	else
		printf("Error\n");

	addr_size = sizeof serverStorage;
	secondSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);

	msg_in = receiveMessage(secondSocket, save_log);
	msg_out = response_handler(msg_in, NULL, 0);
	sendMessage(secondSocket, msg_out, save_log);
	free(msg_in);

	printf("Conected player 2\n");

	static int sockets[2];
	sockets[0] = firstSocket;
	sockets[1] = secondSocket;

	return sockets;
}

void terminate_connection(int socket) {
  close(socket);
}
