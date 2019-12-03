#pragma once

#include "../../shared/connection/message.h"

/** Funcion que maneja cada respuesta */
char* response_handler(char *msg);

char* connectionestablished();

char* asknickname();

char* oponentfound();

char* start();

char* scores();

char* whosfirst();

char* boardstate(char *msg);

char* errmove();

char* okmove();

char* end();

char* gamewinnerloser();

char* asknewgame();

void disconnect();

char* errbadpkg();

char* spreadmsg();

char* image();