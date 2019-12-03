#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "initializer.h"
#include "handlers/handlers.h"
#include "../game/board.h"
#include "../shared/logger/logger.h"
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

  char* msg_in = NULL;
  char* msg_out = NULL;
  char* nickname0;
  char* nickname1;
  char scores[2];
  int* sockets;
  int id_p1;
  int id_p2;

  Game *game = game_init();

  printf("Server\n");
  // log_print(2, "Servidor inicializado");
  sockets = initializeServer(IP, PORT, save_log);

  // LOG_PRINT("Servidor inicializado");

  char msg[] = " ";
  sendMessage(sockets[0], build_pkg(AskNickname, msg), save_log);
  sendMessage(sockets[1], build_pkg(AskNickname, msg), save_log);
  msg_in = receiveMessage(sockets[0], save_log);
  nickname0 = response_handler(msg_in, NULL, 0);
  free(msg_in);
  
  bool used = true;
  while (used) {
    msg_in = receiveMessage(sockets[1], save_log);
    nickname1 = response_handler(msg_in, NULL, 0);
    if (strcmp(nickname0, nickname1) != 0) used = false;
    else {
      free(nickname1);
      sendMessage(sockets[1], build_pkg(AskNickname, msg), save_log);
    }
    free(msg_in);
  }

  sendMessage(sockets[0], build_pkg(Oponentfound, nickname1), save_log);
  sendMessage(sockets[1], build_pkg(Oponentfound, nickname0), save_log);

  int socket;
  bool play = true;
  while (play) {
    sendMessage(sockets[0], build_pkg(Start, msg), save_log);
    sendMessage(sockets[1], build_pkg(Start, msg), save_log);

    scores[0] = game->score1 + '0';
    scores[1] = game->score2 + '0';
    scores[2] = '\0';
    sendMessage(sockets[0], build_pkg(Scores, scores), save_log);

    scores[0] = game->score2 + '0';
    scores[1] = game->score2 + '0';
    scores[2] = '\0';
    sendMessage(sockets[1], build_pkg(Scores, scores), save_log);

    // int number = set_id();
    int number = rand() % 2 + 1;
    id_p1 = number;
    if (id_p1 == 1) id_p2 = 2;
    else id_p2 = 1;

    char order[1];
    sprintf(order, "%d", id_p1);
    sendMessage(sockets[0], build_pkg(WhosFirst, order), save_log);

    sprintf(order, "%d", id_p2);
    sendMessage(sockets[1], build_pkg(WhosFirst, order), save_log);

    int n_socket = 0;
    if (id_p1 == 2) n_socket = 0;
    else n_socket = 1;

    while (1)
    {
      socket = sockets[n_socket];
      // Le mandamos el estado actual
      char *charBoard = malloc(sizeof(char) * 65);
      charBoard = board_to_char(game -> actualBoard, charBoard);
      sendMessage(socket, build_pkg(BoardState, charBoard), save_log);
      free(charBoard);

      // Respuesta del cliente
      msg_in = receiveMessage(socket, save_log);
      if (msg_in[0]==Disconnect) {
        free(msg_in);
        free(nickname0);
        free(nickname1);
        game_destroy(game);
        sendMessage(sockets[0], build_pkg(Disconnect, " \0"), save_log);
        terminate_connection(sockets[0]);
        sendMessage(sockets[1], build_pkg(Disconnect, " \0"), save_log);
        terminate_connection(sockets[1]);
        exit(0);
      }
      msg_out = response_handler(msg_in, game -> actualBoard, n_socket + 1);
      free(msg_in);
      if (msg_out[0] == SpreadMsg) sendMessage(sockets[1 - n_socket], msg_out, save_log);
      else if (msg_out[0] == OkMove) {
        if (game_ended(game -> actualBoard)) {
          char *winner = malloc(sizeof(char) * 2);
          winner[0] = get_winner(game->actualBoard) + '0';
          winner[1] = '\0';
          if (winner[0] == '0') {
            game->score1++;
            game->score2++;
          }
          else if (winner[0] == '1') game->score1++;
          else if (winner[0] == '2') game->score2++;

          sendMessage(socket, build_pkg(GameWinnerLoser, winner), save_log);
          sendMessage(sockets[1 - n_socket], build_pkg(GameWinnerLoser, winner), save_log);
          free(winner);

          sendMessage(socket, build_pkg(End, " "), save_log);
          sendMessage(sockets[1 - n_socket], build_pkg(End, " "), save_log);
          
          bool aux_play_again = true;
          sendMessage(socket, build_pkg(AskNewGame, " "), save_log);
          msg_in = receiveMessage(socket, save_log);
          if (msg_in[2] != '1') aux_play_again = false;
          free(msg_in);

          sendMessage(sockets[1 - n_socket], build_pkg(AskNewGame, " "), save_log);
          msg_in = receiveMessage(sockets[1 - n_socket], save_log);
          if (msg_in[2] != '1') aux_play_again = false;
          free(msg_in);

          free(msg_out);
          if (!aux_play_again) {
            play = false;
            sendMessage(socket, build_pkg(Disconnect, " \0"), save_log);
            terminate_connection(socket);
            sendMessage(sockets[n_socket - 1], build_pkg(Disconnect, " \0"), save_log);
            terminate_connection(sockets[n_socket - 1]);
            break;
          }
          else {
            board_destroy(game->actualBoard);
            game->actualBoard = board_init();            
            break;
          }
        }
        else n_socket = 1 - n_socket;
      }
      sendMessage(socket, msg_out, save_log);
    }
  }

  // log_print(2, "Servidor apagado");
  free(nickname0);
  free(nickname1);
  game_destroy(game);
}
