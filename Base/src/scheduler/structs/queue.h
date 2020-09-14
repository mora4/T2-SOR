#pragma once
#include "./list.h"

struct queue;
typedef struct queue Queue;

// Definimos el struct nodo
struct queue
{
    List* not_started_processes;
    List* waiting_processes;
    List* ready_processes;
    List* running_processes;
    List* finished_processes;

};

Queue* queue_init();
void queue_destroy(Queue* queue);