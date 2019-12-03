#pragma once

#include "../../game/board.h"
#include "../../shared/connection/message.h"

/** Funcion que maneja cada respuesta */
char* response_handler(char *msg, Board *board, int turn);

char* startconnection();

char* retnickname(char *msg);

char* sendmove();

char* answernewgame();

char* sendmessage();

char* image();

int set_id();
