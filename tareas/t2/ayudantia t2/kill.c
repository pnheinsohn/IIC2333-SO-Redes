#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
	int pid;
    if ((pid = fork()) == 0){
    	sleep(1);
        kill(getppid(), SIGKILL);
        exit(0);
    } else {
        while(1){
        	fprintf(stdout, "Correré para siempre >:)\n");
        	usleep(100000);
        };
    };
};