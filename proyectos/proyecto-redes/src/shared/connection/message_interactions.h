#pragma once

#include "message.h"

void sendMessage(int socket, char* package, bool save_log);

char* receiveMessage(int socket, bool save_log);

void print_package(char * package);

int calculate_length(char * input);