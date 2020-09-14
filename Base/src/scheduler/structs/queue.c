#include "queue.h"
#include <stdlib.h>
#include <stdio.h>


Queue* queue_init(){
    Queue* queue = malloc(sizeof(Queue));
    queue ->not_started_processes = list_init();
    queue -> waiting_processes = list_init();
    queue ->ready_processes = list_init();
    queue ->running_processes = list_init();
    queue -> finished_processes = list_init();

}

void queue_destroy(Queue* queue);