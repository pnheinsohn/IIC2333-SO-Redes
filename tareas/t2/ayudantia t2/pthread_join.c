#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *thread_funct(void *args){
    printf("Hola soy un simple thread\n");
    sleep(1);
    return NULL;
};
        
int main(){
    pthread_t my_thread;
    printf("Creando thread\n");
    pthread_create(&my_thread, NULL, thread_funct, NULL);
    printf("Thread creado, esperando\n");
    pthread_join(my_thread, NULL);
    printf("Adiós\n");
    // Retornar el el main mata a todos los threads
    return 0;
};