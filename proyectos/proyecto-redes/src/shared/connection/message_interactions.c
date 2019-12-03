#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "../logger/logger.h"

void print_package(char * package)
{
  // Los primeros dos bytes los imprimimos como 'd' (números), porque así acordamos interpretarlos.
  printf("Paquete es: ");
  printf("%d ", package[0]); printf("%d ", package[1]); printf("%s\n", &package[2]);
}

void sendMessage(int socket, char* package, bool save_log)
{
  // Obtenemos el largo del payload para saber qué tamaño tiene el paquete y cuántos bytes debe enviar mi socket
  int payloadSize = package[1];
  send(socket, package, 2 + payloadSize, 0);

  if (save_log) log_print(0, *package, &package[2]);
  // print_package(package);
  free(package);
}

char* receiveMessage(int socket, bool save_log)
{
  // Esperamos a que llegue el primer byte, que corresponde al ID del paquete
  char ID;
  recv(socket, &ID, 1, 0);
  // printf("\n####### Paquete recibido ####\n");
  // printf("The ID is: %d\n", ID);

  // Recibimos el payload size en el siguiente byte
  char payloadSize;
  recv(socket, &payloadSize, 1, 0);
  // printf("The PayloadSize is: %d\n", payloadSize);

  // Recibimos el resto del paquete, según el payloadSize. Lo guardamos en un puntero de caracteres, porque no es necesario modificarlo
  char payload[payloadSize + 1];
  recv(socket, payload, payloadSize, 0);
  payload[payloadSize] = '\0';
  // printf("The Message is: %s\n", payload);
  // printf("#############################\n");

  char *package = malloc(3 + payloadSize);
  package[0] = ID;
  package[1] = payloadSize;
  strcpy(&package[2], payload);

  if (save_log) log_print(1, *package, &package[2]);
  return package;
}

int calculate_length(char * input)
{
  int i = 0;
  while (1){
    if (input[i] == '\0'){
      return i;
    }
    i++;
  }
}
