#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "initializer.h"
#include "handlers/handlers.h"
#include "../game/board.h"
#include "../shared/connection/message.h"
#include "../shared/connection/builders.h"
#include "../shared/connection/message_interactions.h"

int PORT;
char IP[100];
bool save_log = false;

int main (int argc, char *argv[])
{

  if (argc < 5 || argc > 6 || strcmp(argv[1], "-i") != 0 || strcmp(argv[3], "-p") != 0) {
    puts("Usage: ./server -i <ip_address> -p <tcp-port> -l");
    return 1;
  }
  if (argc == 6) {
    if (strcmp(argv[5], "-l") != 0) {
      puts("Usage: ./server -i <ip_address> -p <tcp-port> -l");
      return 1;
    }
    save_log = true;
  }

  strcpy(IP, argv[2]);
  PORT = atoi(argv[4]);

  int socket;
  printf("Client\n");
  socket = initializeClient(IP, PORT);

  char* msg_in = NULL;
  char* msg_out = NULL;
  char msg[] = " ";
  msg_out = build_pkg(StartConnection, msg);
  sendMessage(socket, msg_out, save_log);

  while (1) {
    msg_in = receiveMessage(socket, save_log);
    if (msg_in[0]==Disconnect) {
      free(msg_in);
      disconnect();
    }
    msg_out = response_handler(msg_in);

    if (msg_out) sendMessage(socket, msg_out, save_log);
    // Esperamos el mensaje del servidor
    if (msg_in) free(msg_in);
  }
}
