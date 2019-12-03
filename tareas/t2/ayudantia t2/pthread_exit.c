#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void *thread_funct(void *args){
    char* string = malloc(sizeof(char) * 20);
    strcpy(string, "Hola soy un string\n");
    pthread_exit(string);
};
        
int main(){
    pthread_t my_thread;
    void* return_val;
    printf("Creando thread\n");
    pthread_create(&my_thread, NULL, thread_funct, NULL);
    printf("Thread creado, esperando\n");
    pthread_join(my_thread, &return_val);
    printf("Mensaje del thread: %s", (char *)return_val);
    printf("Adiós\n");
    // Retornar el el main mata a todos los threads
    return 0;
};