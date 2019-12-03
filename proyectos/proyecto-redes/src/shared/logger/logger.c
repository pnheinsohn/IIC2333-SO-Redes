#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "logger.h"
FILE *fp;
static int SESSION_TRACKER; //Keeps track of session

char *print_time() {
  int size = 0;
  time_t t;
  char *buf;

  t = time(NULL); /* get current calendar time */

  char *timestr = asctime(localtime(&t));
  timestr[strlen(timestr) - 1] = 0; //Getting rid of \n

  size = strlen(timestr) + 1 + 2; //Additional +2 for square braces
  buf = (char *)malloc(size);

  memset(buf, 0x0, size);
  snprintf(buf, size, "[%s]", timestr);

  return buf;
}

void log_print(int type, char ID, char *payload) {
  if (SESSION_TRACKER > 0) fp = fopen("log.txt", "a+");
  else fp = fopen("log.txt", "w");

  char *time = print_time();
  fprintf(fp, "%s ", time);

  printf("%c", ID);

  if (type == 0) fprintf(fp, "ENVIADO -- ID:%c PAYLOAD: %s ", ID, payload);
  else if (type == 1) fprintf(fp, "RECIBIDO: -- ID:%c PAYLOAD: %s ", ID, payload);
  else fprintf(fp, "%s ", payload);

  fputc('\n', fp);

  SESSION_TRACKER++;
  free(time);
  fclose(fp);
}
