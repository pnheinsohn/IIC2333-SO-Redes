#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "handlers.h"
#include "../../game/board.h"
#include "../../shared/connection/message.h"
#include "../../shared/connection/builders.h"

/** Funcion que maneja cada respuesta */
char* response_handler(char *msg, Board *board, int turn)
{
  MsgType type = msg[0];
  switch(type)
  {
    case StartConnection:
      return startconnection();
      break;
    case RetNickname:
      return retnickname(msg);
      break;
    case SendMove:
      return sendmove(board, msg, turn);
      break;
    case AnswerNewGame:
      return answernewgame();
      break;
    case SendMsg:
      return sendmessage(msg);
      break;
    case Image:
      return image();
      break;
    default:
      return build_pkg(ErrBadPkg, " ");
  }

  return NULL;
}

char* startconnection()
{
  char* pkg = build_pkg(ConnectionEstablished, " ");
  return pkg;
}

char* retnickname(char* msg)
{
  char *nickname = malloc(2 + sizeof(char) * (int) msg[1]);
  strcpy(nickname, &msg[2]);
  nickname[(int) msg[1]] = '\0';
  return nickname;
}

char* sendmove(Board *board, char *msg, int turn)
{
  char origin[2] = { msg[2], msg[3] };
  char destiny[2] = { msg[4], msg[5] };
  bool move = check_movement(board, origin, destiny, turn);

  char* pkg = NULL;
  if (move) pkg = build_pkg(OkMove, " ");
  else pkg = build_pkg(ErrMove, " ");

  return pkg;
}

char* answernewgame()
{
  char* pkg = build_pkg(StartConnection, " ");
  return pkg;
}

char* sendmessage(char* msg)
{
  char* pkg = build_pkg(SpreadMsg, &msg[2]);
  return pkg;
}

char* image()
{
  char* pkg = build_pkg(Image, " ");
  return pkg;
}

int set_id(void)
{
  int num = rand() % 2 + 1;
  return num;
}