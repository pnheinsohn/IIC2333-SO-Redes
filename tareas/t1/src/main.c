#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "structs.h"


int main(int argc, char* argv[]) {
    FILE* fp;
    Process* head;
    int priority, start_time, process_length;

    int pid = 0;
    int quantum = 0;
    bool done = false;
    bool running = false;
    int process_amount = 1;
    Queue* queue = queue_init();
    char* name = malloc(sizeof(char)*257);
    Process** all_processes = malloc(sizeof(Process*)*process_amount);  // 1 initial process

    if (argc < 4) {
        fprintf(stderr, "Usage: %s <soure-file> <output-file> <version> [quantum]\n", argv[0]);
        free(name);
        free(queue);
        free(all_processes);
        return 1;
    } 
    
    if (strcmp(argv[3], "p") == 0) {
        if (argc == 4) {
            quantum = 3;
        } else if (argc == 5) {
            quantum = strtol(argv[4], NULL, 10);
        } else {
            free(name);
            free(queue);
            free(all_processes);
            return 1;
        }
    }

    if ((fp = fopen(argv[1], "r")) == NULL) {
        perror("fopen source-file");
        free(name);
        free(queue);
        free(all_processes);
        return 1;
    }

    while (fscanf(fp, "%s %i %i %i", name, &priority, &start_time, &process_length) == 4) {
        int* subprocesses = malloc(sizeof(int) * (2 * process_length - 1));
        for (int i=0; i < (2*process_length - 1); i++) {
            fscanf(fp, "%i", &subprocesses[i]);
        }
        Process* process = process_init(pid, name, priority, start_time, process_length, subprocesses);
        if (pid == process_amount) {  // If size of all_processes won't be enough
            Process** new = malloc(sizeof(Process*) * process_amount * 2);
            for (int i=0; i < process_amount; i++){
                new[i] = all_processes[i];
            }
            process_amount *= 2;
            free(all_processes);
            all_processes = new;
        }
        all_processes[pid] = process;
        pid++;
    }
    fclose(fp);

    int counter = 0;
    while (!done) {
        printf("_______________%i_______________\n", counter);
        for (int i=0; i < pid; i++) {
           try_queue_up(all_processes[i], queue);
        }

        if (queue -> length > 0 && !running) {  // Not empty queue
            running = true;
            head = queue_pop(queue);
            head -> state = RUNNING;
            head -> cpu_use += 1;
            printf("PROCESO %s COMENZO A USAR CPU: %i\n", head -> name, head -> cpu_use);
            if (head -> response_time == -1) {
                head -> response_time = head -> total_counter - head -> start_time;
            }
        }

        if (running) {
            printf("Subprocesos de %s:\n", head -> name);
            for (int i=0; i < head -> length * 2 - 1; i++) {
                printf("%i ", head -> subprocesses[i]);
            }
            printf("\n");
            

            if (quantum != 0) {  // Quantum Version
                head -> quantum_counter += 1;
                head -> subprocesses[head -> actual_subprocess] -= 1;
                printf("QUANTUM: %i || MY QUANTUM: %i\n", quantum, head -> quantum_counter);
                if (head -> quantum_counter == quantum) {
                    head -> quantum_counter = 0;
                    head -> times_interrupted += 1;
                    printf("PROCESO: %s - INTERRUMPIDO: %i\n", head -> name, head -> times_interrupted);
                    running = false;
                    if (has_finished(head) && head -> subprocesses[head -> actual_subprocess] == 0) {
                        head -> state = FINISHED;
                        head -> turnaround_time = head -> total_counter - head -> start_time + 1;
                    } else if (head -> subprocesses[head -> actual_subprocess] == 0) {
                        head -> state = WAITING;
                        head -> actual_subprocess += 1;
                        head -> waiting_time -= 1;
                    } else if (head -> subprocesses[head -> actual_subprocess] != 0) {
                        head -> state = READY;
                        head -> waiting_time -= 1;
                    }
                } else {
                    if (has_finished(head) && head -> subprocesses[head -> actual_subprocess] == 0) {
                        head -> quantum_counter = 0;
                        head -> state = FINISHED;
                        head -> turnaround_time = head -> total_counter - head -> start_time + 1;
                        running = false;
                    } else if (head -> subprocesses[head -> actual_subprocess] == 0) {
                        head -> quantum_counter = 0;
                        head -> actual_subprocess += 1;
                        head -> state = WAITING;
                        running = false;
                        head -> waiting_time -= 1;
                    }
                }
            } else {  // No Quantum Version
                head -> subprocesses[head -> actual_subprocess] -= 1;
                if (has_finished(head) && head -> subprocesses[head -> actual_subprocess] == 0) {
                    head -> state = FINISHED;
                    head -> turnaround_time = head -> total_counter - head -> start_time + 1;
                    running = false;
                } else if (head -> subprocesses[head -> actual_subprocess] == 0) {
                    head -> actual_subprocess += 1;
                    head -> state = WAITING;
                    running = false;
                    head -> waiting_time -= 1;
                }
            }

            printf("Subprocesos de %s DESPUES:\n", head -> name);
            for (int i=0; i < head -> length * 2 - 1; i++) {
                printf("%i ", head -> subprocesses[i]);
            }
            printf("\n");
        }

        for (int i=0; i < pid; i++) {
            all_processes[i] -> total_counter += 1;

            if (all_processes[i] -> state == WAITING) {
                                
                if (all_processes[i] -> subprocesses[all_processes[i] -> actual_subprocess] != 0) {
                    all_processes[i] -> subprocesses[all_processes[i] -> actual_subprocess] -= 1;
                } else {
                    all_processes[i] -> actual_subprocess += 1;
                    all_processes[i] -> state = READY;
                }
                all_processes[i] -> waiting_time += 1;
            } else if (all_processes[i] -> state == READY) {
                all_processes[i] -> waiting_time += 1;
            }
        }

        done = true;
        for (int i=0; i < pid; i++) {
            if (all_processes[i] -> state != FINISHED) {
                done = false;
                break;
            }
        }
        counter++;
    }

    FILE* fpout = fopen(argv[2], "w");
    for (int i=0; i < pid; i++) {
        fprintf(fpout, "%s, %i, %i, %i, %i, %i\n", all_processes[i] -> name, all_processes[i] -> cpu_use, 
                all_processes[i] -> times_interrupted, all_processes[i] -> turnaround_time, 
                all_processes[i] -> response_time, all_processes[i] -> waiting_time);
    }
    fclose(fpout);

    for (int i=0; i < pid; i++) {
        free(all_processes[i] -> subprocesses);
        free(all_processes[i]);
    }
    free(all_processes);
    free(queue);
    free(name);
    return 0;
}
