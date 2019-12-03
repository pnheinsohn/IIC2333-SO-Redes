#include <stdbool.h>

#pragma once


typedef enum estado {
    RUNNING,
    READY,
    WAITING,
    FINISHED,
    IDLE
} State;

typedef struct process {
    int pid;
    char name[257];
    int priority;
    int start_time;
    int length;
    State state;
    int* subprocesses;
    int actual_subprocess;
    bool in_queue;
    bool finished;
    int quantum_counter;

    // Statistical Purposes
    int total_counter;
    int turnaround_time;
    int response_time;
    int waiting_time;
    int cpu_use;
    int times_interrupted;
} Process;

typedef struct node {
    struct node* prev;
    struct node* next;
    Process* process;
} Node;

typedef struct queue {
    Node* head;
    Node* tail;
    int length;
} Queue;

bool has_finished(Process* process);

void queue_printer(Node* node);

Queue* queue_init();

Queue* try_queue_up(Process* process, Queue* queue);

Process* queue_pop(Queue* queue);

void append(Process* process, Queue* queue);

Process* process_init();
