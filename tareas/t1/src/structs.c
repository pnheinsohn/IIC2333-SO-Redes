#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "structs.h"


Process* process_init(int pid, char* name, int priority, int start_time, int length, int* subprocesses) {
    Process* p = malloc(sizeof(Process));
    p -> pid = pid;
    strcpy(p -> name, name);
    p -> priority = priority;
    p -> start_time = start_time;
    p -> length = length;
    p -> state = IDLE;
    p -> subprocesses = subprocesses;
    p -> actual_subprocess = 0;
    p -> in_queue = false;
    p -> finished = false;
    p -> quantum_counter = 0;

    p -> total_counter = 0;
    p -> turnaround_time = 0;
    p -> response_time = -1;
    p -> waiting_time = 0;
    p -> cpu_use = 0;
    p -> times_interrupted = 0;
    return p;
}

bool has_finished(Process* p) {
    int aux = p -> length;
    if (2*aux - 1 > p -> actual_subprocess + 1) {
        return false;
    }
    return true;
}

Node* node_init(Process* process) {
    Node* n = malloc(sizeof(Node));
    n -> prev = NULL;
    n -> next = NULL;
    n -> process = process;
    return n;
}

Queue* queue_init() {
    Queue* q = malloc(sizeof(Queue));
    q -> head = NULL;
    q -> tail = NULL;
    q -> length = 0;
    return q;
}

void queue_printer(Node* head) {
    if (head == NULL) {
        printf("Queue Vacía\n");
    } else {
        printf("Id: %i || Proceso: %s || Start_time: %i || State: %i || Total_counter: %i\n", 
                head -> process -> pid, head -> process -> name, 
                head -> process -> start_time, head -> process -> state, head -> process -> total_counter);
        if (head -> next == NULL) {
            return;
        }
        queue_printer(head -> next);
    }
}

void append(Process* process, Queue* queue) {
    Node* node = node_init(process);
    if (queue -> head == NULL) {
        queue -> head = node;
        queue -> tail = node;
    } else {
        queue -> tail -> next = node;
        node -> prev = queue -> tail;
        queue -> tail = node;
    }
    queue -> length += 1;
}

static void ready_queue_up(Node* node, Queue* queue, Node* aux) {
    if (queue -> head == NULL) {
        queue -> head = node;
        queue -> tail = node;
        queue -> length += 1;
    } else if ((node -> process -> priority > aux -> process -> priority) || (aux -> process -> state != READY) || 
            (node -> process -> priority == aux -> process -> priority && node -> process -> pid > aux -> process -> pid)) {
        if (aux -> prev == NULL) {
            queue -> head = node;
        } else {
            node -> prev = aux -> prev;
            aux -> prev -> next = node;
        }
        node -> next = aux;
        aux -> prev = node;
        queue -> length += 1;
    } else if (aux -> next == NULL) {
        aux -> next = node;
        node -> prev = aux;
        node -> next = NULL;
        queue -> tail = node;
        queue -> length += 1;
    } else {
        ready_queue_up(node, queue, aux -> next);
    }
}

Queue* try_queue_up(Process* process, Queue* queue) {
    if (process -> state == READY && !process -> in_queue) {
        printf("PROCESO %s PASO A READY EN %i\n", process -> name, process -> total_counter);
        process -> in_queue = true;
        Node* node = node_init(process);
        ready_queue_up(node, queue, queue -> head);
    } else if (process -> state == IDLE && !process -> in_queue) {
        if (process -> start_time == process -> total_counter) {
            printf("PROCESO %s PASO A READY EN %i\n", process -> name, process -> total_counter);
            process -> state = READY;
            process -> in_queue = true;
            Node* node = node_init(process);
            ready_queue_up(node, queue, queue -> head);
        } 
    }
    return queue;
}

Process* queue_pop(Queue* queue) {
    if (queue -> length == 0) {
        return NULL;
    }

    Node* n = queue -> head;
    Process* p = n -> process;
    p -> in_queue = false;

    if (n -> next != NULL) {
        queue -> head = n -> next;
        n -> next -> prev = NULL;
    } else {
        queue -> head = NULL;
        queue -> tail = NULL;
    }

    free(n);
    queue -> length -= 1;
    return p;
}
