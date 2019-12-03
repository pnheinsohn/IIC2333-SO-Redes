#include <string.h>
#include <stdlib.h>

#include "builders.h"
#include "message_interactions.h"
#include "message.h"

char* build_pkg(MsgType type, char* payload)
{
  int payloadSize = calculate_length(payload);
  char *package = malloc(3 + payloadSize);
  package[0] = type;
  package[1] = payloadSize;
  strcpy(&package[2], payload);
  return package;
}