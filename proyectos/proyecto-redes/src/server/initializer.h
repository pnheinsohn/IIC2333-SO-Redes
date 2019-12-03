#pragma once

#include <stdbool.h>

int* initializeServer(char* ip, int port, bool save_log);

void terminate_connection(int socket);