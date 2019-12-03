#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>


void main() {
    if (!fork()){
        printf("Naci!\n");
        char cmd[10] = "ls";
        char *args[2];
        args[0] = malloc(sizeof(char) * 10);
        strcpy(args[0], "-la");
        args[1] = NULL;
        printf("%s\n", args[0]);
        execvp(cmd, args);
    };
};