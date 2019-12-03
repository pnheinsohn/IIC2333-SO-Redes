#include <stdio.h>
#include <unistd.h>

int main() {
	int pid1 = fork();
	int pid2 = fork();
	printf("Hola! Soy %d, mi padre es %d y mis hijos son %d y %d\n", getpid(), getppid(), pid1, pid2);
};