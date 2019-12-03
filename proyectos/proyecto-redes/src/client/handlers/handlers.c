#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "handlers.h"
#include "../../game/board.h"
#include "../../shared/connection/message.h"
#include "../../shared/connection/builders.h"

/** Funcion que maneja cada respuesta */
char* response_handler(char *msg)
{
  MsgType type = msg[0];
  switch(type)
  {
    case ConnectionEstablished:
      return connectionestablished();
      break;
    case AskNickname:
      return asknickname();
      break;
    case Oponentfound:
      return oponentfound(msg);
      break;
    case Start:
      return start();
      break;
    case Scores:
      return scores(msg);
      break;
    case WhosFirst:
      return whosfirst(msg);
      break;
    case BoardState:
      return boardstate(msg);
      break;
    case ErrMove:
      return errmove();
      break;
    case OkMove:
      return okmove();
      break;
    case End:
      return end();
      break;
    case GameWinnerLoser:
      return gamewinnerloser(msg);
      break;
    case AskNewGame:
      return asknewgame();
      break;
    case Disconnect:
      disconnect();
      break;
    case ErrBadPkg:
      return errbadpkg();
      break;
    case SpreadMsg:
      return spreadmsg(msg);
      break;
    case Image:
      return image();
      break;
    default:
      printf("Wrong message type\n");
  }

  return NULL;
}

char* connectionestablished()
{
  return NULL;
}

char* asknickname()
{
  char input[100];
  printf("\nIngresa nombre de usuario. En caso de estar ocupado se preguntará por este nuevamente\nRespuesta: ");
  scanf("%s", input);
  printf("\n");
  char* pkg = build_pkg(RetNickname, input);
  puts("Cargando...");
  return pkg;
}

void disconnect()
{
  puts("Se ha concluido tu conexión: ya sea porque te desconectaste o el contrincante no quiere seguir jugando");
  exit(0);
}

char* oponentfound(char *msg)
{
  char *oponent = malloc(2 + sizeof(char) * (int) msg[1]);
  strcpy(oponent, &msg[2]);
  oponent[(int) msg[1]] = '\0';
  printf("El oponente es: %s\n", oponent);
  free(oponent);
  return NULL;
}

char* start()
{
  printf("Empieza la partida! \n");
  return NULL;
}

char* scores(char *msg)
{
  printf("Tu puntaje es: %c\n", msg[2]);
  printf("El puntaje del oponente es: %c\n", msg[3]);
  return NULL;
}

char* whosfirst(char *msg)
{
  if (msg[2]=='2') printf("Tu Partes con las fichas blancas\n");
  else printf("Parte tu oponente. Juegas con las fichas negras\n");
  return NULL;
}

char* boardstate(char* msg)
{
  Board* board = char_to_board(msg);
  board_printer(board);

  char command[1];
  while (true) {
    printf("\n(1) Enviar movimiento \n(2) Enviar mensaje\n(3) Enviar imagen\n(4) Salir\nRespuesta: ");
    scanf("%s", command);
    if (command[0] == '1' || command[0] == '2' || command[0] == '3' || command[0] == '4') break;
    puts("Comando inválido, pruebe nuevamente");
  }

  char input[100];
  MsgType type;

  if (command[0] == '1') {
    printf("\nIngresa tu movimiento: ");
    char aux_input[100]; 
    char aux_input2[100]; 
    scanf("%s %s", aux_input, aux_input2);
    strcpy(input, aux_input);
    strcat(input, aux_input2);
    type = SendMove;
  } else if (command[0] == '2') {
    printf("\nIngresa tu mensaje: ");
    scanf("%s", input);
    type = SendMsg;
  } else if (command[0] == '3') {
    printf("\nIngresa tu comando: ");
    scanf("%s", input);
    type = Image;
  } else if (command[0] == '4') {
    type = Disconnect;
    input[0] = ' ';
    input[1] = '\0';
  }

  char* pkg = build_pkg(type, input);
  board_destroy(board);
  return pkg;
}

char* errmove()
{
  printf("Error en el movimiento, intenta nuevamente\n");
  return NULL;
}

char* okmove()
{
  printf("Movimiento realizado exitosamente, turno de tu oponente\n");
  return NULL;
}

char* end()
{
  printf("Partida finalizada");
  return NULL;
}

char* gamewinnerloser(char *msg)
{
  if (msg[2] == 0) puts("¡El juego termina en empate!");
  else if (msg[2] == -1) puts("El juego no ha terminado");
  else printf("Gana el jugador %c\n",msg[2]);
  return NULL;
}

char* asknewgame()
{
  char command[1];
  printf("\n(1) Nuevo juego\n(2) Salir\nRespuesta: ");
  scanf("%s", command);
  char* pkg;
  if (command[0] == '1') pkg = build_pkg(AnswerNewGame, "1");
  else pkg = build_pkg(AnswerNewGame, "0");
  return pkg;
}

char* errbadpkg()
{
  char msg[] = " ";
  char* pkg = build_pkg(StartConnection, msg);
  return pkg;
}

char* spreadmsg(char* msg)
{
  char *mensaje = malloc(2 + sizeof(char) * (int) msg[1]);
  strcpy(mensaje, &msg[2]);
  mensaje[(int) msg[1]] = '\0';
  printf("El oponente envía: %s\n", mensaje);
  free(mensaje);
  return NULL;
}

char* image()
{
  char msg[] = " ";
  char* pkg = build_pkg(StartConnection, msg);
  return pkg;
}